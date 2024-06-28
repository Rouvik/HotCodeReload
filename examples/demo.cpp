/*
MIT License

Copyright (c) 2024 Rouvik Maji

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>

#define HOT_LOGGING_ENABLE // enable logging
#include "../hotreload.hpp"

int main()
{
    // initialise the dll with the HotLibraryMaintainer this follows RAII so the resource is immediately consumed
    HotLibraryMaintainer maintainer("libshared.dll");
    maintainer.addSymbol("printText"); // add the symbol to find for in DLL each time it is reloaded
    if (maintainer.loadSymbols()) // load symbols and check for errors
    {
        std::cout << "Failed to load symbols\n";
        return 1;
    }

    void (*mathFunc)(int, int); // initialise a function pointer to store the symbol in
    
    // The following statement is to fool the compiler to ignore ISO C type safety for that particular assignment
    //
    // tldr;
    //
    // This weird looking cast is a ISO C requirement to cast a function pointer from FARPROC (aka void * or size_t)
    // to a desired function pointer type, here void func(int, int) type
    // However, ISO C doesnot directly allow casts like void (*func)(int, int) = (void (*)(int, int))(maintainer.getSymbol("..."))
    // This will throw an error -fpermissive, thus instead we use this method, to make a valid typesafe pointer to pointer allocation in ISO C
    // Here as you can see, we first take the address of mathFunc as (&athFunc) and then dereference it with *(&mathFunc) effectly asking the
    // compiler to assign the pointer type as a value to mathFunc without type checking, the compiler happily accepts the cast
    // (void (*)(int, int)) as it thinks it actually assigns the pointer to pointer with the value FARPROC and dereferences it
    //
    // If reading all of that didnt make sense to you, just make sure to use this practise in your code base if your compiler
    // throws -fpermissive errors, dont go for -fpermissive flag, it will TURN OFF ALL YOUR TYPE SAFETY yikes!
    *(&mathFunc) = (void (*)(int, int))(maintainer.getSymbolPointer("printText"));

    // example code below -----------------------------------------------

    char choice;
    while (choice != 'q')
    {
        std::cout << "Enter: ";
        std::cin >> choice;
        std::cout << '\n';

        if (choice == 'r') // reload the dll, also, the reloadLibrary function will unbind and rebind the library if not closed manually
        {
            std::cout << "Reloading...\n";
            if (maintainer.reloadLibrary())
            {
                std::cerr << "Failed to reload!\n";
            }

            *(&mathFunc) = (void (*)(int, int))maintainer.getSymbolPointer("printText"); // reinitialise the pointer for this example
            continue;
        }
        else if (choice == 'c') // close the library handle, allow for compilation of the DLL
        {
            std::cout << "Closing library handle...\n";
            maintainer.releaseLibrary();
        }

        if (maintainer.isLoaded()) // just check if the library is loaded, not required if you are sure about it
        {
            mathFunc(5, 2);
        }
        else
        {
            std::cout << "Please reload the library to continue...\n";
        }
    }
}