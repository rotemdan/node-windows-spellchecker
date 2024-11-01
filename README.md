# Minimalist Node.js binding to the Windows spell-checker

Uses N-API to bind to the Windows native spell-checker:

* Only a minimal set of operations are implemented: `testSpelling`, `getSpellingSuggestions`, `addWord` and `removeWord` (all apply to a single word only)
* Addon binary is pre-bundled. Doesn't require any install-time scripts
* Uses the stable `napi.h` C++ API (with `NAPI_VERSION = 8`). It should not generally require recompilation for new different Node.js versions
* Should work in different versions of Electron.js without recompilation
* Does **not** use `node-gyp`, or any form of build automation like `MAKE` or `CMAKE`. Only a simple `.cmd` file that builds `node_api.lib` using `dlltool` and the N-API addon using `cl.exe` (MSVC)

## Usage Example
```
npm install windows-spellchecker-minimal
```

```ts
import { createWindowsSpellChecker, getSupportedLanguages } from 'windows-spellchecker-minimal'

console.log(getSupportedLanguages())
// Output: [ 'en-US', 'fr-FR', 'es-ES' ]

const checker = createWindowsSpellChecker('en-US')

console.log(checker.testSpelling('Hello'))
// Output: true

console.log(checker.testSpelling('Hellow'))
// Output: false

console.log(checker.getSpellingSuggestions('Hellow'))
// Output: [
//   'Hallow', 'Hollow', 'Hello', 'Hellos', 'Hallows',
//   'Henlow', 'Mellow', 'Yellow'
// ]

checker.dispose()
```

## Adding and removing words

Adding and removing words is supported by the Windows API, but the added word would persist in the Windows dictionary forever, for the current user. That may not be desired for all applications.

```ts
checker.addWord('Hellow')
```

`removeWord` is only supported on Windows 10 and above, and would fail on Windows 8 (though [Windows 8 has officially reached EOL](https://support.microsoft.com/en-us/windows/windows-8-1-support-ended-on-january-10-2023-3cfd4cde-f611-496a-8057-923fba401e93#:~:text=As%20a%20reminder%2C%20Windows%208.1,will%20no%20longer%20be%20provided.) and is not supported by latest Node.js):
```ts
checker.removeWord('Hellow')
```

The Windows custom dictionary, containing the added word, is located at:
```
%userprofile%\AppData\Roaming\Microsoft\Spelling\neutral\default.dic
```

**Note**: for unknown reasons, a call to `addWord` doesn't instantly register the word via the Windows API. If you call `addWord('foobar')` and then immediately call `testSpelling('foobar')`, you may get `false` even though the word was successfully added to the system dictionary.

## Building the N-API addon

The library is bundled with a pre-built addons, so no recompilation should be needed.

If you still want to compile yourself, for a modification or a fork:

* Install Visual Studio 2022 build tools
* Install [`msys2`](https://www.msys2.org/) and its x64 `binutils` package (`pacman -S mingw-w64-x86_64-binutils`)
* Ensure you have `C:\msys64\mingw64\bin` in path
* Use `x64 Native Tools Command Prompt for VS 2022` to ensure x64 VS2022 build tools are available in path
* In the `addon/` directory, run `build.cmd`. It will build a `.node` addon using `cl.exe` (MSVC), as well as its `node_api.lib` dependency using `dlltool`


## License

MIT
