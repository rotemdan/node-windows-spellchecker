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
checker.removeWord('Hellow')
```

(`removeWord` is only supported on Windows 10 and above, and would fail on Windows 8)

The Windows custom dictionary, containing the added word, is located in:
```
%userprofile%\AppData\Roaming\Microsoft\Spelling\neutral\default.dic
```

**Note**: for unknown reason, a call to `addWord` doesn't instantly register the word via the Windows API. If you call `addWord('foobar')` and then immediately call `testSpelling('foobar')`, you may get `false` even though the word was successfully added to the system dictionary.

## Building the N-API addon

See the instructions [here](docs/Building.md).

## License

MIT
