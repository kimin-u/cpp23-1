//display 19.1

#include <iostream>
#include <vector>

using namespace std;

int main()
{
    vector<int> container;
    for(int i =1; i<=4; i++){
        container.push_back(i);
    }

    cout<<"Here is what is in the container:\n";
    vector<int>::iterator p;
    for(p= container.begin(); p!=container.end(); p++)
        cout<<*p<<" ";
    cout<<endl;

    for (int i=0; i<container.size(); i++)
        cout<<container[i]<<" ";
    cout<<endl;

    auto iter = container.begin();
    cout<<*iter<<endl;

    cout << "Setting entries to 0:\n";
    for ( p= container.begin(); p!=container.end();p++)
        *p=0;

    cout<<"Container now contains:\n";
    for(p= container.begin(); p!=container.end(); p++)
        cout<<*p<<" ";
    cout<<endl;

    return 0;
}