#include <iostream>

using namespace std;

template <typename T>

class SharedPtr
{

private:
    int *ref_cnt;
    T *ptr;

public:
    // normal constructor.
    SharedPtr(T *p)
    {
        ptr = p;

        ref_cnt = new int(1);
        // cout << "SharedPtr " << *ptr << " has ref count " << *ref_cnt << endl;
    }

    // copy constructor
    SharedPtr(const SharedPtr &other)
    {
        this->ptr = other.ptr;
        this->ref_cnt = other.ref_cnt;
        *(this->ref_cnt) += 1;
        cout << "COPY SharedPtr " << *(this->ptr) << " has ref count " << *(this->ref_cnt) << endl;
    }

    // destructor
    ~SharedPtr()
    {
        if (ref_cnt == nullptr) return;   // ✅ moved-from object → nothing to release

        if (*ref_cnt != 1)
        {
            // cout << "SharedPtr " << *ptr << " has ref count " << *ref_cnt << " ,so NO DELETE" << endl;
            *ref_cnt -= 1;
            return;
        }
        cout << "\n";
        // cout << "SharedPtr " << *ptr << " has ref count " << *ref_cnt << " ,so we DELETE NOW" << endl;
        delete ptr; // after this heap memo is freed that was occupied by 10.
        // now pointer is dangling ,as it points to add that has no vlaue.
        ptr = nullptr; // now its safe as it does not point to freed memo.
        cout << "ptr is freed along with heap memo" << endl;
        cout << "pointer points to null" << ptr << endl;
        // did same for refcount.
        delete ref_cnt;
        ref_cnt = nullptr;
    }

    // copy assignment (operator overloading)
    SharedPtr &operator=(const SharedPtr &other)
    {

        if (this == &other)
            return *this; // bail early on self assignment.

        if (*this->ref_cnt == 1)
        {
            cout << "DELETE BEFORE ASSIGNMENT  " << this->ptr << " ,has value " << *this->ptr << " ,and ref_count as " << *this->ref_cnt << endl;
            delete this->ptr;
            delete this->ref_cnt;
        }
        else
        {
            cout << "DECREMENT REF_COUNT BEFORE ASSIGNMENT " << this->ptr << " ,has value " << *this->ptr << " ,and ref_count as " << *this->ref_cnt << endl;
            *this->ref_cnt -= 1;
        }

        this->ptr = other.ptr;
        this->ref_cnt = other.ref_cnt;
        *this->ref_cnt += 1;
        cout << "ASSIGNMENT done " << this->ptr << " ,has value " << *this->ptr << " ,and ref_count as " << *this->ref_cnt << endl;

        return *this;
    }

    // move constructor
    SharedPtr(SharedPtr &&other) noexcept
    {
        this->ptr = other.ptr;
        this->ref_cnt = other.ref_cnt;

        other.ptr = nullptr;
        other.ref_cnt = nullptr;
    }

    // move assignment operator
    SharedPtr &operator=(SharedPtr &&other) noexcept
    {
        if (this == &other)
            return *this;

        // release what THIS currently owns (same as copy-assign / destructor)
        if (*this->ref_cnt == 1)
        {
            delete this->ptr;
            delete this->ref_cnt;
        }
        else
        {
            *this->ref_cnt -= 1;   // ✅ decrement the COUNT (the value), not the pointer
        }

        // steal other's resources
        this->ptr = other.ptr;
        this->ref_cnt = other.ref_cnt;

        // null the source so its destructor is a harmless no-op
        other.ptr = nullptr;
        other.ref_cnt = nullptr;

        return *this;   // no increment — moving keeps the same owner count
    }

    // deference operator overloading
    T &operator*()
    {
        return *this->ptr;
    }

    T *operator->()
    {
        return ptr;
    }

    int get_count() const
    {
        return *this->ref_cnt;
    }
};

struct Person
{
    string name;
    int age;

    Person(string name, int age)
    {
        this->name = name;
        this->age = age;
    }
};

int main()
{

    cout << "hello world" << endl;

    SharedPtr<Person> p = new Person("vaseem", 23);

    cout << "name is " << p->name << " ,age is " << p->age << endl;

    int *temp_ptr1 = new int(10);
    int *temp_ptr2 = new int(123);

    SharedPtr p1 = SharedPtr(temp_ptr1);
    SharedPtr p2 = p1; // copying the p1 ptr
    SharedPtr p4 = SharedPtr(temp_ptr2);
    p4 = p1; // copy-assigning p1 to p4

    cout << "value of pointer is " << (*p1) << " ,and cnt for this ptr is " << p1.get_count() << endl;
    cout << "value of pointer is " << (*p2) << endl;
    cout << "\n";
    cout << "value of pointer is " << (*p4) << endl;
    cout << "\n";

    // ---------------- MOVE constructor test ----------------
    cout << "=== MOVE CONSTRUCTOR ===\n";
    SharedPtr<int> m1(new int(555));       // count = 1
    SharedPtr<int> m2 = std::move(m1);     // move ctor: m2 steals from m1, m1 is now hollow
    cout << "m2 value = " << (*m2) << ", count = " << m2.get_count() << "\n";
    // m1 is moved-from (hollow) — do NOT read *m1; it will be safely destroyed at scope end.

    // ---------------- MOVE assignment test ----------------
    cout << "\n=== MOVE ASSIGNMENT ===\n";
    SharedPtr<int> m3(new int(777));       // m3 owns 777 (count = 1)
    m3 = std::move(m2);                    // move-assign: m3 releases 777, steals 555 from m2
    cout << "m3 value = " << (*m3) << ", count = " << m3.get_count() << "\n";
    // m2 is now moved-from (hollow); m3 owns 555.

    cout << "\n(program ending — destructors run, moved-from hollow objects are safe)\n";
    return 0;
}
