g++ montLexer.cpp montParser.cpp montConceiver.cpp montAssembler.cpp montCompiler.cpp -o test
test %1 testcode.txt
@ del test.exe