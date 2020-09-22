#include <iostream>

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
    if (children[path[0]] == nullptr) 
        children[path[0]] = new LexAutomatonNode(); 
    return children[path[0]]->getNode(path+1);
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