#include<iostream>

using namespace std;

class shared_ptr{

    public:
    int* ptr;
    shared_ptr(int *p){
          ptr = p;
    }

    ~shared_ptr(){
       delete ptr;//after this heap memo is freed that was occupied by 10.
       //now pointer is dangling ,as it points to add that has no vlaue.
       ptr=nullptr;//now its safe as it does not point to freed memo. 
       cout<<"ptr is freed along with heap memo of 10"<<endl;
       cout<<"pointer points to null"<<ptr<<endl;
    }
};

int main()
{
    cout<<"hello world"<<endl;
    int* temp_ptr= new int(10);
    shared_ptr p=shared_ptr(temp_ptr);
    cout<<"Addess of value is "<<(p.ptr)<<endl;
    cout<<"value of pointer is "<<*(p.ptr)<<endl;
    return 0;
}
