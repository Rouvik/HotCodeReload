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


#ifndef INCLUDED_HOTRELOAD_HPP
#define INCLUDED_HOTRELOAD_HPP

#include <iostream>
#include <unordered_map>
#include <windows.h>
#include <winbase.h>

// debug logs -------------------------------------
#ifdef HOT_LOGGING_ENABLE

#define log(type, message) \
std::cout << "[" << #type << " LOG] " << message << '\n'

#define err(type, message) \
std::cerr << "[" << #type << " ERROR] " << message << '\n'

#else

#define log(type, message)
#define err(type, message)
#endif

// WINDOWS DEBUG ----------------------------------
#define WIN_ERR_MSG_SIZE 512
void WPrintLastErrorMessage(HINSTANCE instance)
{
    DWORD dwErr;
    CHAR wmsgBuf[WIN_ERR_MSG_SIZE];
    DWORD dwNumChars;
    dwNumChars = FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE |
                                   FORMAT_MESSAGE_IGNORE_INSERTS,
                               instance,
                               dwErr,
                               0,
                               wmsgBuf,
                               WIN_ERR_MSG_SIZE,
                               NULL);
    if (dwNumChars != 0)
    {
        std::cerr << "WINDOWS ERROR: " << wmsgBuf << "\n";
    }
}

// WINDOWS DEBUG ----------------------------------

class HotLibraryMaintainer
{
public:
    LPCSTR libraryName;
    HINSTANCE library;

    std::unordered_map<const char *, FARPROC> symbols;

    HotLibraryMaintainer(const char *_libraryName) : libraryName(_libraryName)
    {
        library = getSharedLibrary(libraryName);
    }

    ~HotLibraryMaintainer()
    {
        releaseLibrary();
    }

    void addSymbol(const char *symbol)
    {
        symbols[symbol] = NULL;
    }

    void removeSymbol(const char *symbol)
    {
        if (symbols.find(symbol) != symbols.end())
        {
            symbols.erase(symbol);
        }
        else
        {
            err(HOT_RELOAD_REMOVE_SYMBOL, "Failed to remove symbol: " << symbol << " from DLL: " << libraryName);
        }
    }

    bool loadSymbols()
    {
        if (library == NULL)
        {
            err(HOT_RELOAD_LOAD_SYMBOLS, "Failed to load symbols, library is NULL, please call loadLibrary() first");
            return true;
        }

        bool status = false;
        for (auto &symbol : symbols)
        {
            symbol.second = loadLibrarySymbol(library, symbol.first);
            status |= symbol.second == NULL;
        }

        return status;
    }

    bool reloadLibrary()
    {
        if (library != NULL)
        {
            FreeLibrary(library);
        }

        library = getSharedLibrary(libraryName);
        if (library == NULL)
        {
            return true;
        }

        return loadSymbols();
    }

    void releaseLibrary()
    {
        if (library != NULL)
        {
            if(!FreeLibrary(library))
            {
                WPrintLastErrorMessage(library);
                err(HOT_RELOAD_RELEASE_LIBRARY, "Failed to free library " << libraryName);
            }
            library = NULL;
        }
    }

    inline bool isLoaded()
    {
        return library != NULL;
    }

    FARPROC getSymbolPointer(const char *symbolName)
    {
        try
        {
            return symbols.at(symbolName);
        }
        catch(const std::exception& e)
        {
            err(HOT_RELOAD_GET_SYMBOL_POINTER, "Failed to find symbol in symbol table error: " << e.what());
            return NULL;
        }
    }

    static HINSTANCE getSharedLibrary(const char *libName)
    {
        HINSTANCE library = LoadLibraryA(libName);
        if (library == NULL)
        {
            err(HOT_RELOAD_GET_SHARED_LIBRARY_STATIC_INTERNALS, "Failed to load shared library");
            WPrintLastErrorMessage(library);
            FreeLibrary(library);
        }

        return library;
    }

    static FARPROC loadLibrarySymbol(HINSTANCE library, const char *symbolName)
    {
        FARPROC addr = GetProcAddress(library, (LPCSTR)symbolName);
        if (addr == NULL)
        {
            err(HOT_RELOAD_LOAD_LIBRARY_SYMBOL_STATIC_INTERNALS, "Failed to get address of function " << symbolName);
            WPrintLastErrorMessage(library);
        }

        return addr;
    }
};

// turn of logging
#undef log
#undef err

#endif // INCLUDED_HOTRELOAD_HPP