#include <iostream>
#include <regex>

using namespace std;

int main(){
    regex(".*");
    cout<<regex_search("<init>", regex(".*"))<<endl;
    return 0;    
}