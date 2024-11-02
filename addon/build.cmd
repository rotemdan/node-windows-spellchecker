dlltool --dllname node.exe --def ../definitions/node_api.def --output-lib node_api.lib

cl.exe WindowsSpellChecker.cpp node_api.lib ole32.lib /I"../include/napi" /Fe"WindowsSpellChecker.node" /std:c++17 /EHsc /O1 /LD

del *.obj *.exp WindowsSpellChecker.lib
