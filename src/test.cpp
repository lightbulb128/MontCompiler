#include "montLexer.h"
#include "montParser.h"
#include "montConceiver.h"
#include "montAssembler.h"

#include <iostream>
#include <list>
using namespace std;

int compile(){
    MontLexer lexer(true);
    ifstream fileReader("lextest.txt");
    lexer.setStream(&fileReader);
    MontParser parser;
    bool f = parser.parse(lexer);
    fileReader.close();
    if (!f) {
        cout << "Lexer message: " << endl;
        cout << lexer.getErrorInfo();
        cout << "Parser message: " << endl;
        cout << parser.getErrorInfo();
        while (lexer.peek().tokenKind!=TK_EOF) {
            cout<<lexer.nextToken();
        }
        return 0;
    }
    MontConceiver conceiver; 
    f = conceiver.conceive(parser);
    if (f) {
        cout << "Intermediate representation:" << endl;
        cout << conceiver; 
    } else {
        cout << "Conceiver message: " << endl;
        cout << conceiver.getErrorInfo();
        return 0;
    }
    MontAssembler assembler;
    ofstream fileWriter("output.txt");
    assembler.setStream(&fileWriter);
    assembler.assemble(conceiver);
    fileWriter.close();
    cout << "Assembled." << endl;
    return 0;
}

int main(){
    compile();
    return 0;
}