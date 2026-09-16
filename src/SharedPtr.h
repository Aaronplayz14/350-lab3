#include <utility>
#include <cassert>
#include <memory>
#include <stdexcept>
#ifndef SHARED_PTR_HEADER
#define SHARED_PTR_HEADER

class ControlBlockBase {
public:
    ControlBlockBase() : refcount(1) {}

    // dtor is virtual, so that we can call derived class's dtor from a ptr to this base class.
    virtual ~ControlBlockBase() { assert(refcount == 0); }

    // pure virtual function; must be overriden by derived classes
    virtual void* managedAddress() = 0;

    // Delete copies, which also implicitly deletes moves.
    ControlBlockBase(const ControlBlockBase&) = delete;
    ControlBlockBase& operator=(const ControlBlockBase&) = delete;

    long increment()
    {
        assert(refcount > 0);
        return ++refcount;
    }

    long decrement()
    {
        assert(refcount > 0);
        return --refcount;
    }

    long refCount() const
    {
        return refcount;
    }

private:
    long refcount;
};
template <typename T>
class ControlBlock : public ControlBlockBase {
public:
    ControlBlock(T* ptr):ptr(ptr) {}
    ~ControlBlock() override {delete ptr;}
    void* managedAddress() override
    {
        return ptr;
    }

private:
    T* ptr;
};
template <typename T>
class ControlBlockEmbedded : public ControlBlockBase
{
public:
    template <typename... Args>
    ControlBlockEmbedded(Args&&... args) : obj(std::forward<Args>(args)...) {}

    ~ControlBlockEmbedded() override = default;

    void* managedAddress() override
    {
        return std::addressof(obj);
    }

private:
    T obj;
};
template <typename T>
class SharedPtr {
public:
    SharedPtr():storedPtr(nullptr), controlBlock(nullptr) {}
    explicit SharedPtr(T* ptr):storedPtr(ptr), controlBlock(new ControlBlock<T>(ptr)) {}
    ~SharedPtr()
    {
        if (controlBlock != nullptr)
        {
            if (controlBlock->decrement() == 0)
            {
                delete controlBlock;
            }
        }
    }
    SharedPtr(const SharedPtr& other):storedPtr(other.storedPtr), controlBlock(other.controlBlock)
    {
        if (controlBlock != nullptr)
        {
            controlBlock->increment();
        }
    }
    SharedPtr(SharedPtr&& other) noexcept :storedPtr(other.storedPtr), controlBlock(other.controlBlock)
    {
        other.storedPtr = nullptr;
        other.controlBlock = nullptr;
    }
    SharedPtr& operator=(const SharedPtr& other)
    {
        if (this != &other)
        {
            if (controlBlock != nullptr && controlBlock->decrement() == 0)
            {
                delete controlBlock;
            }
            storedPtr = other.storedPtr;
            controlBlock = other.controlBlock;
            if (controlBlock != nullptr)
            {
                controlBlock->increment();
            }
        }
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept
    {
        if (this != &other)
        {
            if (controlBlock != nullptr && controlBlock->decrement() == 0)
            {
                delete controlBlock;
            }
            storedPtr = other.storedPtr;
            controlBlock = other.controlBlock;
            other.storedPtr = nullptr;
            other.controlBlock = nullptr;
        }
        return *this;
    }
    T& operator*() const 
    {
        if (storedPtr != nullptr)
        {
            return *storedPtr;
        }
        throw std::runtime_error("Dereferencing null pointer");
    }
    T* operator->() const 
    {
        if (storedPtr != nullptr)
        {
            return storedPtr;
        }
        throw std::runtime_error("Dereferencing null pointer");
    }
    T* get() const
    {
        return storedPtr;
    }
    bool operator==(const SharedPtr<T>& other) const{return storedPtr == other.storedPtr;}
    void reset()
    {
        if (controlBlock != nullptr && controlBlock->decrement() == 0)
        {
            delete controlBlock;
        }
        storedPtr = nullptr;
        controlBlock = nullptr;
    }
    void reset(T* others){
        if (others==storedPtr)
        {
            return;
        }
        if (controlBlock != nullptr && controlBlock->decrement() == 0)
        {
            delete controlBlock;
        }
        storedPtr = others;
        if (storedPtr != nullptr)
        {
            controlBlock = new ControlBlock<T>(storedPtr);
        }
    }
    void swap(SharedPtr<T>& other) noexcept
    {
        std::swap(storedPtr, other.storedPtr);
        std::swap(controlBlock, other.controlBlock);
    }
    operator bool() const{return storedPtr != nullptr;}
    long useCount() const
    {
        if (controlBlock != nullptr)
        {
            return controlBlock->refCount();
        }
        return 0;
    }
    template <typename U>
    SharedPtr(const SharedPtr<U>& other, T* storedPtr)
        : storedPtr{storedPtr}, controlBlock{other.controlBlock}
    {
        if (controlBlock != nullptr) {
            controlBlock->increment();
        }
    }

private:
    explicit SharedPtr(T* stored, ControlBlockBase* cb) : storedPtr(stored), controlBlock(cb) {}

    void releaseOwnership()
    {
        if (controlBlock != nullptr) {
            if (controlBlock->decrement() == 0) {
                delete controlBlock;
            }
        }
    }

    T* storedPtr;
    ControlBlockBase* controlBlock;

    template <typename>
    friend class SharedPtr;

    template <typename U, typename... Us>
    friend SharedPtr<U> makeShared(Us&&... us);
};
template <typename T, typename... Args>
SharedPtr<T> makeSharedBasic(Args&&... args)
{
    return SharedPtr<T>(
        new T(std::forward<Args>(args)...)
    );
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args)
{
    auto* cb = new ControlBlockEmbedded<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(static_cast<T*>(cb->managedAddress()), cb);
}

#endif
