/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <optional>

std::optional<int> optional_function(const bool &b){
    if(b)
        return 8;
    return {};
}   

int main(void)
{
    std::cout<<std::is_same_v<double, int><<std::endl;
    return 0;
}
