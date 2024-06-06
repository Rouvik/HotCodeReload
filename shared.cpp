#include <iostream>

extern "C"
{
    void printText(int a, int b)
    {
        for (int i = 0; i < a; i++)
        {
            std::cout << b << '\n';
        }
        
        // std::cout << "Modulus: " << (a % b) << '\n';
    }
}