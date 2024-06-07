#include <iostream>

extern "C"
{
    void printText(int a, int b)
    {
        std::cout << "Add: " << (a + b) << '\n';
    }
}