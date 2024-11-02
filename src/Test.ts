import { createWindowsSpellChecker, getSupportedLanguages, isAddonAvailable } from './Exports.js'

const available = isAddonAvailable()

console.log(available)

const languages = getSupportedLanguages()

console.log(languages)

const checker = createWindowsSpellChecker('en-US')

console.log(checker.testSpelling('Hello'))
console.log(checker.testSpelling('Hellow'))

console.log(checker.testSpelling('遺事可堪解頤'))

//checker.addWord('Hellow')
//checker.removeWord('Hellow')

console.log(checker.testSpelling('Hellow'))

console.log(checker.getSpellingSuggestions('Hellow'))

checker.dispose()

const x = 0
