# HotCodeReload
A simple header only library to enable "Windows" C++ devs to use hot code reloading in their projects

# For a quick read about how this might work:
Hot Code Reloading is a feature that enables developers to change certain parts of code without directly recompiling the entire binary
This sort of a maneuver is usually possiblefor interpreted languages only most notable C# which is made possible due to their VM being able to swap the old code with new code
while keeping the memory intact. This is however not possible for our case.

To actually make this happen we begin with putting all out reloadable code in a seperate `shared` library and then using the library to open a handle to this library from the main program.
The library class `HotLibraryManager` will automatically register the shared library, open a handle upon initialisation, then we set the `symbols` we are interested in using the `HotLibraryMaintainer::addSymbol()` function
this adds the symbol to an internal table, then call `HotLibraryManager::loadSymbols()` to load all the raw pointers to the internal structure
finally we can get to the pointers with `HotLibraryManager::getSymbolPointer()` to get a pointer to it
which can be cast to whatever type we wish and used however we want!

# An even quicker code example:

## Load a library and use it:
``` C++
HotLibraryMaintainer maintainer("libshared.dll");
maintainer.addSymbol("mathFunc");
if(maintainer.loadSymbols())
{
    // something went wrong
    return 1;
}

typedef void (*mathFunc_t)(int, int) // some function pointer that takes 2 ints as parameters

mathFunc_t func = reinterpret_cast<mathFunc_t>(maintainer.getSymbolPointer("mathFunc");
func(5, 2); // output depends on the implementation libshared.dll
```

## How to implement the hot code reload:
``` C++
HotLibraryMaintainer maintainer("libshared.dll");
maintainer.addSymbol("mathFunc");
if (maintainer.loadSymbols())
{
  return 1;
}

typedef void (*mathFunc_t)(int, int); // a function pointer with 2 ints as parameters

mathFunc_t mathFunc = reinterpret_cast<mathFunc_t>(maintainer.getSymbolPointer("mathFunc"));

// here is where we implement hot code reloading ------------------------
// about choices
// for 'q' the program will exit
// for 'c' the library is sort of ejected and is open for updates, this is when we swap the dll
// for 'r' the newly updated dll is reloaded
// for any other choice '*' the code will execute mathFunc with (5, 2) as arguments

char choice;
while (choice != 'q')
{
  std::cout << "Enter: ";
  std::cin >> choice;
  std::cout << '\n';

  if (choice == 'r') // reload the library with all its symbols
  {
    std::cout << "Reloading...\n";
    if (maintainer.reloadLibrary())
    {
      std::cerr << "Failed to reload!\n";
    }

    mathFunc = maintainer.getSymbolPointer("printText");
    continue;
  }
  // eject the library
  // once ejected, you can do whatever you want to libshared.dll, and recompile it
  // once done editing recompile libshared.dll and use choice 'r' to reload again for execution
  // you will find that the behaviour has changed as per your code
  // also illegal function signatures and missing symbols will cause undefined exceptions so make sure
  // you update all symbols and functions after the edits
  else if (choice == 'c')
  {
    std::cout << "Closing library handle...\n";
    maintainer.releaseLibrary();
  }

  if (maintainer.isLoaded()) // check if the library is loaded or do nothing
  {
    mathFunc(5, 2); // call the function pointer with the arguments (5, 2)
  }
}
```

# Notable issues:
The library compiles fine on most windows platforms but on mingw, g++ disallows casting of FARPROC
pointers, thus on `mingw:g++ and mingw:gcc` you are required to compile it with `-fpermissive` flag
to allow such conversions

# Author
Rouvik Maji [Gmail](mailto:majirouvik@gmail.com)
