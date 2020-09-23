#include <iostream>
#include <fstream>
#include <string.h>
#include "montLexer.h"

typedef LexAutomatonNode LNode;
typedef LNode* LNodePtr;

Token::Token(TokenKind tokenKind, int value) : 
    tokenKind(tokenKind), value(value) {}

LNode::LexAutomatonNode() {
    for (int i=0;i<VALID_CHAR_RANGE;i++) children[i]=nullptr; 
    tokenKind = TK_UNDEFINED;
}

LNodePtr LNode::getNode(const char* path) {
    if (path[0]==0) return this;
    int c = path[0] - VALID_CHAR_OFFSET;
    if (children[c] == nullptr) 
        children[c] = new LexAutomatonNode(); 
    return children[c]->getNode(path+1);
}

LNodePtr LNode::createNode(const char* path, const TokenKind token) {
    LNodePtr ptr = getNode(path);
    ptr->tokenKind = token;
    return ptr;
}

LNode::~LexAutomatonNode() {
    for (int i=0;i<VALID_CHAR_RANGE;i++) 
        if (children[i]!=nullptr) {
            delete children[i];
            children[i] = nullptr;
        }
}

LNode* LNode::transfer(char c){
    return children[c];
}

LexAutomaton::LexAutomaton() {
    root = new LNode();
    currentPtr = root;
}

bool LexAutomaton::isAlphabet(char c) { 
    return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c=='_');
}

bool LexAutomaton::isLowercase(char c) { 
    return (c>='a' && c<='z');
}

bool LexAutomaton::isNumber(char c) {
    return (c>='0' && c<='9');
}

bool LexAutomaton::isSymbol(char c){
    int k = strlen(VALID_SYMBOLS);
    for (int i=0;i<k;i++) if (c==VALID_SYMBOLS[i]) return true;
    return false;
}

void LexAutomaton::openStream(const char* filename) {
    stream.open(filename);
}

void LexAutomaton::reset(){
    flagIdentifier = true;
    flagSymbol = true;
    flagKeyword = true;
    currentPtr = root;
    currentLength = 0;
}

LexAutomaton::TransferResult LexAutomaton::transfer(char c, char peek){
    if (flagKeyword) {
        if (isSymbol(c) || isSpace(c)) {
            if (currentPtr->getTokenKind() != TK_UNDEFINED) {
                currentToken = Token(currentPtr->getTokenKind());
                return TR_PUTBACK;
            } else {
                flagKeyword = false;
            }
        } else if (isLowercase(c)) {
            currentPtr = currentPtr->transfer(c);
            if (currentPtr == nullptr) flagKeyword = false; 
        } else {
            flagKeyword = false;
        }
    } 
    if (flagSymbol) {
        switch (c) {
            case '(': 
                currentToken = Token(TK_LPAREN); return TR_FINISHED; break; 
            case ')':
                currentToken = Token(TK_RPAREN); return TR_FINISHED; break; 
            case '{':
                currentToken = Token(TK_LBRACE); return TR_FINISHED; break;
            case '}':
                currentToken = Token(TK_RBRACE); return TR_FINISHED; break;
            case ';':
                currentToken = Token(TK_SEMICOLON); return TR_FINISHED; break;  
            case '=':
                if (peek=='=') {
                    currentToken = Token(TK_EQUAL); return TR_PEEKUSED; break;
                }
        }
    }
}