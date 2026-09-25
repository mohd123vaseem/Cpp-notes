#include <iostream>

using namespace std;

class my_vector
{
private:
    int *data;
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
        data = new int[capacity]; // ✅ allocate OUR OWN buffer
        for (int i = 0; i < size; i++)
            data[i] = other.data[i]; // ✅ copy the ELEMENTS, not the pointer
    }

    // copy assignment.
    my_vector &operator=(const my_vector &other)
    {

        if (this == &other)return *this;
        delete[] data;
        size = other.size;
        capacity = other.capacity;
        data = new int[capacity];
        for (int i = 0; i < size; i++)
        {
            data[i] = other.data[i];
        }

        return *this;
    }

    void push_back(int val)
    {

        if (capacity == size)
        {
            capacity = size * 2 + 1;
            int *temp = new int[capacity];

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

    int &back()
    {

        if (size == 0)
        {

            throw out_of_range("vector is impty");
        }
        return data[size - 1];
    }

    int &front()
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

    int &operator[](int index)
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
    my_vector v;

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
    cout<<endl;

    // Copy Assignment
    my_vector nums;
    nums = vec;
    nums[0]=1000;
    cout << " Copy Assignment vector nums  : ";
    for (int i = 0; i < nums.get_size(); i++)
    {
        cout << nums[i] << ",";
    }

    return 0;
}
