#include <iostream>
#include <vector>
#include <windows.h>
#include <winbase.h>

// WINDOWS DEBUG ----------------------------------
#define WIN_ERR_MSG_SIZE 512
void WPrintLastErrorMessage(HINSTANCE instance)
{
    DWORD dwErr;
    CHAR wmsgBuf[WIN_ERR_MSG_SIZE];
    DWORD dwNumChars;
    dwNumChars = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
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

    struct SymbolContainer
    {
        FARPROC pointer;
        LPCSTR symbolName;
    };

    std::vector<struct SymbolContainer> symbols;

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
        symbols.push_back({NULL,
                           (LPCSTR)symbol});
    }

    bool loadSymbols()
    {
        if (library == NULL)
        {
            std::cerr << "Failed to load symbols, library is NULL, please call loadLibrary() first\n";
            return true;
        }

        bool status = false;
        for (struct SymbolContainer &container : symbols)
        {
            container.pointer = loadLibrarySymbol(library, container.symbolName);
            status |= container.pointer == NULL;
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
                std::cerr << "Failed to free library " << libraryName << '\n';
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
        for (int i = 0; i < symbols.size(); i++)
        {
            if (strcmp(symbols.at(i).symbolName, symbolName) == 0)
            {
                return symbols.at(i).pointer;
            }
        }

        return NULL;
    }

    static HINSTANCE getSharedLibrary(const char *libName)
    {
        HINSTANCE library = LoadLibrary((LPCSTR)libName);
        if (library == NULL)
        {
            std::cerr << "Error, failed to load shared library\n";
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
            std::cerr << "Error, failed to get address of function " << symbolName << '\n';
            WPrintLastErrorMessage(library);
        }

        return addr;
    }
};