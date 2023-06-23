#include <iostream>
#include <map>
#include <string>

using namespace std;

int main()
{
    map<string, string> planets;

    planets["Mercury"] = "Hot planet";
    planets["Venus"] = "acid";
    planets["Earth"] = "home";

    if(planets.find("Venus") !=  planets.end())
        cout<<planets.find("Venus")->first<<planets.find("Venus")->second<<endl;
    if (planets.find("jup") == planets.end())
        cout<<"존재x"<<endl;
    for(auto p : planets){
        cout<<p.first<<" "<<p.second<<endl;
    }
    return 0;
}

