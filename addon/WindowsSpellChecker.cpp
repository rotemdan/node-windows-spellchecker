#include <spellcheck.h>
#include <string>
#include <vector>
#include <codecvt>
#include <sstream>

#include <stdio.h>

#include <napi.h>

///////////////////////////////////////////////////////////////////////////////////////////
// Utility methods
///////////////////////////////////////////////////////////////////////////////////////////
std::u16string wstringToU16string(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;

    return converter.from_bytes(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr));
}

std::wstring u16stringToWstring(const std::u16string& u16string) {
    std::wstring wstring;

    for (char16_t ch : u16string) {
        wstring.push_back(static_cast<wchar_t>(ch));
    }

    return wstring;
}

std::wstring napiStringToWString(const Napi::String& napiString) {
	return u16stringToWstring(napiString.Utf16Value());
}

///////////////////////////////////////////////////////////////////////////////////////////
// Windows spell-checker class
///////////////////////////////////////////////////////////////////////////////////////////
class WindowsSpellChecker {
private:
	ISpellChecker* spellChecker;
	std::wstring language;

	bool isInitialized = false;

public:
	WindowsSpellChecker(const std::wstring& language) : spellChecker(nullptr) {
		this->language = language;
	}

	HRESULT Initialize() {
		if (isInitialized) {
			return 0;
		}

		HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		if (FAILED(hr)) {
			return hr;
		}

		ISpellCheckerFactory* factory;

		hr = CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
		if (FAILED(hr)) {
			return hr;
		}

		hr = factory->CreateSpellChecker(language.c_str(), &spellChecker);
		if (FAILED(hr)) {
			factory->Release();

			return hr;
		}

		factory->Release();

		isInitialized = true;

		return hr;
	}

	bool TestSpelling(const std::wstring& word) const {
		if (!isInitialized) {
			return false;
		}

		IEnumSpellingError* spellingErrors = nullptr;
		HRESULT hr = spellChecker->Check(word.c_str(), &spellingErrors);
		if (FAILED(hr)) {
			return false;
		}

		ISpellingError* spellingError = nullptr;

		HRESULT nextResult = spellingErrors->Next(&spellingError);

		bool returnValue;

		if (nextResult == 0) {
			returnValue = false;
			spellingError->Release();
		} else if (nextResult == 1) {
			returnValue = true;
		} else {
			returnValue = false;
		}

		spellingErrors->Release();

		return returnValue;
	}

	std::vector<std::wstring> GetSpellingSuggestions(const std::wstring& word) const {
		std::vector<std::wstring> suggestions;

		if (!isInitialized) {
			return suggestions;
		}

		IEnumString* suggestedWords = nullptr;
		HRESULT hr = spellChecker->Suggest(word.c_str(), &suggestedWords);
		if (FAILED(hr)) {
			return suggestions;
		}

		LPOLESTR suggestion;
		while (suggestedWords->Next(1, &suggestion, nullptr) == S_OK) {
			suggestions.push_back(suggestion);

			CoTaskMemFree(suggestion);
		}

		suggestedWords->Release();

		return suggestions;
	}

	// AddWord adds a word to the system dictionary.
	// The added word persists in the current user's dictionary, forever.
	//
	// Location is %userprofile%\AppData\Roaming\Microsoft\Spelling\neutral\default.dic
	HRESULT AddWord(const std::wstring& word) {
		return spellChecker->Add(word.c_str());
	}

	// There's no 'Remove' method in ISpellChecker
	// But a 'Remove' method is available in the ISpellChecker2 interface,
	// Though ISpellChecker2 only works on Windows 10 and above, not on Windows 8
	HRESULT RemoveWord(const std::wstring& word) {
		ISpellChecker2 *spellChecker2 = NULL;

		auto hr = spellChecker->QueryInterface(__uuidof(ISpellChecker2), (LPVOID *)&spellChecker2);

		if (SUCCEEDED(hr)) {
			hr = spellChecker2->Remove(word.c_str());

			spellChecker2->Release();
		}

		return hr;
	}

	~WindowsSpellChecker() {
		if (!isInitialized) {
			return;
		}

		if (spellChecker) {
			spellChecker->Release();
			spellChecker = nullptr;
		}

		CoUninitialize();
	}
};

///////////////////////////////////////////////////////////////////////////////////////////
// Language list method
///////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::wstring> getSupportedLanguageList() {
	std::vector<std::wstring> result;

	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (FAILED(hr)) {
		return result;
	}

	ISpellCheckerFactory* factory;

	hr = CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
	if (FAILED(hr)) {
		return result;
	}

	IEnumString* langList;
	hr = factory->get_SupportedLanguages(&langList);

	if (FAILED(hr)) {
		factory->Release();

		return result;
	}

	factory->Release();

	LPOLESTR lang;
	while (langList->Next(1, &lang, nullptr) == S_OK) {
		result.push_back(lang);

		CoTaskMemFree(lang);
	}
	
	CoUninitialize();

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Napi methods
///////////////////////////////////////////////////////////////////////////////////////////
Napi::Value createWindowsSpellChecker(const Napi::CallbackInfo& info) {
	auto env = info.Env();

	auto language = napiStringToWString(info[0].As<Napi::String>());

	auto windowsSpellChecker = new WindowsSpellChecker(language);

	auto errorCode = windowsSpellChecker->Initialize();

	if (FAILED(errorCode)) {
		std::stringstream errorString;
		errorString << "Failed to initialize Windows spell checker. Got error code " << errorCode;

		Napi::Error::New(env, errorString.str()).ThrowAsJavaScriptException();

		return env.Undefined();
	}

	// Build JavaScript result object

	auto resultObject = Napi::Object().New(env);

	auto testSpelling = [windowsSpellChecker](const Napi::CallbackInfo& info) {
		auto env = info.Env();

		auto wordString = napiStringToWString(info[0].As<Napi::String>());

		auto result = windowsSpellChecker->TestSpelling(wordString);

		return Napi::Boolean::New(env, result);
	};

	auto getSpellingSuggestions = [windowsSpellChecker](const Napi::CallbackInfo& info) {
		auto env = info.Env();

		auto wordString = napiStringToWString(info[0].As<Napi::String>());

		auto suggestions = windowsSpellChecker->GetSpellingSuggestions(wordString);

		auto resultNapiArray = Napi::Array::New(env, suggestions.size());

		for (size_t i = 0; i < suggestions.size(); i++) {
			const std::wstring& suggestion = suggestions[i];

			auto suggestionNapiString = Napi::String::New(env, wstringToU16string(suggestion));

			resultNapiArray.Set(i, suggestionNapiString);
		}

		return resultNapiArray;
	};

	auto addWord = [windowsSpellChecker](const Napi::CallbackInfo& info) {
		auto env = info.Env();

		auto wordString = napiStringToWString(info[0].As<Napi::String>());

		auto errorCode = windowsSpellChecker->AddWord(wordString);

		if (FAILED(errorCode)) {
			std::stringstream errorString;
			errorString << "Failed to add word '" << wordString.c_str() << "' to Windows spell checker. Got error code " << errorCode;

			Napi::Error::New(env, errorString.str()).ThrowAsJavaScriptException();
		}

		return env.Undefined();
	};

	auto removeWord = [windowsSpellChecker](const Napi::CallbackInfo& info) {
		auto env = info.Env();

		auto wordString = napiStringToWString(info[0].As<Napi::String>());

		auto errorCode = windowsSpellChecker->RemoveWord(wordString);

		if (FAILED(errorCode)) {
			std::stringstream errorString;
			errorString << "Failed to add remove '" << wordString.c_str() << "' from Windows spell checker. Got error code " << errorCode;

			Napi::Error::New(env, errorString.str()).ThrowAsJavaScriptException();
		}

		return env.Undefined();
	};

	// Note: Should ensure, within a JavaScript wrapper, that 'dispose' is never called more than once.
	// Otherwise a memory corruption may occur.
	auto dispose = [windowsSpellChecker](const Napi::CallbackInfo& info) {
		delete windowsSpellChecker;
	};

	resultObject.Set(Napi::String::New(env, "testSpelling"), Napi::Function::New(env, testSpelling));
	resultObject.Set(Napi::String::New(env, "getSpellingSuggestions"), Napi::Function::New(env, getSpellingSuggestions));
	resultObject.Set(Napi::String::New(env, "addWord"), Napi::Function::New(env, addWord));
	resultObject.Set(Napi::String::New(env, "removeWord"), Napi::Function::New(env, removeWord));
	resultObject.Set(Napi::String::New(env, "dispose"), Napi::Function::New(env, dispose));

	return resultObject;
}

Napi::Array getSupportedLanguages(const Napi::CallbackInfo& info) {
	auto env = info.Env();

	auto languageList = getSupportedLanguageList();

	auto resultNapiArray = Napi::Array::New(env, languageList.size());

	for (size_t i = 0; i < languageList.size(); i++) {
		const std::wstring& language = languageList[i];

		auto languageNapiString = Napi::String::New(env, wstringToU16string(language));

		resultNapiArray.Set(i, languageNapiString);
	}

	return resultNapiArray;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	exports.Set(Napi::String::New(env, "getSupportedLanguages"), Napi::Function::New(env, getSupportedLanguages));
	exports.Set(Napi::String::New(env, "createWindowsSpellChecker"), Napi::Function::New(env, createWindowsSpellChecker));

	return exports;
}

NODE_API_MODULE(addon, Init)
