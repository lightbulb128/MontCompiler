#include <iostream>
#include <fstream>

const int   VALID_CHAR_RANGE = 26;
const int   VALID_CHAR_OFFSET = 97;
// const char* VALID_SYMBOLS    = "~!%^&*()-=+[]{}\\|;:'\"<>,./?";
const char* VALID_SYMBOLS = "(){};><=";

enum TokenKind {
    // identifier
    TK_IDENTIFIER,
    // primitive types
    TK_INT, TK_CHAR,
    // values
    TK_INT_VALUE, TK_CHAR_VALUE,
    // keywords
    TK_RETURN,
    // symbols
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE, TK_SEMICOLON,
    TK_GREATER, TK_LESS, TK_GREATER_EQUAL, TK_LESS_EQUAL, 
    TK_EQUAL,
    TK_ASSIGN,
    // eof
    TK_EOF,
    // error
    TK_UNDEFINED
};

struct Token{
    TokenKind tokenKind;
    int value;
    Token(TokenKind tokenKind, int value = 0);
    Token(){tokenKind=TK_UNDEFINED; value=0;}
};

class LexAutomatonNode {
private:
    LexAutomatonNode* children[VALID_CHAR_RANGE];
    TokenKind tokenKind;
public:
    LexAutomatonNode();
    ~LexAutomatonNode();
    LexAutomatonNode* getNode(const char* path);
    LexAutomatonNode* createNode(const char* path, const TokenKind token);
    LexAutomatonNode* transfer(char c);
    TokenKind getTokenKind(){return tokenKind;}
};

/*
    Use a trie structure to detect keywords
    Constant value and identifier detection are done manually (brute-force)
*/
class LexAutomaton {
private:
    LexAutomatonNode* root;
    LexAutomatonNode* currentPtr;
    Token currentToken;
    bool flagIdentifier;
    bool flagSymbol;
    bool flagKeyword;
    int currentLength;
    std::ifstream stream;
    enum TransferResult{
        TR_CONTINUE,
        TR_FINISHED,
        TR_PUTBACK,
        TR_PEEKUSED
    };
    TransferResult transfer(char c, char peek);
    void reset();
public:
    LexAutomaton();
    static bool isAlphabet(char c); // underline is considered as an alphabet
    static bool isNumber(char c);
    static bool isSymbol(char c);
    static bool isLowercase(char c);
    static bool isSpace(char c){return c==' ';}
    void openStream(const char* filename);
    Token nextToken();
};