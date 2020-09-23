#include <iostream>
#include <fstream>
#include <string.h>
#include "montLexer.h"

typedef LexAutomatonNode LNode;
typedef LNode* LNodePtr;

using std::cout;
using std::endl;

std::ostream& operator <<(std::ostream& stream, const Token& t){
    stream << "[";
    switch (t.tokenKind) {
        case TK_IDENTIFIER: stream << "Identifier: " << t.identifier; break;
        case TK_INT_VALUE: stream << "Integer Value: " << t.value; break;
        case TK_CHAR_VALUE: 
            stream << "Char Value: ";
            if (t.value >= 0x21 && t.value <= 0x7e) 
                stream << "\'" << (char)(t.value)  << "\'";
            else 
                stream << t.value;
            break;
        case TK_RETURN: stream << "Return"; break;
        case TK_INT: stream << "Integer"; break;
        case TK_CHAR: stream << "Char"; break;
        case TK_LPAREN: stream << "LParen ("; break;
        case TK_RPAREN: stream << "RParen )"; break;
        case TK_LBRACE: stream << "LBrace {"; break;
        case TK_RBRACE: stream << "RBrace }"; break;
        case TK_SEMICOLON: stream << "Semicolon ;"; break;
        case TK_COLON: stream << "Colon ,"; break;
        case TK_GREATER: stream << "Greater >"; break; 
        case TK_LESS: stream << "Less <"; break;
        case TK_GREATER_EQUAL: stream << "Greater or equal <="; break;
        case TK_LESS_EQUAL: stream << "Less or equal >="; break;
        case TK_RIGHT_SHIFT: stream << "Right shift >>"; break;
        case TK_LEFT_SHIFT: stream << "Left shift <<"; break;
        case TK_EQUAL: stream << "Equal =="; break;
        case TK_ASSIGN: stream << "Assign ="; break;
        case TK_EOF: stream << "EOF"; break;
        case TK_UNDEFINED: stream << "Undefined"; break;
        case TK_ERROR: stream << "Error"; break;
        default: stream << "Unknown token type"; break;
    }
    stream << "]";
    return stream;
}

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
    return children[c-VALID_CHAR_OFFSET];
}

MontLexer::MontLexer(bool addDefaultKeyword) {
    root = new LNode();
    currentPtr = root;
    if (addDefaultKeyword) addDefaultKeywords();
}

bool MontLexer::isAlphabet(char c) { 
    return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c=='_');
}

bool MontLexer::isLowercase(char c) { 
    return (c>='a' && c<='z');
}

bool MontLexer::isNumber(char c) {
    return (c>='0' && c<='9');
}

bool MontLexer::isSymbol(char c){
    int k = strlen(VALID_SYMBOLS);
    for (int i=0;i<k;i++) if (c==VALID_SYMBOLS[i]) return true;
    return false;
}

bool MontLexer::isSpace(char c){
    return (c=='\n') || (c=='\r') || (c=='\t') || (c==' ') || (c==EOF);
}

void MontLexer::openStream(const char* filename) {
    stream.open(filename);
}

void MontLexer::closeStream(){
    stream.close();
}

void MontLexer::reset(){
    currentToken = Token();
    flagIdentifier = true;
    flagSymbol = true;
    flagKeyword = true;
    flagValue = true;
    currentPtr = root;
    currentIdentifier = "";
    currentValue = 0;
    isValueChar = false;
    isValueCharEscape = false;
    isValueHex = false;
    currentLength = 0;
    errorInfo = "";
}

// If function transfer are run the first time after reset(),
// the stream must have skipped all the spaces.
MontLexer::TransferResult MontLexer::transfer(char c, char peek){
    //cout << endl << "c=" << c << " p=" << peek << " len=" << currentLength << " iskv" << 
    //    flagIdentifier << flagSymbol << flagKeyword << flagValue << endl;
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
            case ',':
                currentToken = Token(TK_COLON); return TR_FINISHED; break; 
            case '=':
                if (peek=='=') {currentToken = Token(TK_EQUAL); return TR_PEEKUSED_FINISHED;} 
                else {currentToken = Token(TK_ASSIGN); return TR_FINISHED;}
                break;
            case '>':
                if (peek=='=') {currentToken = Token(TK_GREATER_EQUAL); return TR_PEEKUSED_FINISHED;}
                else if (peek='>') {currentToken = Token(TK_RIGHT_SHIFT); return TR_PEEKUSED_FINISHED;}
                else {currentToken = Token(TK_GREATER); return TR_FINISHED;}
                break;
            case '<':
                if (peek=='=') {currentToken = Token(TK_LESS_EQUAL); return TR_PEEKUSED_FINISHED;}
                else if (peek='>') {currentToken = Token(TK_LEFT_SHIFT); return TR_PEEKUSED_FINISHED;}
                else {currentToken = Token(TK_LESS); return TR_FINISHED;}
                break;
            default:
                flagSymbol = false;
        }
    }
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
    if (flagIdentifier) {
        if (isAlphabet(c) || (currentLength>0 && isNumber(c))) {
            currentIdentifier += c;
        } else if (currentLength>0) {
            currentToken = Token(currentIdentifier);
            return TR_PUTBACK;
        } else {
            flagIdentifier = false;
        }
    }
    if (flagValue) {
        if (currentLength == 0) {
            if (!isNumber(c) && c!='\'') {flagValue = false;}
            else if (c=='0' && peek == 'x') {
                isValueHex = true; return TR_PEEKUSED_CONTINUE;
            } else if (c=='\'' && peek=='\\') {
                isValueCharEscape = isValueChar = true;
                return TR_PEEKUSED_CONTINUE;
            } else if (c=='\'') {
                isValueChar = true;
            } else {
                currentValue = c-48;
            }
        } else {
            if (!isValueChar) { // 1234, 0x1234
                if (!isValueHex) { 
                    if (isSpace(c) || isSymbol(c)) {currentToken = Token(TK_INT_VALUE, currentValue); return TR_PUTBACK;}
                    else if (c=='f') {errorInfo = "Value Error: Float value unsupported."; return TR_ERROR;}
                    else if (c=='.') {errorInfo = "Value Error: Float value unsupported."; return TR_ERROR;}
                    else if (!isNumber(c)) {errorInfo = "Value Error: Illegal decimal integer value."; return TR_ERROR;}
                    currentValue = currentValue * 10 + c - 48; 
                } else { 
                    if (isSpace(c) || isSymbol(c)) {currentToken = Token(TK_INT_VALUE, currentValue); return TR_PUTBACK;}
                    else if (!isNumber(c) && !(c>='A' && c<='F') && (!c>='a'  && c<='f')) {errorInfo = "Value: Illegal hexadecimal value."; return TR_ERROR;}
                    currentValue = currentValue * 16 + ((c<='9') ? (c-48) : ((c<='F') ? (c-'A'+10) : (c-'a'+10))); 
                }
            } else { // 'c', '\n', '\123', '\x123456'
                if (currentLength == 1) {
                    if (peek == '\'') {currentToken = Token(TK_CHAR_VALUE, c); return TR_PEEKUSED_FINISHED;}
                    else {errorInfo = "Value Error: Illegal char value, too long."; return TR_ERROR;}
                } else if (currentLength == 2 && isValueCharEscape) { 
                    switch (c) {
                        case 'n' : currentValue = '\n'; break;
                        case 'r' : currentValue = '\r'; break;
                        case 't' : currentValue = '\t'; break;
                        case 'v' : currentValue = '\v'; break;
                        case 'x' : isValueHex = true; return TR_CONTINUE; break; 
                        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
                            currentValue = c - 48; break;
                        default:
                            errorInfo = "Value Error: Illegan escape char."; return TR_ERROR; break;
                    }
                } else if (currentLength == 2 && !isValueCharEscape) {
                    if (c!='\'') {errorInfo = "Value Error: Illegal char value, too long."; return TR_ERROR;}
                    else {currentToken = Token(TK_CHAR_VALUE, currentValue); return TR_FINISHED;}
                } else if (currentLength > 2) {
                    if (!isValueCharEscape) {errorInfo = "Value Error: Illegal char value, too long."; return TR_ERROR;}
                    else if (c=='\'') {
                        if (currentLength == 3) {currentToken = Token(TK_CHAR_VALUE, currentValue); return TR_FINISHED;}
                        else if (isValueHex) {currentToken = Token(TK_CHAR_VALUE, currentValue); return TR_FINISHED;}
                        else if (currentLength != 5) {errorInfo = "Value Error: Illegal octal char value."; return TR_ERROR;}
                        else {currentToken = Token(TK_CHAR_VALUE, currentValue); return TR_FINISHED;}
                    } else if (isValueHex) {
                        if (!isNumber(c) && !(c>='A' && c<='F') && (!c>='a'  && c<='f')) {errorInfo = "Value: Illegal hexadecimal char value."; return TR_ERROR;}
                        currentValue = currentValue * 16 + ((c<='9') ? (c-48) : ((c<='F') ? (c-'A'+10) : (c-'a'+10))); 
                    } else {
                        if (c<'0' || c>'7') {errorInfo = "Value: Illegal octal char value."; return TR_ERROR;}
                        currentValue = currentValue * 8 + c - 48;
                    }
                }
            }
        }
    }
    return TR_CONTINUE;
}

Token MontLexer::nextToken(){
    reset();
    char c;
    while (true) { // get rid of spaces
        if (stream.eof()) return Token(TK_EOF);
        c = stream.get();
        if (!isSpace(c)) break;
    }
    char peek = stream.peek();
    while (true) {
        TransferResult result = transfer(c, peek);
        switch (result) {
            case TR_FINISHED: 
                currentLength++; 
                return currentToken; break;
            case TR_PEEKUSED_FINISHED: 
                currentLength+=2;
                stream.get(); return currentToken; break;
            case TR_PEEKUSED_CONTINUE:
                currentLength+=2; 
                stream.get(); break;
            case TR_PUTBACK: 
                stream.putback(c); return currentToken; break;
            case TR_CONTINUE:
                currentLength++; break;
        }
        c = stream.get(); peek = stream.peek();
    }
}

void MontLexer::addKeyword(const char* keyword, TokenKind tk) {
    root->createNode(keyword, tk);
}

void MontLexer::addDefaultKeywords(){
    // please add keywords by alphabetical order
    addKeyword("char", TK_CHAR);
    addKeyword("int", TK_INT);
    addKeyword("return", TK_RETURN);
}