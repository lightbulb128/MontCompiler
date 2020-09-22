#include <iostream>

void foo(const char* c){
    printf(c);
}

int main(){
    const char* hw = "helloworld!\n";
    foo(hw+1);
}