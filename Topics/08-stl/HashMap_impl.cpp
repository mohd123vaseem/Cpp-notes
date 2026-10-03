#include <iostream>
#include <vector>
#include<list>

using namespace std;

class MyHashMap
{

    vector<list<pair<int,int>>> nums;
    // vector<bool> taken;

public:
    MyHashMap(){
        nums = vector<list<pair<int,int>>>(10);
        // taken = vector<bool>(10,false);
    }

    int hash_key(int key){

        return (key%10);
    }

    void put(int key,int value){

        int key_hash = hash_key(key);
        // if(taken[key_hash])return;

        // taken[key_hash]=true;
        
        int fl=0;
        for(auto &[k,v] : nums[key_hash]){

            if(k==key){
                v=value;
                fl=1;
            }
        }

        if(fl==0){
            nums[key_hash].push_back({key,value});
        }

        return;
    }

    int get(int key){

        int key_hash = hash_key(key);


        for(auto &[k,v] : nums[key_hash]){

            if(k==key){
                return v;
            }
        }

        return -1;
    }

    void remove(int key){
        
        int key_hash = hash_key(key);
        // taken[key_hash]=false;
        // for(auto p : nums[key_hash]){

        //     if(p.first==key){
        //         nums[key_hash].erase(p);
        //     }
        // }
        for (auto it = nums[key_hash].begin(); it != nums[key_hash].end(); ++it) {
              if (it->first == key) {
              nums[key_hash].erase(it);
              break;
            }
        }

        return;
    }
};

int main()
{
    MyHashMap mp;

    mp.put(1,10);
    mp.put(11,110);
    mp.put(2,20);
    cout<<"value of key 1 : "<<mp.get(1)<<endl;
    cout<<"value of key 11 : "<<mp.get(11)<<endl;
    cout<<"value of key 2 : "<<mp.get(2)<<endl;
    mp.remove(2);
    cout<<"value of key 2 : "<<mp.get(2)<<endl;

    return 0;
}
