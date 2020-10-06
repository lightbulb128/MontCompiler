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

int onlyLexer(const char* filename){
    MontLexer lexer(true);
    ifstream fileReader(filename);
    lexer.setStream(&fileReader);
    while (true) {
        Token t = lexer.nextToken();
        cout << t;
        if (t.tokenKind == TK_LBRACE || t.tokenKind == TK_SEMICOLON || 
            t.tokenKind == TK_RBRACE || t.tokenKind == TK_EOF) 
            cout << endl;
        if (t.tokenKind == TK_EOF) break; 
        if (t.tokenKind == TK_ERROR) {
            cerr << "Lexer message: " << endl;
            cerr << lexer.getErrorInfo();
            break;
        }   
    }
    fileReader.close();
    return 0;
}


int onlyParser(const char* filename){
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
        return 0;
    } else {
        cout << "Parsed: " << endl;
        cout << parser; 
    }
    return 0;
}

int onlyConceiver(const char* filename) {
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
    } else {
        cout << conceiver;
    }
    return 0;
}

int main(int argc, const char* argv[]){
    if (argv[1][0] == 'l') return onlyLexer(argv[2]);
    else if (argv[1][0] == 'p') return onlyParser(argv[2]);
    else if (argv[1][0] == 'c') return onlyConceiver(argv[2]);
    else if (argv[1][0] == 'a') return compile(argv[2]) ? 0 : -1;
    else return compile(argv[1]) ? 0 : -1;
}