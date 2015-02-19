#include "testing.h"

#define private public
#include "danglingptr.hh"
#undef private

#include <memory>

class Object : public dangling::target<Object> {
public:
	const Object* GetThis() const {
		return this;
	}
};

typedef dangling::ptr<Object> Ptr;

BEGIN_TEST()
	// exception
	{
		dangling::bad_access e;

		EXPECT_EQUAL((std::string)e.what(), (std::string)"bad dangling::ptr access");
	}

	// simple construct & destroy
	{
		Object obj;

		{
			Ptr p1(&obj);

			EXPECT_TRUE(obj.dangling_ptrs_.size() == 1);
			EXPECT_TRUE(p1 && p1.get() == &obj);

			Ptr p2(&obj);

			EXPECT_TRUE(obj.dangling_ptrs_.size() == 2);
			EXPECT_TRUE(p1 && p1.get() == &obj);
			EXPECT_TRUE(p2 && p2.get() == &obj);
		}

		EXPECT_TRUE(obj.dangling_ptrs_.size() == 0);
	}

	// dereferencing
	{
		Object obj;

		Ptr p(&obj);

		EXPECT_TRUE((bool)p);
		EXPECT_TRUE(p.get() == &obj);
		EXPECT_TRUE(&*p == &obj);
		EXPECT_TRUE(p->GetThis() == &obj);
	}

	// operators
	{
		Object obj1, obj2;
		Ptr p1(&obj1);

		{
			Ptr p2(&obj1);
			EXPECT_TRUE(p2 == p1);
			EXPECT_TRUE(p2 <= p1);
			EXPECT_TRUE(p2 >= p1);
			EXPECT_TRUE(!(p2 != p1));
			EXPECT_TRUE(!(p2 < p1));
			EXPECT_TRUE(!(p2 > p1));
		}

		{
			Ptr p2(&obj2);
			EXPECT_TRUE(p2 != p1);
			EXPECT_TRUE(!(p2 == p1));
			EXPECT_TRUE(p2 <= p1 || p2 >= p1);
			EXPECT_TRUE(p2 < p1 || p2 > p1);
		}

		{
			Ptr p2;
			EXPECT_TRUE(p2 != p1);
			EXPECT_TRUE(!(p2 == p1));
			EXPECT_TRUE(p2 <= p1 || p2 >= p1);
			EXPECT_TRUE(p2 < p1 || p2 > p1);
		}

		{
			Ptr n1, n2;
			EXPECT_TRUE(n1 == n2);
			EXPECT_TRUE(n1 <= n2);
			EXPECT_TRUE(n1 >= n2);
			EXPECT_TRUE(!(n1 != n2));
			EXPECT_TRUE(!(n1 < n2));
			EXPECT_TRUE(!(n1 > n2));
		}
	}

	// null dereferencing
	{
		Ptr n, m;
		{
			Ptr tmp = std::move(m);
		}

		// null
		EXPECT_TRUE(!n);
		EXPECT_TRUE(n.get() == nullptr);
		EXPECT_EXCEPTION((void)&*n, dangling::bad_access);
		EXPECT_EXCEPTION((void)n->GetThis(), dangling::bad_access);

		// moved-from
		EXPECT_TRUE(!m);
		EXPECT_TRUE(m.get() == nullptr);
		EXPECT_EXCEPTION((void)&*m, dangling::bad_access);
		EXPECT_EXCEPTION((void)m->GetThis(), dangling::bad_access);
	}

	// dangling
	{
		std::unique_ptr<Object> obj(new Object);
		Ptr p1(obj.get()), p2(obj.get());

		EXPECT_TRUE(p1 && p1.get() == obj.get());
		EXPECT_TRUE(p2 && p2.get() == obj.get());
		EXPECT_TRUE(p1 == p2);

		obj.reset(nullptr);

		EXPECT_TRUE(!p1);
		EXPECT_TRUE(p1.get() == nullptr);
		EXPECT_TRUE(!p2);
		EXPECT_TRUE(p2.get() == nullptr);
		EXPECT_TRUE(p1 == p2);
	}

	// copy construct
	{
		Object obj;
		Ptr p1(&obj), n, m;

		{
			Ptr tmp = std::move(m);
		}

		EXPECT_TRUE(obj.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(p1 && p1.get() == &obj);
		EXPECT_TRUE(!n && n.get() == nullptr);

		{
			Ptr p2(p1);

			EXPECT_TRUE(obj.dangling_ptrs_.size() == 2);
			EXPECT_TRUE(p1 && p1.get() == &obj);
			EXPECT_TRUE(p2 && p2.get() == &obj);
		}

		{
			// from null
			Ptr p2(n);

			EXPECT_TRUE(!p2 && p2.get() == nullptr);
		}

		{
			// from moved-from
			Ptr p2(m);

			EXPECT_TRUE(!p2 && p2.get() == nullptr);
		}

		EXPECT_TRUE(obj.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(p1 && p1.get() == &obj);
		EXPECT_TRUE(!n && n.get() == nullptr);
	}

	// move construct
	{
		Object obj;
		Ptr p1(&obj), n, m;

		{
			Ptr tmp = std::move(m);
		}

		EXPECT_TRUE(obj.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(p1 && p1.get() == &obj);

		{
			Ptr p2(std::move(p1));

			EXPECT_TRUE(obj.dangling_ptrs_.size() == 1);
			EXPECT_TRUE(!p1 && !p1.get());
			EXPECT_TRUE(p2 && p2.get() == &obj);
		}

		{
			// from null
			Ptr p2(n);

			EXPECT_TRUE(!p2 && p2.get() == nullptr);
		}

		{
			// from moved-from
			Ptr p2(m);

			EXPECT_TRUE(!p2 && p2.get() == nullptr);
		}

		EXPECT_TRUE(obj.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(!p1 && !p1.get());
	}

	// copy assign
	{
		Object obj1, obj2;
		Ptr p1(&obj1), p2(&obj2), n, m;
		{
			Ptr tmp = std::move(m);
		}

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(p2 && p2.get() == &obj2);

		p2 = p1;

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 2);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(p2 && p2.get() == &obj1);

		p2 = p2; // self assign

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 2);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(p2 && p2.get() == &obj1);

		p2 = p1; // pointer to same object

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 2);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(p2 && p2.get() == &obj1);

		p2 = n; // to null

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(!p2 && p2.get() == nullptr);

		p2 = p1; // back to object

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 2);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(p2 && p2.get() == &obj1);

		p2 = m; // to moved-from

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(!p2 && p2.get() == nullptr);

		p2 = p1; // back to object

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 2);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(p2 && p2.get() == &obj1);
	}

	// move assign
	{
		Object obj1, obj2;
		Ptr p1(&obj1), p2(&obj2), n, m;
		{
			Ptr tmp = std::move(m);
		}

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(p2 && p2.get() == &obj2);

		p2 = std::move(p1);

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(!p1 && p1.get() == nullptr);
		EXPECT_TRUE(p2 && p2.get() == &obj1);

		p2 = std::move(p2); // self assign

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(!p1 && p1.get() == nullptr);
		EXPECT_TRUE(p2 && p2.get() == &obj1);

		p1 = p2;

		p2 = std::move(p1); // pointer to same object

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(!p1 && p1.get() == nullptr);
		EXPECT_TRUE(p2 && p2.get() == &obj1);

		p1 = p2;

		p2 = std::move(n); // to null

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(!p2 && p2.get() == nullptr);

		p2 = p1; // back to object

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 2);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(p2 && p2.get() == &obj1);

		p2 = std::move(m); // to moved-from

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(!p2 && p2.get() == nullptr);

		p2 = p1; // back to object

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 2);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p1 && p1.get() == &obj1);
		EXPECT_TRUE(p2 && p2.get() == &obj1);
	}

	// reset
	{
		Object obj1, obj2;
		Ptr p(&obj1);

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(p && p.get() == &obj1);

		// other pointer
		p.reset(&obj2);

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(p && p.get() == &obj2);

		// same pointer
		p.reset(&obj2);

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(p && p.get() == &obj2);

		// non-null -> null
		p.reset(nullptr);

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(!p && p.get() == nullptr);

		// null -> null
		p.reset(nullptr);

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(!p && p.get() == nullptr);

		// null -> non-null
		p.reset(&obj2);

		EXPECT_TRUE(obj1.dangling_ptrs_.size() == 0);
		EXPECT_TRUE(obj2.dangling_ptrs_.size() == 1);
		EXPECT_TRUE(p && p.get() == &obj2);
	}
END_TEST()
