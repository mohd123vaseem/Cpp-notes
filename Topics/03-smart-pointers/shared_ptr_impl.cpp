#include <iostream>

using namespace std;

class shared_ptr
{

    int *ref_cnt;

public:
    int *ptr;

    // normal constructor.
    shared_ptr(int *p)
    {
        ptr = p;

        ref_cnt = new int(1);
        cout << "shared_ptr " << *ptr << " has ref count " << *ref_cnt << endl;
    }

    // copy constructor
    shared_ptr(const shared_ptr &other)
    {
        this->ptr = other.ptr;
        this->ref_cnt = other.ref_cnt;
        *(this->ref_cnt) += 1;
        cout << "COPY shared_ptr " << *(this->ptr) << " has ref count " << *(this->ref_cnt) << endl;
    }

    // Assignment constructor
    shared_ptr &operator=(const shared_ptr &other)
    {

        if (this == &other)
            return *this; // bail early on self assignment.

        if (*this->ref_cnt == 1)
        {   
            cout<<"DELETE BEFORE ASSIGNMENT  "<<this->ptr<<" ,has value "<<*this->ptr<<" ,and ref_count as "<<*this->ref_cnt<<endl;
            delete this->ptr;
            delete this->ref_cnt;
        }
        else
        {    
            cout<<"DECREMENT REF_COUNT BEFORE ASSIGNMENT "<<this->ptr<<" ,has value "<<*this->ptr<<" ,and ref_count as "<<*this->ref_cnt<<endl;
            *this->ref_cnt -= 1;
        }

        this->ptr = other.ptr;
        this->ref_cnt = other.ref_cnt;
        *this->ref_cnt += 1;
        cout<<"ASSIGNMENT done "<<this->ptr<<" ,has value "<<*this->ptr<<" ,and ref_count as "<<*this->ref_cnt<<endl;

        return *this;
    }

    ~shared_ptr()
    {
        if (*ref_cnt != 1)
        {
            cout << "shared_ptr " << *ptr << " has ref count " << *ref_cnt << " ,so NO DELETE" << endl;
            *ref_cnt -= 1;
            return;
        }
        cout << "\n";
        cout << "shared_ptr " << *ptr << " has ref count " << *ref_cnt << " ,so we DELETE NOW" << endl;
        delete ptr; // after this heap memo is freed that was occupied by 10.
        // now pointer is dangling ,as it points to add that has no vlaue.
        ptr = nullptr; // now its safe as it does not point to freed memo.
        cout << "ptr is freed along with heap memo" << endl;
        cout << "pointer points to null" << ptr << endl;
        // did same for refcount.
        delete ref_cnt;
        ref_cnt = nullptr;
    }
};

int main()
{

    cout << "hello world" << endl;
    int *temp_ptr1 = new int(10);
    int *temp_ptr2 = new int(123);

    shared_ptr p1 = shared_ptr(temp_ptr1);
    shared_ptr p2 = p1;//coping the p1 ptr
    // shared_ptr p3 = p1;
    shared_ptr p4 = shared_ptr(temp_ptr2);
    p4 = p1;//assigning the ptr p1 to p4.

    cout << "value of pointer is " << *(p1.ptr) << " ,Address of value is " << (p1.ptr)<< endl;
    cout << "value of pointer is " << *(p2.ptr) << " ,Address of value is " << (p2.ptr) << endl;
    // cout << "value of pointer is " << *(p3.ptr) << " ,Address of value is " << (p3.ptr) << endl;
    cout << "\n";
    cout << "value of pointer is " << *(p4.ptr) << " ,Address of value is " << (p4.ptr) << endl;
    // cout << "value of pointer is " << *(p5.ptr) << " ,Address of value is " << (p5.ptr) << endl;
    cout << "\n";
    return 0;
}
