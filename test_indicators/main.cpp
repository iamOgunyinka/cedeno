#include <variant>
#include <string>
#include <cassert>
#include <iostream>

void function_void(void){
    std::cout<<"function"<<std::endl;
}

void function_str(const std::string &str){
    std::cout<<str<<std::endl;
}

struct strct_t{
    strct_t(){}
    ~strct_t(){}
    static void function(void){
        std::cout<<"struct function"<<std::endl;
    }
};

int main(void){
    strct_t object;
    auto pointer = &object.function;
    // std::variant<void(*)(void), void(*)(const std::string &)> var;
    // var = &object.function;
    // (*std::get<0>(var))(); 
    return 0;
}