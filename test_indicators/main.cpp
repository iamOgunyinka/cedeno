/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

void split_string_by_delimiter( const std::string &str, 
                                const char &delimiter,
                                std::vector<std::string> &result){
    size_t end_index = 0;
    size_t start_index = 0;
    while (end_index != std::string::npos)
    {   
        end_index = str.find_first_of(delimiter,start_index);
        result.push_back(str.substr(start_index, end_index - start_index));
        start_index = end_index+1;
    }
    for(auto &str : result){
        std::cout<<str<<std::endl;
    }   
}

int main(void)
{
    std::vector<std::string> result;
    split_string_by_delimiter( "1,2,3,4,5,6",
                                ',',
                                result);
    return 0;
}
