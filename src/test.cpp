#include "montLexer.h"
#include "montParser.h"
#include <iostream>
#include <list>
using namespace std;

int mainLexer(){
    MontLexer lexer(true);
    lexer.openStream("lextest.txt");
    //cout << "opened Stream" << endl;
    while (true) {
        Token token = lexer.nextToken();
        cout << token;
        if (token.tokenKind == TK_ERROR) {
            cout << lexer.getErrorInfo() << endl; break;
        }
        if (token.tokenKind == TK_LBRACE || token.tokenKind == TK_RBRACE || token.tokenKind == TK_SEMICOLON) {
            cout << endl;
        }
        if (token.tokenKind == TK_EOF) break;
    }
    return 0;
}

int mainParser(){
    MontLexer lexer(true);
    lexer.openStream("lextest.txt");
    MontParser parser;
    bool f = parser.parse(lexer);
    if (f) {
        cout << parser;
    } else {
        cout << parser.getErrorMessage();
        while (lexer.peek().tokenKind!=TK_EOF) {
            cout<<lexer.nextToken();
        }
    }
    return 0;
}

int main(){
    mainParser();
    return 0;
}