# dangling::ptr

This is simple header-only library implementing class of pointers
which automatically track lifetime of their pointees - when pointee
is destroyed, pointer automatically resets to nullptr, so there is
a way to know that object it references no longer exists and accessing
freed memory is impossible.

## Synopsis

    #include <danglingptr.hh>

    class MyObject : public dangling::target<MyObject> {
    public:
        bool MyMethod() {
            return true;
        }
    };

    ...

    {
        dangling::ptr<MyObject> ptr;

        {
            MyObject obj;
            ptr = &obj;

            assert(ptr);
            assert(ptr.get() == &obj);
            assert(ptr->MyMethod());
        }

        // object is no more, but pointer is aware of it
        assert(!ptr);
        assert(ptr.get() == nullptr)
		try {
			ptr->MyMethod();
        } catch (dangling::bad_access& e) {
            std::cerr << "attempt to access destroyed object" << std::endl;
        }
    }

## Author ##

* [Dmitry Marakasov](https://github.com/AMDmi3) <amdmi3@amdmi3.ru>

## License ##

2-clause BSD, see COPYING.
