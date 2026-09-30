#include <iostream>

using namespace std;
template <typename T>

class my_vector
{
private:
    T *data;
    int size;
    int capacity;

public:
    my_vector()
    {
        data = nullptr;
        size = 0;
        capacity = 0;
    }
    ~my_vector()
    {
        delete[] data;
        data = nullptr;
        size = 0;
        capacity = 0;
    }

    // copy constructor.
    my_vector(const my_vector &other)
    {
        size = other.size;
        capacity = other.capacity;
        data = new T[capacity]; // ✅ allocate OUR OWN buffer
        for (int i = 0; i < size; i++)
            data[i] = other.data[i]; // ✅ copy the ELEMENTS, not the pointer
    }

    // copy assignment.
    my_vector &operator=(const my_vector &other)
    {

        if (this == &other)
            return *this;
        delete[] data;
        size = other.size;
        capacity = other.capacity;
        data = new T[capacity];
        for (int i = 0; i < size; i++)
        {
            data[i] = other.data[i];
        }

        return *this;
    }

    // MOVE CONSTRUCTOR — target is brand new, nothing to free
    my_vector(my_vector &&other) noexcept
    {
        data = other.data; // steal
        size = other.size;
        capacity = other.capacity;
        other.data = nullptr; // null the source
        other.size = other.capacity = 0;
    }

    // MOVE ASSIGNMENT — target already owns a buffer
    my_vector &operator=(my_vector &&other) noexcept
    {
        if (this == &other)
            return *this;  // 1. self-check       ← extra
        delete[] data;     // 2. free OUR old buffer  ← extra
        data = other.data; // 3. steal (same as ctor)
        size = other.size;
        capacity = other.capacity;
        other.data = nullptr; // 4. null the source (same as ctor)
        other.size = other.capacity = 0;
        return *this; // 5. return *this     ← extra
    }

    void push_back(T val)
    {

        if (capacity == size)
        {
            capacity = size * 2 + 1;
            T *temp = new T[capacity];

            for (int i = 0; i < size; i++)
            {
                temp[i] = data[i];
            }
            delete[] data;

            data = temp;
        }

        data[size] = val;
        size++;
    }

    void pop_back()
    {
        if (size == 0)
            return;

        size--;

        return;
    }

    T &back()
    {

        if (size == 0)
        {

            throw out_of_range("vector is impty");
        }
        return data[size - 1];
    }

    T &front()
    {

        if (size == 0)
        {

            throw out_of_range("vector is empty");
        }
        return data[0];
    }

    bool empty()
    {

        return (size == 0);
    }

    void clear()
    {

        // delete[] data; bcoz it will make data a dangling pointer.
        size = 0;

        return;
    }

    T &operator[](int index)
    {

        if (index >= size)
        {
            throw out_of_range("index out of range");
        }
        return data[index];
    }

    int get_size() const
    {

        return size;
    }

    int get_capacity() const
    {

        return capacity;
    }
};

int main()
{
    /* code */
    my_vector<int> v;

    for (int i = 1; i < 5; i++)
    {
        v.push_back(i * 5);
    }

    for (int i = 0; i < v.get_size(); i++)
    {
        cout << v[i] << ",";
    }

    cout << "first ele : " << v.front() << endl;
    cout << "last ele : " << v.back() << endl;
    v.pop_back();
    cout << "last ele after pop_back() : " << v.back() << endl;
    cout << "is vector empty : " << v.empty() << endl;
    // v.clear();
    // try
    // {
    //     cout << v.front();
    // }
    // catch (const out_of_range &e)
    // {
    //     cout << "empty: " << e.what()<<endl;
    // }

    // try
    // {
    //     cout << v.back();
    // }
    // catch (const out_of_range &e)
    // {
    //     cout << "empty: " << e.what()<<endl;
    // }
    // cout << "size of vector " << v.get_size() << endl;

    // copy constructor

    // my_vector vec = v;
    my_vector vec(v); // same as above
    cout << " copying vector v into vector vec : ";
    for (int i = 0; i < vec.get_size(); i++)
    {
        cout << vec[i] << ",";
    }
    cout << endl;

    // Copy Assignment
    my_vector<int> nums;
    nums = vec;
    nums[0] = 1000;
    cout << " Copy Assignment vector nums  : ";
    for (int i = 0; i < nums.get_size(); i++)
    {
        cout << nums[i] << ",";
    }
    cout << " vec[0] to prove deep copy  : " << vec[0] << endl;

    // move constructor.
    my_vector x = move(vec);
    // my_vector x(move(vec));
    cout << " move constructor for vector x : ";
    for (int i = 0; i < x.get_size(); i++)
    {
        cout << x[i] << ",";
    }
    cout << endl;
    cout << "vec's size and capacity" << vec.get_size() << "," << vec.get_capacity() << endl;

    // move Assignment.
    my_vector<int> y;
    y = move(x);
    cout << " move Assignment for vector x : ";
    for (int i = 0; i < y.get_size(); i++)
    {
        cout << y[i] << ",";
    }
    cout << endl;
    cout << "X's size and capacity" << x.get_size() << "," << x.get_capacity() << endl;

    cout << "---------------TEMPLATE-TEST-----------------------" << endl;

    my_vector<string> st;

    st.push_back("vaz");
    st.push_back("naz");
    st.push_back("jazz");
    
    cout << "vector of string : ";

    for (int i = 0; i < st.get_size(); i++)
    {
        cout << st[i] << " , ";
    }

    return 0;
}
