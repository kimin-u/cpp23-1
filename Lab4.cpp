//display 19.5

#include <iostream>
#include <list>
using namespace std;

int main()
{
    list<int> listobj;
    for( int i =1; i<=3; i++)
        listobj.push_back(i);
    cout<<"List contains:\n";
    list<int>::iterator iter;
    for(iter=listobj.begin(); iter!=listobj.end(); iter++)
        cout<<*iter<<" ";
    cout<<endl;

    cout<<"Setting all entries to 0:\n";
    for (iter=listobj.begin(); iter!=listobj.end(); iter++)
        *iter =0;
    for(iter=listobj.begin(); iter!=listobj.end(); iter++)
        cout<<*iter<<" ";
    cout<<endl;
    return 0;
}