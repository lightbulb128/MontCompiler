#include <iostream>
#include <fstream>
#include <string.h>
#include "montLexer.h"

typedef LexAutomatonNode LNode;
typedef LNode* LNodePtr;

using std::cout;
using std::endl;
using std::istream;

const bool SHOW_ROW_LINE = true;

MontLog MontLexer::logger = MontLog();

std::ostream& operator <<(std::ostream& stream, const Token& t){
    stream << "[";
    if (SHOW_ROW_LINE) stream<<t.row<<":"<<t.column<<" ";
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
        case TK_COMMA: stream << "Comma ,"; break;
        case TK_GREATER: stream << "Greater >"; break; 
        case TK_LESS: stream << "Less <"; break;
        case TK_GREATER_EQUAL: stream << "Greater or equal >="; break;
        case TK_LESS_EQUAL: stream << "Less or equal <="; break;
        case TK_RIGHT_SHIFT: stream << "Right shift >>"; break;
        case TK_LEFT_SHIFT: stream << "Left shift <<"; break;
        case TK_EQUAL: stream << "Equal =="; break;
        case TK_ASSIGN: stream << "Assign ="; break;
        case TK_EOF: stream << "EOF"; break;
        case TK_UNDEFINED: stream << "Undefined"; break;
        case TK_ERROR: stream << "Error"; break;
        case TK_EXCLAMATION: stream << "Exclamation !"; break;
        case TK_TILDE: stream << "Tilde ~"; break;
        case TK_NOT_EQUAL: stream << "Not equal !="; break;
        case TK_MINUS: stream << "Minus -"; break;
        case TK_PLUS: stream << "Plus +"; break;
        case TK_ASTERISK: stream << "Asterisk *"; break;
        case TK_LSLASH: stream << "LSlash /"; break;
        case TK_PERCENT: stream << "Percentage %"; break;
        case TK_AND: stream << "And &"; break;
        case TK_OR: stream << "Or |"; break;
        case TK_LAND: stream << "Logical and &&"; break;
        case TK_LOR: stream << "Logical or ||"; break;
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

MontLexer::~MontLexer(){
    delete root;
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

void MontLexer::setStream(istream* input) {
    stream = input;
    currentRow = 1; currentColumn = 0; lastChar = ' ';
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
}

// If function transfer are run the first time after reset(),
// the stream must have skipped all the spaces.
MontLexer::TransferResult MontLexer::transfer(char c, char peek){
    if (!flagSymbol && !flagKeyword && !flagIdentifier && !flagValue) {
        appendErrorInfo("Token: Unknown token."); return TR_ERROR;
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
            case ',':
                currentToken = Token(TK_COMMA); return TR_FINISHED; break; 
            case '=':
                if (peek=='=') {currentToken = Token(TK_EQUAL); return TR_PEEKUSED_FINISHED;} 
                else {currentToken = Token(TK_ASSIGN); return TR_FINISHED;}
                break;
            case '>':
                if (peek=='=') {currentToken = Token(TK_GREATER_EQUAL); return TR_PEEKUSED_FINISHED;}
                else if (peek=='>') {currentToken = Token(TK_RIGHT_SHIFT); return TR_PEEKUSED_FINISHED;}
                else {currentToken = Token(TK_GREATER); return TR_FINISHED;}
                break;
            case '<':
                if (peek=='=') {currentToken = Token(TK_LESS_EQUAL); return TR_PEEKUSED_FINISHED;}
                else if (peek=='>') {currentToken = Token(TK_LEFT_SHIFT); return TR_PEEKUSED_FINISHED;}
                else {currentToken = Token(TK_LESS); return TR_FINISHED;}
                break;
            case '-':
                currentToken = Token(TK_MINUS); return TR_FINISHED; break;
            case '!':
                if (peek=='=') {currentToken = Token(TK_NOT_EQUAL); return TR_PEEKUSED_FINISHED;}
                else {currentToken = Token(TK_EXCLAMATION); return TR_FINISHED;}
                break;
            case '~':
                currentToken = Token(TK_TILDE); return TR_FINISHED; break;
            case '+':
                currentToken = Token(TK_PLUS); return TR_FINISHED; break;
            case '*':
                currentToken = Token(TK_ASTERISK); return TR_FINISHED; break;
            case '/':
                currentToken = Token(TK_LSLASH); return TR_FINISHED; break;
            case '%':
                currentToken = Token(TK_PERCENT); return TR_FINISHED; break;
            case '|':
                if (peek=='|') {currentToken = Token(TK_LOR); return TR_PEEKUSED_FINISHED;}
                else {currentToken = Token(TK_OR); return TR_FINISHED;}
                break;
            case '&':
                if (peek=='&') {currentToken = Token(TK_LAND); return TR_PEEKUSED_FINISHED;}
                else {currentToken = Token(TK_AND); return TR_FINISHED;}
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
                    else if (c=='f') {appendErrorInfo("Value Error: Float value unsupported."); return TR_ERROR;}
                    else if (c=='.') {appendErrorInfo("Value Error: Float value unsupported."); return TR_ERROR;}
                    else if (!isNumber(c)) {appendErrorInfo("Value Error: Illegal decimal integer value."); return TR_ERROR;}
                    long long intermediate = (long long)currentValue * 10 + c - 48; 
                    if (intermediate > MAX_INT) {
                        appendErrorInfo("Value Error: Too large an integer.");
                        return TR_ERROR;
                    } else currentValue = intermediate;
                } else { 
                    if (isSpace(c) || isSymbol(c)) {currentToken = Token(TK_INT_VALUE, currentValue); return TR_PUTBACK;}
                    else if (!isNumber(c) && !(c>='A' && c<='F') && (!c>='a'  && c<='f')) {appendErrorInfo("Value: Illegal hexadecimal value."); return TR_ERROR;}
                    long long intermediate = (long long)currentValue * 16 + ((c<='9') ? (c-48) : ((c<='F') ? (c-'A'+10) : (c-'a'+10))); 
                    if (intermediate > MAX_INT) {
                        appendErrorInfo("Value Error: Too large an integer.");
                        return TR_ERROR;
                    } else currentValue = intermediate;
                }
            } else { // 'c', '\n', '\123', '\x123456'
                if (currentLength == 1) {
                    if (peek == '\'') {currentToken = Token(TK_CHAR_VALUE, c); return TR_PEEKUSED_FINISHED;}
                    else {appendErrorInfo("Value Error: Illegal char value, too long."); return TR_ERROR;}
                } else if (currentLength == 2 && isValueCharEscape) { 
                    switch (c) {
                        case 'a' : currentValue = '\a'; break;
                        case 'b' : currentValue = '\b'; break;
                        case 'f' : currentValue = '\f'; break;
                        case 'n' : currentValue = '\n'; break;
                        case 'r' : currentValue = '\r'; break;
                        case 't' : currentValue = '\t'; break;
                        case 'v' : currentValue = '\v'; break;
                        case '\\' : currentValue = '\\'; break;
                        case '\'' : currentValue = '\''; break;
                        case '\"' : currentValue = '\"'; break;
                        case '\?' : currentValue = '\?'; break;
                        case 'x' : isValueHex = true; return TR_CONTINUE; break; 
                        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
                            currentValue = c - 48; break;
                        default:
                            appendErrorInfo("Value Error: Illegan escape char."); return TR_ERROR; break;
                    }
                } else if (currentLength == 2 && !isValueCharEscape) {
                    if (c!='\'') {appendErrorInfo("Value Error: Illegal char value, too long."); return TR_ERROR;}
                    else {currentToken = Token(TK_CHAR_VALUE, currentValue); return TR_FINISHED;}
                } else if (currentLength > 2) {
                    if (!isValueCharEscape) {appendErrorInfo("Value Error: Illegal char value, too long."); return TR_ERROR;}
                    else if (c=='\'') {
                        if (currentLength == 3) {currentToken = Token(TK_CHAR_VALUE, currentValue); return TR_FINISHED;}
                        else if (isValueHex) {currentToken = Token(TK_CHAR_VALUE, currentValue); return TR_FINISHED;}
                        else if (currentLength != 5) {appendErrorInfo("Value Error: Illegal octal char value."); return TR_ERROR;}
                        else {currentToken = Token(TK_CHAR_VALUE, currentValue); return TR_FINISHED;}
                    } else if (isValueHex) {
                        if (!isNumber(c) && !(c>='A' && c<='F') && (!c>='a'  && c<='f')) {appendErrorInfo("Value: Illegal hexadecimal char value."); return TR_ERROR;}
                        currentValue = currentValue * 16 + ((c<='9') ? (c-48) : ((c<='F') ? (c-'A'+10) : (c-'a'+10))); 
                    } else {
                        if (c<'0' || c>'7') {appendErrorInfo("Value: Illegal octal char value."); return TR_ERROR;}
                        currentValue = currentValue * 8 + c - 48;
                    }
                }
            }
        }
    }
    return TR_CONTINUE;
}

void MontLexer::killSpaces(){ 
    char c;
    while (true) { // get rid of spaces
        if (stream->eof()) return;
        c = getChar();
        if (c=='/' && stream->peek()=='/') {
            while (c!='\n') c = getChar();
        } else if (c=='/' && stream->peek()=='*') {
            while (c!='*' || stream->peek()!='/') c=getChar();
            c=getChar(); c=getChar();
        }
        if (!isSpace(c)) break;
    }
    putbackChar(c);
}

Token MontLexer::nextToken(){
    if (!buffer.empty()) {Token ret = buffer.top(); buffer.pop(); return ret;}
    reset(); killSpaces();
    char c = getChar();
    if (c==EOF){
        Token ret = Token(TK_EOF);
        ret.setRC(currentRow+1, currentColumn);
        return ret;
    }
    int cr = currentRow, cc = currentColumn;
    char peek = stream->peek();
    while (true) {
        TransferResult result = transfer(c, peek);
        switch (result) {
            case TR_FINISHED: 
                currentLength++; currentToken.setRC(cr,cc);
                return currentToken; break;
            case TR_PEEKUSED_FINISHED: 
                currentLength+=2; getChar(); currentToken.setRC(cr,cc); 
                return currentToken; break;
            case TR_PEEKUSED_CONTINUE:
                currentLength+=2; 
                getChar(); break;
            case TR_PUTBACK: 
                putbackChar(c); currentToken.setRC(cr,cc);
                return currentToken; break;
            case TR_CONTINUE:
                currentLength++; break;
            case TR_ERROR:
                Token ret = Token(TK_ERROR); ret.setRC(cr, cc);
                return ret; break;
        }
        c = getChar(); peek = stream->peek();
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

void MontLexer::putback(Token token) {
    buffer.push(token);
}

Token MontLexer::peek(){
    Token next = nextToken();
    putback(next);
    return next;
}