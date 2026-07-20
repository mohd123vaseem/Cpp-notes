#include<iostream>

using namespace std;

class person{

    public:
    string *name;

    person(string s){
        name=new string(s);
    }
    
    //copy contructor
    person(const person&other){

        name = new string(*other.name);

    }

    ~person(){

        delete name;
    }

    //Assignment operator
    //this is a ptr to person obj hence we return obj by deferencing it like this : *this
    person& operator=(const person &other ){
           
        if(this==&other)return *this;
        
        *this->name = *other.name;

        return *this;
    }
};

int main()
{
    /* code */
    person p1("Vaseem");
    
    person p2=p1;

    person p3("Ali");
    cout<<"Address of p1: "<<p1.name<<", Address of p2: "<<p2.name<<endl;
    
    cout<<"name of p1: "<<*p1.name<<", name of p2: "<<*p2.name<<endl;

    *p2.name = "Naseem";
    cout<<"name of p3: "<<*p3.name<<endl;

    cout<<"name of p1: "<<*p1.name<<", name of p2: "<<*p2.name<<endl;
    
    p3=p1;
    cout<<"name of p3: "<<*p3.name<<endl;
    return 0;
}
