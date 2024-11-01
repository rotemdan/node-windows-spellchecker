import { createWindowsSpellChecker, getSupportedLanguages } from './Exports.js'

const languages = getSupportedLanguages()

console.log(languages)

const checker = createWindowsSpellChecker('en-US')

console.log(checker.testSpelling('Hello'))
console.log(checker.testSpelling('Hellow'))

//checker.addWord('Hellow')
//checker.removeWord('Hellow')

console.log(checker.testSpelling('Hellow'))

console.log(checker.getSpellingSuggestions('Hellow'))

checker.dispose()

const x = 0
