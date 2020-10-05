#ifndef MONTLEXER_H
#define MONTLEXER_H

#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include "montLog.h"

using std::string;
using std::stack;

const int   VALID_CHAR_RANGE  = 26;
const int   VALID_CHAR_OFFSET = 97;
// const char* VALID_SYMBOLS    = "~!%^&*()-=+[]{}\\|;:<>,./?";
const char * const VALID_SYMBOLS = "(){};><=,-~!+*%/|&"; // quote should not be treated as a symbol
const int MAX_INT = 0x7fffffff;

enum TokenKind {
    // identifier
    TK_IDENTIFIER,
    // values
    TK_INT_VALUE, TK_CHAR_VALUE,
    // keywords
    TK_RETURN, TK_INT, TK_CHAR,
    // symbols
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE, TK_SEMICOLON, 
    TK_COMMA, 
    TK_GREATER, TK_LESS, TK_GREATER_EQUAL, TK_LESS_EQUAL, 
    TK_RIGHT_SHIFT, TK_LEFT_SHIFT, 
    TK_EQUAL,
    TK_MINUS, TK_TILDE, TK_EXCLAMATION, TK_NOT_EQUAL, 
    TK_ASSIGN,
    TK_PLUS, TK_ASTERISK, TK_LSLASH, TK_PERCENT, 
    TK_LOR, TK_LAND, TK_OR, TK_AND, 
    // eof
    TK_EOF,
    // error
    TK_UNDEFINED, TK_ERROR
};

struct Token{
    TokenKind tokenKind;
    int value;
    string identifier;
    int row, column;
    Token(TokenKind tokenKind, int value=0):tokenKind(tokenKind), value(value){identifier="";}
    Token(){tokenKind=TK_UNDEFINED; value=0; identifier="";}
    Token(string identifier):identifier(identifier), tokenKind(TK_IDENTIFIER), value(0){}
    void setRC(int r, int c){row=r;column=c;}
    friend std::ostream& operator <<(std::ostream& stream, const Token& t);
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
class MontLexer {
private:
    static MontLog logger;
    LexAutomatonNode* root;
    LexAutomatonNode* currentPtr;
    Token currentToken;
    bool flagIdentifier;
    bool flagSymbol;
    bool flagKeyword;
    bool flagValue;
    int currentValue;
    bool isValueHex;
    bool isValueChar;
    bool isValueCharEscape;
    string currentIdentifier;
    int currentLength;
    std::istream* stream;
    //string errorInfo;
    stack<Token> buffer;
    int currentRow, currentColumn;
    char lastChar;
    enum TransferResult{
        TR_CONTINUE,
        TR_FINISHED,
        TR_PUTBACK,
        TR_ERROR,
        TR_PEEKUSED_CONTINUE,
        TR_PEEKUSED_FINISHED
    };
    TransferResult transfer(char c, char peek);
    void reset();
    void appendErrorInfo(string str) {
        logger.log("(" + std::to_string(currentRow) + ":" + 
            std::to_string(currentColumn) + ") " 
            + str); 
    }
public:
    MontLexer(bool addDefaultKeyword);
    ~MontLexer();
    static bool isAlphabet(char c); // underline is considered as an alphabet
    static bool isNumber(char c);
    static bool isSymbol(char c);
    static bool isLowercase(char c);
    static bool isSpace(char c);
    void setStream(std::istream* input);
    void addKeyword(const char* keyword, TokenKind tk);
    void addDefaultKeywords();
    Token nextToken();
    string getErrorInfo(){return logger.get();}
    void putback(Token token);
    void killSpaces();
    Token peek();
    char getChar(){
        if (lastChar == '\n' || lastChar == EOF) {currentRow++; currentColumn=1;}
        else {currentColumn++;}
        // std::cout<<"getchar: "<<currentRow<<":"<<currentColumn<<std::endl;
        lastChar = stream->get();
        return lastChar;
    }
    void putbackChar(char c){
        currentColumn--;
        lastChar = ' ';
        stream->putback(c);
    }
    int getCurrentRow(){
        if (!buffer.empty()) return buffer.top().row;
        return currentRow;
    }
    int getCurrentColumn(){
        if (!buffer.empty()) return buffer.top().column;
        return currentColumn;
    }
};

#endif