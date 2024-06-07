#include <iostream>
#include "hotreload.hpp"

int main()
{
    HotLibraryMaintainer maintainer("libshared.dll");
    maintainer.addSymbol("printText");
    if (maintainer.loadSymbols())
    {
        std::cout << "Failed to load symbols\n";
        return 1;
    }

    typedef void (*mathFunc_t)(int, int);

    mathFunc_t mathFunc = reinterpret_cast<mathFunc_t>(maintainer.getSymbolPointer("printText"));

    char choice;
    while (choice != 'q')
    {
        std::cout << "Enter: ";
        std::cin >> choice;
        std::cout << '\n';

        if (choice == 'r')
        {
            std::cout << "Reloading...\n";
            if (maintainer.reloadLibrary())
            {
                std::cerr << "Failed to reload!\n";
            }

            mathFunc = maintainer.getSymbolPointer("printText");
            continue;
        }
        else if (choice == 'c')
        {
            std::cout << "Closing library handle...\n";
            maintainer.releaseLibrary();
        }

        if (maintainer.isLoaded())
        {
            mathFunc(5, 2);
        }
    }
}