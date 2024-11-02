import { createRequire } from 'node:module'

export function getSupportedLanguages(): string[] {
	const addon = getAddonInstance()

	return addon.getSupportedLanguages()
}

export function createWindowsSpellChecker(language: string) {
	const addon = getAddonInstance()

	const instance = addon.createWindowsSpellChecker(language) as WindowsSpellChecker

	let disposed = false

	const wrappedInstance: WindowsSpellChecker = {
		testSpelling: (word: string) => {
			ensureNotDisposed()
			ensureIsString(word)

			return instance.testSpelling(word)
		},

		getSpellingSuggestions: (word: string) => {
			ensureNotDisposed()
			ensureIsString(word)

			return instance.getSpellingSuggestions(word)
		},

		addWord: (word: string) => {
			ensureNotDisposed()
			ensureIsString(word)

			instance.addWord(word)
		},

		removeWord: (word: string) => {
			ensureNotDisposed()
			ensureIsString(word)

			instance.removeWord(word)
		},

		dispose: () => {
			// Ensure the instance's `dispose` method is never called
			// more than once, to prevent memory corruption
			if (!disposed) {
				instance.dispose()
			}
		}
	}

	function ensureNotDisposed() {
		if (disposed) {
			throw new Error(`Windows spell checker instance has been disposed`)
		}
	}

	function ensureIsString(str: string) {
		if (typeof str !== 'string') {
			throw new Error(`Parameter ${str} is not a string`)
		}
	}

	return wrappedInstance
}

export function isAddonAvailable() {
	try {
		//console.log(`Trying to create addon..`)
		const addon = getAddonInstance()

		//console.log(`Trying to call 'isAddonLoaded'..`)
		const result = addon.isAddonLoaded()

		return result === true
	} catch (e) {
		return false
	}
}

function getAddonInstance() {
	return createRequire(import.meta.url)('../addon/WindowsSpellChecker.node')
}

export interface WindowsSpellChecker {
	testSpelling(word: string): boolean
	getSpellingSuggestions(word: string): string[]
	addWord(word: string): void
	removeWord(word: string): void
	dispose(): void
}
