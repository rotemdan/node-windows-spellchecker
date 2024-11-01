# Building the N-API addon

The library uses pre-build addons only, no recompilation should be needed.

If you still want to compile yourself, for a modification or a fork, use these instructions.

## Windows x64

* Install Visual Studio 2022 build tools
* Install [`msys2`](https://www.msys2.org/) and its x64 `binutils` package (`pacman -S mingw-w64-x86_64-binutils`)
* Ensure you have `C:\msys64\mingw64\bin` in path
* Use `x64 Native Tools Command Prompt for VS 2022` to ensure x64 VS2022 build tools are available in path
* In the `addon/` directory, run `build.cmd`. It will build a `.node` addon using `cl.exe` (MSVC), as well as its `node_api.lib` dependency using `dlltool`
