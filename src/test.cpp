#include "montLexer.h"
#include <iostream>
using namespace std;

int main(){
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