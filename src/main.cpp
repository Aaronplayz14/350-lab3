#include <cstdlib>
#include <iostream>
#include <utility>
#include "SharedPtr.h"

struct TestObject {
    int value;

    explicit TestObject(int value) : value{value}
    {
        std::cout << "TestObject(" << value << ") constructed\n";
    }

    ~TestObject()
    {
        std::cout << "TestObject(" << value << ") destroyed\n";
    }

    void add(int amount)
    {
        value += amount;
    }
};

struct Pair {
    int first;
    int second;

    Pair(int first, int second) : first{first}, second{second}
    {
    }
};

int main()
{
    bool allPassed = true;
    {
        SharedPtr<int> p;

        if (p.get() == nullptr && p.useCount() == 0 && !p)
        {
            std::cout << "PASS: default constructor\n";
        }
        else
        {
            std::cout << "FAIL: default constructor\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p(new int(5));

        if (p.get() != nullptr && *p == 5 && p && p.useCount() == 1)
        {
            std::cout << "PASS: raw-pointer constructor / dereference / get / bool\n";
        }
        else
        {
            std::cout << "FAIL: raw-pointer constructor / dereference / get / bool\n";
            allPassed = false;
        }
    }
    {
        // control block exists but managed/stored pointer is nullptr
        SharedPtr<int> p(nullptr);

        if (p.get() == nullptr && p.useCount() == 1)
        {
            std::cout << "PASS: raw-pointer constructor with nullptr\n";
        }
        else
        {
            std::cout << "FAIL: raw-pointer constructor with nullptr\n";
            allPassed = false;
        }
    }
    {
        // copying/moving an empty pointer is legal
        SharedPtr<int> e;
        SharedPtr<int> e2(e);
        SharedPtr<int> e3(std::move(e2));

        if (!e && !e2 && !e3 && e.useCount() == 0 && e3.useCount() == 0)
        {
            std::cout << "PASS: copy/move of empty pointer\n";
        }
        else
        {
            std::cout << "FAIL: copy/move of empty pointer\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p1(new int(10));
        SharedPtr<int> p2(p1);

        bool correctBeforeChange =
            p1.get() == p2.get() &&
            p1.useCount() == 2 &&
            p2.useCount() == 2;

        *p2 = 20;
        bool correctAfterChange = (*p1 == 20);

        if (correctBeforeChange && correctAfterChange)
        {
            std::cout << "PASS: copy constructor / shared ownership\n";
        }
        else
        {
            std::cout << "FAIL: copy constructor / shared ownership\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p1(new int(10));
        SharedPtr<int> p2(new int(20));

        p2 = p1;

        if (p1.get() == p2.get() && *p2 == 10 &&
            p1.useCount() == 2 && p2.useCount() == 2)
        {
            std::cout << "PASS: copy assignment\n";
        }
        else
        {
            std::cout << "FAIL: copy assignment\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p1(new int(30));
        SharedPtr<int> p2(std::move(p1));

        if (p1.get() == nullptr && p1.useCount() == 0 &&
            *p2 == 30 && p2.useCount() == 1)
        {
            std::cout << "PASS: move constructor\n";
        }
        else
        {
            std::cout << "FAIL: move constructor\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p1(new int(40));
        SharedPtr<int> p2(new int(50));

        p2 = std::move(p1);

        if (p1.get() == nullptr && p1.useCount() == 0 &&
            *p2 == 40 && p2.useCount() == 1)
        {
            std::cout << "PASS: move assignment\n";
        }
        else
        {
            std::cout << "FAIL: move assignment\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<TestObject> p(new TestObject(5));

        p->add(3);

        if (p->value == 8 && (*p).value == 8)
        {
            std::cout << "PASS: arrow operator\n";
        }
        else
        {
            std::cout << "FAIL: arrow operator\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p1(new int(60));
        SharedPtr<int> p2(p1);
        SharedPtr<int> p3(new int(60));

        if (p1 == p2 && p1 != p3)
        {
            std::cout << "PASS: equality operator\n";
        }
        else
        {
            std::cout << "FAIL: equality operator\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p1(new int(70));
        SharedPtr<int> p2(new int(80));

        p1.swap(p2);

        if (*p1 == 80 && *p2 == 70 &&
            p1.useCount() == 1 && p2.useCount() == 1)
        {
            std::cout << "PASS: swap\n";
        }
        else
        {
            std::cout << "FAIL: swap\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p(new int(90));
        SharedPtr<int> copy(p);

        bool beforeReset = (p.useCount() == 2);
        p.reset();

        bool afterReset =
            p.get() == nullptr &&
            p.useCount() == 0 &&
            copy.useCount() == 1 &&
            *copy == 90;

        if (beforeReset && afterReset)
        {
            std::cout << "PASS: reset()\n";
        }
        else
        {
            std::cout << "FAIL: reset()\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p(new int(100));

        p.reset(new int(110));
        bool afterNewPointer = *p == 110 && p.useCount() == 1;

        p.reset(p.get());
        bool afterSelfReset = *p == 110 && p.useCount() == 1;

        if (afterNewPointer && afterSelfReset)
        {
            std::cout << "PASS: reset(T*) / self-reset\n";
        }
        else
        {
            std::cout << "FAIL: reset(T*) / self-reset\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<int> p1(new int(120));
        SharedPtr<int> p2(p1);
        SharedPtr<int> p3(p2);

        if (p1.useCount() == 3 && p2.useCount() == 3 && p3.useCount() == 3)
        {
            std::cout << "PASS: useCount()\n";
        }
        else
        {
            std::cout << "FAIL: useCount()\n";
            allPassed = false;
        }
    }
    {
        auto p = makeSharedBasic<Pair>(1, 2);

        if (p->first == 1 && p->second == 2 && p.useCount() == 1)
        {
            std::cout << "PASS: makeSharedBasic()\n";
        }
        else
        {
            std::cout << "FAIL: makeSharedBasic()\n";
            allPassed = false;
        }
    }
    {
        auto p = makeShared<Pair>(7, 8);

        if (p->first == 7 && p->second == 8 && p.useCount() == 1)
        {
            std::cout << "PASS: makeShared() embedded control block\n";
        }
        else
        {
            std::cout << "FAIL: makeShared() embedded control block\n";
            allPassed = false;
        }
    }
    {
        auto p1 = makeShared<TestObject>(150);
        bool initial = p1.useCount() == 1 && p1->value == 150;

        {
            SharedPtr<TestObject> p2(p1);
            bool shared =
                p1.useCount() == 2 && p2.useCount() == 2 &&
                p1->value == 150 && p2->value == 150 && p1.get() == p2.get();

            if (initial && shared)
            {
                std::cout << "PASS: makeShared() shares ownership\n";
            }
            else
            {
                std::cout << "FAIL: makeShared() shares ownership\n";
                allPassed = false;
            }
        }

        if (p1.useCount() == 1 && p1->value == 150)
        {
            std::cout << "PASS: makeShared() refcount after shared_ptr dies\n";
        }
        else
        {
            std::cout << "FAIL: makeShared() refcount after shared_ptr dies\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<TestObject> p(new TestObject(200));
        const SharedPtr<TestObject>& cp = p;

        cp->add(1);

        if (p->value == 201)
        {
            std::cout << "PASS: const SharedPtr access\n";
        }
        else
        {
            std::cout << "FAIL: const SharedPtr access\n";
            allPassed = false;
        }
    }
    {
        SharedPtr<Pair> p = makeSharedBasic<Pair>(300, 400);
        SharedPtr<int> first(p, &p->first);

        bool beforeReset =
            first.get() == &p->first &&
            *first == 300 &&
            first.useCount() == 2;

        p.reset();

        bool afterReset = *first == 300 && first.useCount() == 1;

        *first = 350;
        bool afterModify = (*first == 350);

        if (beforeReset && afterReset && afterModify)
        {
            std::cout << "PASS: aliasing constructor (bonus)\n";
        }
        else
        {
            std::cout << "FAIL: aliasing constructor (bonus)\n";
            allPassed = false;
        }
    }

    if (allPassed)
    {
        std::cout << "\nAll tests passed!\n";
        return 0;
    }

    std::cout << "\nSome tests failed.\n";
    return 1;
}