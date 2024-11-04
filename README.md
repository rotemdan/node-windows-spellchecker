# Node.js binding to the Windows spell-checker

Uses N-API to bind to the Windows native spell-checker:

* Only a minimal set of operations are currently implemented: `testSpelling`, `getSpellingSuggestions`, `addWord` and `removeWord` (all apply to a single word only)
* Addon binary is pre-bundled. Doesn't require any install-time scripts
* Uses the stable `napi.h` C++ API (with `NAPI_VERSION = 8`). It should not generally require recompilation for new different Node.js versions
* Should work in different versions of Electron.js without recompilation
* Supports both x64 and arm64 versions of Windows

## Usage example
```
npm install windows-spellchecker
```

```ts
import { createWindowsSpellChecker, getSupportedLanguages } from 'windows-spellchecker'

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

The library is bundled with a pre-built addon, so recompilation shouldn't be needed.

If you still want to compile yourself, for a modification or a fork:

* Install Visual Studio 2022 build tools
* In the `addons` directory, run `npm install`, which would install the necessary build tools. Then run `npm run build-x64`
* To cross-compile for arm64 go to `Visual Studio Installer -> Individual components`, and ensure `MSVC v143 - VS 2022 C++ ARM64 build tools (latest)` is checked. Then run `npm run build-arm64`
* Resulting binaries should be written to the `addons/bin` directory

## License

MIT
