#include "montLexer.h"
#include "montParser.h"
#include "montConceiver.h"
#include "montAssembler.h"

#include <iostream>
#include <list>
using namespace std;

bool compile(const char* filename){
    MontLexer lexer(true);
    ifstream fileReader(filename);
    lexer.setStream(&fileReader);
    MontParser parser;
    bool f = parser.parse(lexer);
    fileReader.close();
    if (!f) {
        cerr << "Lexer message: " << endl;
        cerr << lexer.getErrorInfo();
        cerr << "Parser message: " << endl;
        cerr << parser.getErrorInfo();
        return false;
    }
    MontConceiver conceiver; 
    f = conceiver.conceive(parser);
    if (!f) {
        cerr << "Conceiver message: " << endl;
        cerr << conceiver.getErrorInfo();
        return false;
    }
    MontAssembler assembler;
    assembler.setStream(&cout);
    assembler.assemble(conceiver);
    return true;
}

int main(int argc, const char* argv[]){
    compile(argv[1]);
    return 0;
}