const int VALID_CHAR_RANGE = 128;

enum TokenKind {
    // identifier
    TK_IDENTIFIER,
    // primitive types
    TK_INT, TK_CHAR,
    // values
    TK_INT_VALUE, TK_CHAR_VALUE,
    // keywords
    TK_RETURN,
    // error
    TK_UNDEFINED
};

struct Token{
    TokenKind tokenKind;
    int value;
    Token(TokenKind tokenKind, int value = 0);
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
};

/*
    Use a trie structure to detect keywords
    Constant value and identifier detection are done manually (brute-force)
*/
class LexAutomaton {
private:
    LexAutomatonNode* root;
    LexAutomatonNode* currentPtr;
public:
    LexAutomaton();
};