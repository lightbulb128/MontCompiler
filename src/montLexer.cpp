#include <iostream>

const int VALID_CHAR_RANGE = 128;

enum TokenKind {
    // identifier
    TK_IDENTIFIER,
    // primitive types
    TK_INT, TK_CHAR,
    // values
    TK_INT_VALUE, TK_CHAR_VALUE,
    // keywords
    TK_RETURN
};

struct Token {
    TokenKind tokenKind;
    int value;
    Token(TokenKind tokenKind, int value) : tokenKind(tokenKind), value(value) {}
};

class LexAutomatonNode {
public:
    LexAutomatonNode* children[VALID_CHAR_RANGE];
    Token* token;
    LexAutomatonNode(){for (int i=0;i<VALID_CHAR_RANGE;i++) children[i]=nullptr; token=nullptr;}
    
};

/*
    Use a trie structure to detect keywords
    Constant value and identifier detection are done manually (brute-force)
*/
class LexAutomaton { 
private:
    LexAutomatonNode* root;
public:
};

int main(){
    
}