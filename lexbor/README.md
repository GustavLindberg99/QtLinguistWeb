# Lexbor

This folder contains the files from [Lexbor](https://github.com/lexbor/lexbor) that are necessary to build lupdate-js. The header files in `include` are downloaded from [here](https://github.com/lexbor/lexbor/tree/master/source/lexbor).

The file `include/lexbor/core/types.h` has been modified to make `lxb_char_t` a typedef for `char` rather than `unsigned char` since most C++ libraries, including Qt and the standard library, use `char`.

The `liblexbor.a` file (for Linux) and the `lexbor.lib` file (for Windows) in `lib` are compiled from Lexbor's sources, as well as the `lexbor.dll` file in the windows-dependencies\lupdate folder. If you need to re-compile them yourself, run the following steps depending on your operating system:

- Linux:

    ```bash
    git clone https://github.com/lexbor/lexbor
    cd lexbor
    cmake . -DLEXBOR_BUILD_TESTS=ON -DLEXBOR_BUILD_EXAMPLES=ON
    make
    ```

- Windows (with MSVC):

    ```bash
    git clone https://github.com/lexbor/lexbor
    cd lexbor
    "C:\Program Files\CMake\bin\cmake.exe" . -DLEXBOR_BUILD_TESTS=ON -DLEXBOR_BUILD_EXAMPLES=ON
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe" lexbor.sln
    ```

    For this to work, you need to download [CMake for Windows](https://cmake.org/download/) and [MSBuild](https://learn.microsoft.com/en-us/visualstudio/msbuild/walkthrough-using-msbuild?view=vs-2022#install-msbuild). The above paths may vary depending on the version of your compiler.