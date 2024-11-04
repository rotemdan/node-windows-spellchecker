{
    "targets": [
        {
            "conditions": [
                [
                    "OS=='win'",
                    {
                        "sources": ["src/WindowsSpellChecker.cpp"],
                        "include_dirs": [
                            "<!@(node -p \"require('node-addon-api').include\")"
                        ],
                        "libraries": ["ole32.lib"],
                        "defines": ["NAPI_CPP_EXCEPTIONS"],
                        "cflags!": ["-fno-exceptions"],
                        "cflags_cc!": ["-fno-exceptions"],
                        "msvs_settings": {"VCCLCompilerTool": {"ExceptionHandling": 1}},
						"conditions": [
							[
								"target_arch=='x64'",
								{
									"target_name": "windows-x64-spellchecker",
								},
							],
							[
								"target_arch=='arm64'",
								{
									"target_name": "windows-arm64-spellchecker",
								},
							],
						],						
                    },
                ]
            ],
        }
    ]
}
