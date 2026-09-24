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
        delete []data;
        data = nullptr;
        size = 0;
        capacity = 0;
    }

    void push_back(int val)
    {

        int *temp = new int[size];
        if (capacity == size)
        {
            capacity = size * 2 + 1;
            if (size != 0)
            {
                for (int i = 0; i < size; i++)
                {
                    temp[i] = data[i];
                }
                delete[] data;
            }
            data = new int[capacity];

            for (int i = 0; i < size; i++)
            {
                data[i] = temp[i];
            }
        }

        delete[] temp;
        data[size] = val;
        size++;
    }

    int &operator[](int index)
    {

        if (index >= size)
        {
            throw out_of_range("index out of range");
        }
        return data[index];
    }

    int get_size(){
        return size;
    }
    int get_capacity(){
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
    v.~my_vector();
    return 0;
}
