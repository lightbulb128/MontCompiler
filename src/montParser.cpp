#include "montParser.h"
#include <string>
#include <iostream>
#include <cmath>

#define PARSEFAIL(str) {ptr->putback(lexer); appendErrorInfo(str, ptr->row, ptr->column); delete ptr; return false;}

using std::string;
using std::ostream;
using std::endl;
using std::cout;

const bool DEBUG = false;
const bool SHOW_ROW_LINE = true;
bool MontParser::outputType = false;

MontLog MontParser::logger = MontLog();

typedef MontNodePtr Mnp; 

MontNode::~MontNode(){
    for (int i=0;i<children.size();i++) 
        if (children[i]!=nullptr) delete children[i];
    children.clear();
}

// 应当保证再trystart和tryend调用之间不存在任何中途return。
void MontNode::tryStart(){MontParser::tryStart();}
void MontNode::tryEnd(){MontParser::tryEnd();}

void MontNode::putback(MontLexer& lexer){
    int s = children.size();
    for (int i=s-1;i>=0;i--) {
        children[i]->putback(lexer);
        delete children[i];
    }
    children.clear();
}

bool MontNode::appendErrorInfo(string str, int row, int column){
    return MontParser::appendErrorInfo(str, row, column);
}

void MontTokenNode::putback(MontLexer& lexer) {
    lexer.putback(token);
}

void MontNode::addChildren(Mnp p){
    // if (children.size()==0) {row=p->row; column=p->column;}
    memorySize += p->memorySize;
    children.push_back(p);
}

bool MontNode::tryParse(MontLexer& lexer, TokenKind tk) {
    if (DEBUG) cout << "try parse " << lexer.peek() << endl;
    Token load = lexer.nextToken();
    if (load.tokenKind == tk) {
        if (DEBUG) cout << "ok parsed token " << load << endl;
        addChildren(new MontTokenNode(load)); return true;
    } else {
        lexer.putback(load); return false;
    }
}

bool MontNode::isValueToken(Token& token){
    TokenKind tk = token.tokenKind;
    return (tk==TK_CHAR_VALUE || tk==TK_INT_VALUE || tk==TK_TRUE || tk==TK_FALSE);
}

bool MontNode::tryParseValue(MontLexer& lexer) {
    if (DEBUG) cout << "try parse value " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_VALUE;
    Token token = lexer.peek();
    if (isValueToken(token)) ptr->tryParse(lexer, token.tokenKind);
    else PARSEFAIL("Value: Expect value token.");
    if (DEBUG) cout << "ok parsed value" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseEmpty(MontLexer& lexer){
    if (DEBUG) cout << "try parse empty " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_EMPTY;
    if (DEBUG) cout << "ok parsed empty" << endl;
    addChildren(ptr); return true;
}

bool MontNode::isUnaryOperatorToken(Token& t){
    return (t.tokenKind == TK_EXCLAMATION ||
        t.tokenKind == TK_TILDE ||
        t.tokenKind == TK_MINUS);
}

bool MontNode::isAdditiveOperatorToken(Token& t){
    return (t.tokenKind == TK_MINUS) || (t.tokenKind == TK_PLUS);
}

bool MontNode::isMultiplicativeOperatorToken(Token& t){
    return (t.tokenKind == TK_ASTERISK) || (t.tokenKind == TK_LSLASH) 
        || (t.tokenKind == TK_PERCENT);
}

bool MontNode::isEqualityOperatorToken(Token& t){
    return (t.tokenKind == TK_EQUAL || t.tokenKind == TK_NOT_EQUAL);
}

bool MontNode::isRelationalOperatorToken(Token& t) {
    return (t.tokenKind == TK_LESS || t.tokenKind == TK_LESS_EQUAL 
        || t.tokenKind == TK_GREATER || t.tokenKind == TK_GREATER_EQUAL);
}

bool MontNode::tryParsePrimary(MontLexer& lexer){
    if (DEBUG) cout << "try parse primary " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_PRIMARY;
    Token token = lexer.peek();
    if (token.tokenKind == TK_LPAREN) {
        ptr->expansion = NE_PRIMARY_PAREN;
        if (!ptr->tryParse(lexer, TK_LPAREN) || !ptr->tryParseExpression(lexer) 
            || !ptr->tryParse(lexer, TK_RPAREN)) 
            PARSEFAIL("Primary: Expect LParen expression RParen.");
    } else if (token.tokenKind == TK_IDENTIFIER) { 
        ptr->expansion = NE_PRIMARY_IDENTIFIER;
        if (!ptr->tryParse(lexer, TK_IDENTIFIER))
            PARSEFAIL("Primary: Expect identifier.");
    } else {
        ptr->expansion = NE_PRIMARY_VALUE;
        if (!ptr->tryParseValue(lexer)) 
            PARSEFAIL("Primary: Expect value syntax.");
    }
    if (DEBUG) cout << "ok parsed primary" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseUnary(MontLexer& lexer) {
    if (DEBUG) cout << "try parse unary " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_UNARY;
    Token token = lexer.peek();
    if (!isUnaryOperatorToken(token)) {
        ptr->expansion = NE_UNARY_POSTFIX;
        if (!ptr->tryParsePostfix(lexer)) 
            PARSEFAIL("Unary: Expect postfix syntax.");
    } else {
        ptr->expansion = NE_UNARY_OPERATION;
        if (!ptr->tryParse(lexer, token.tokenKind) || !ptr->tryParseUnary(lexer)) 
            PARSEFAIL("Unary: Expect unary.");
    }
    if (DEBUG) cout << "ok parsed unary" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseRelational(MontLexer& lexer) {
    if (DEBUG) cout << "try parse relational " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_RELATIONAL;
    ptr->expansion = NE_RELATIONAL_LEAF;
    if (!ptr->tryParseAdditive(lexer)) 
        PARSEFAIL("Relational: Expect additive syntax.");
    Token token = lexer.peek();
    while (isRelationalOperatorToken(token)) {
        Mnp newptr = new MontNode(); newptr->kind = NK_RELATIONAL;
        newptr->expansion = NE_RELATIONAL_INNER;
        newptr->copyRC(*ptr);
        newptr->addChildren(ptr); ptr = newptr;
        ptr->tryParse(lexer, token.tokenKind);
        if (!ptr->tryParseAdditive(lexer)) 
            PARSEFAIL("Relational: Expect additive syntax after operator.");
        token = lexer.peek();
    }
    if (DEBUG) cout << "ok parsed relational" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseEquality(MontLexer& lexer) {
    if (DEBUG) cout << "try parse equality " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_EQUALITY;
    ptr->expansion = NE_EQUALITY_LEAF;
    if (!ptr->tryParseRelational(lexer)) 
        PARSEFAIL("Equality: Expect relational syntax.");
    Token token = lexer.peek();
    while (isEqualityOperatorToken(token)) {
        Mnp newptr = new MontNode(); newptr->kind = NK_EQUALITY;
        newptr->expansion = NE_EQUALITY_INNER;
        newptr->copyRC(*ptr);
        newptr->addChildren(ptr); ptr = newptr;
        ptr->tryParse(lexer, token.tokenKind);
        if (!ptr->tryParseRelational(lexer)) 
            PARSEFAIL("Equality: Expect relational syntax after operator.");
        token = lexer.peek();
    }
    if (DEBUG) cout << "ok parsed equality" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseLogicalAnd(MontLexer& lexer) {
    if (DEBUG) cout << "try parse logical_and " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_LOGICAL_AND;
    ptr->expansion = NE_LAND_LEAF;
    if (!ptr->tryParseEquality(lexer)) 
        PARSEFAIL("Logical and: Expect equality syntax.");
    Token token = lexer.peek();
    while (token.tokenKind == TK_LAND) {
        Mnp newptr = new MontNode(); newptr->kind = NK_LOGICAL_AND;
        newptr->expansion = NE_LAND_INNER;
        newptr->copyRC(*ptr);
        newptr->addChildren(ptr); ptr = newptr;
        ptr->tryParse(lexer, token.tokenKind);
        if (!ptr->tryParseEquality(lexer)) 
            PARSEFAIL("Logical and: Expect equality syntax after operator.");
        token = lexer.peek();
    }
    if (DEBUG) cout << "ok parsed logical_and" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseLogicalOr(MontLexer& lexer) {
    if (DEBUG) cout << "try parse logical_or " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_LOGICAL_OR;
    ptr->expansion = NE_LOR_LEAF;
    if (!ptr->tryParseLogicalAnd(lexer)) 
        PARSEFAIL("Logical or: Expect logical_and syntax.");
    Token token = lexer.peek();
    while (token.tokenKind == TK_LOR) {
        Mnp newptr = new MontNode(); newptr->kind = NK_LOGICAL_OR;
        newptr->expansion = NE_LOR_INNER;
        newptr->copyRC(*ptr);
        newptr->addChildren(ptr); ptr = newptr;
        ptr->tryParse(lexer, token.tokenKind);
        if (!ptr->tryParseLogicalAnd(lexer)) 
            PARSEFAIL("Logical or: Expect logical_and syntax after operator.");
        token = lexer.peek();
    }
    if (DEBUG) cout << "ok parsed logical_or" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseMultiplicative(MontLexer& lexer) {
    if (DEBUG) cout << "try parse multiplicative " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_MULTIPLICATIVE;
    ptr->expansion = NE_MULTIPLICATIVE_LEAF;
    if (!ptr->tryParseUnary(lexer)) 
        PARSEFAIL("Multiplicative: Expect unary syntax.");
    Token token = lexer.peek();
    while (isMultiplicativeOperatorToken(token)) {
        Mnp newptr = new MontNode(); newptr->kind = NK_MULTIPLICATIVE;
        newptr->expansion = NE_MULTIPLICATIVE_INNER;
        newptr->copyRC(*ptr);
        newptr->addChildren(ptr); ptr = newptr;
        ptr->tryParse(lexer, token.tokenKind);
        if (!ptr->tryParseUnary(lexer)) 
            PARSEFAIL("Multiplicative: Expect unary syntax after operator.");
        token = lexer.peek();
    }
    if (DEBUG) cout << "ok parsed multiplicative" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseAdditive(MontLexer& lexer) {
    if (DEBUG) cout << "try parse additive " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_ADDITIVE;
    ptr->expansion = NE_ADDITIVE_LEAF;
    if (!ptr->tryParseMultiplicative(lexer)) 
        PARSEFAIL("Additive: Expect multiplicative syntax.");
    Token token = lexer.peek();
    while (isAdditiveOperatorToken(token)) {
        Mnp newptr = new MontNode(); newptr->kind = NK_ADDITIVE;
        newptr->expansion = NE_ADDITIVE_INNER;
        newptr->copyRC(*ptr);
        newptr->addChildren(ptr); ptr = newptr;
        ptr->tryParse(lexer, token.tokenKind);
        if (!ptr->tryParseMultiplicative(lexer)) 
            PARSEFAIL("Additive: Expect multiplicative syntax after operator.");
        token = lexer.peek();
    }
    if (DEBUG) cout << "ok parsed additive" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseExpression(MontLexer& lexer) {
    if (DEBUG) cout << "try parse expression " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_EXPRESSION;
    if (!ptr->tryParseAssignment(lexer)) PARSEFAIL("Expression: Not valid assignment.");
    if (DEBUG) cout << "ok parsed expression" << endl;
    addChildren(ptr); return true;
}

bool MontNode::isTypeToken(Token& token){
    TokenKind tk = token.tokenKind;
    return (tk==TK_CHAR || tk==TK_INT || tk==TK_BOOL);
}

bool MontNode::tryParseType(MontLexer& lexer) {
    if (DEBUG) cout << "try parse type " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_TYPE;
    Token token = lexer.peek();
    if (isTypeToken(token)) ptr->tryParse(lexer, token.tokenKind);
    else PARSEFAIL("Type: Expect type name.");
    if (DEBUG) cout << "ok parsed type" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseDeclaration(MontLexer& lexer) {
    if (DEBUG) cout << "try parse declaration " << lexer.peek() << endl;
    Token peek = lexer.peek();
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_DECLARATION; 
    ptr->memorySize = 4;
    if (!ptr->tryParseType(lexer) || !ptr->tryParse(lexer, TK_IDENTIFIER)) 
        PARSEFAIL("Declaration: Expect type and identifier.");
    peek = lexer.peek();
    if (peek.tokenKind == TK_ASSIGN) {
        ptr->expansion = NE_DECLARATION_INIT;
        if (!ptr->tryParse(lexer, TK_ASSIGN) || !ptr->tryParseExpression(lexer)) 
            PARSEFAIL("Declaration: Expect expression after assign mark.");
    } else 
        ptr->expansion = NE_DECLARATION_SIMPLE;
    if (DEBUG) cout << "ok parsed declaration" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseAssignment(MontLexer& lexer) {
    if (DEBUG) cout << "try parse assignment " << lexer.peek() << endl;
    Token peek = lexer.peek();
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_ASSIGNMENT;
    bool successful = false;
    tryStart();
    if (peek.tokenKind == TK_IDENTIFIER) { // Identifier Assign expression
        if (!ptr->tryParse(lexer, TK_IDENTIFIER) || !ptr->tryParse(lexer, TK_ASSIGN) || !ptr->tryParseExpression(lexer)) 
            ptr->putback(lexer);
        else successful = true, ptr->expansion = NE_ASSIGNMENT_ASSIGN;
    }
    if (!successful) { // conditional
        successful = ptr->tryParseConditional(lexer), ptr->expansion = NE_ASSIGNMENT_VALUE;
        if (!successful) ptr->putback(lexer);
    }
    tryEnd();
    if (!successful) 
        PARSEFAIL("Assignment: Invalid assignment. Expect conditional or assignment expression.");
    if (DEBUG) cout << "ok parsed assignment" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseConditional(MontLexer& lexer) {
    if (DEBUG) cout << "try parse conditional " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_CONDITIONAL; ptr->expansion = NE_CONDITIONAL_LEAF;
    if (!ptr->tryParseLogicalOr(lexer)) 
        PARSEFAIL("Conditional: Illegal logical_or syntax.");
    Token peek = lexer.peek();
    if (peek.tokenKind == TK_QUESTION) {
        ptr->expansion = NE_CONDITIONAL_INNER;
        if (!ptr->tryParse(lexer, TK_QUESTION) || !ptr->tryParseExpression(lexer)
            || !ptr->tryParse(lexer, TK_COLON) || !ptr->tryParseConditional(lexer)) 
            PARSEFAIL("Conditional: Illegal conditional expression.");
    }
    if (DEBUG) cout << "ok parsed conditional" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParsePostfix(MontLexer& lexer) {
    if (DEBUG) cout << "try parse postfix " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_POSTFIX; 
    Token p1 = lexer.nextToken(), p2 = lexer.nextToken();
    lexer.putback(p2); lexer.putback(p1);
    if (p1.tokenKind == TK_IDENTIFIER && p2.tokenKind == TK_LPAREN) {
        ptr->expansion = NE_POSTFIX_CALL;
        if (!ptr->tryParse(lexer, TK_IDENTIFIER) || !ptr->tryParse(lexer, TK_LPAREN) 
            || !ptr->tryParseExprlist(lexer) || !ptr->tryParse(lexer, TK_RPAREN)) 
            PARSEFAIL("Postfix: Illegal function call syntax.");
    } else {
        ptr->expansion = NE_POSTFIX_PRIMARY;
        if (!ptr->tryParsePrimary(lexer))
            PARSEFAIL("Postfix: Illegal primary syntax.");
    } 
    if (DEBUG) cout << "ok parsed postfix" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseIf(MontLexer& lexer) {
    if (DEBUG) cout << "try parse if " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_IF;
    ptr->expansion = NE_IF_SIMPLE;
    if (!ptr->tryParse(lexer, TK_IF) || !ptr->tryParse(lexer, TK_LPAREN) 
        || !ptr->tryParseExpression(lexer) || !ptr->tryParse(lexer, TK_RPAREN) 
        || !ptr->tryParseStatement(lexer))
        PARSEFAIL("If: Failed to parse simple if.");
    Token peek = lexer.peek();
    if (peek.tokenKind == TK_ELSE) {
        ptr->expansion = NE_IF_ELSE;
        if (!ptr->tryParse(lexer, TK_ELSE) || !ptr->tryParseStatement(lexer))
            PARSEFAIL("If: Failed to parse else statement.");
        ptr->memorySize = std::max(ptr->children[4]->memorySize, ptr->children[6]->memorySize);
    } 
    if (DEBUG) {
        cout << "ok parsed if";
        if (ptr->expansion == NE_IF_ELSE) cout << " else";
        cout << endl;
    }
    addChildren(ptr); return true;
}

bool MontNode::tryParseFor(MontLexer& lexer) {
    if (DEBUG) cout << "try parse for " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_FOR;
    if (!ptr->tryParse(lexer, TK_FOR) || !ptr->tryParse(lexer, TK_LPAREN)) 
        PARSEFAIL("For: Expect for keyword and LParen syntax.");
    Token peek = lexer.peek();
    if (isTypeToken(peek)) { 
        // For LParen declaration Semicolon expression Semicolon expression RParen statement
        ptr->expansion = NE_FOR_DECLARATION;
        if (!ptr->tryParseDeclaration(lexer)) 
            PARSEFAIL("For: Invalid for syntax with declaration init.");
    } else {
        ptr->expansion = NE_FOR_EXPRESSION;
        // For LParen expression Semicolon expression Semicolon expression RParen statement
        peek = lexer.peek();
        if (peek.tokenKind != TK_SEMICOLON) {
            if (!ptr->tryParseExpression(lexer)) 
                PARSEFAIL("For: Invalid for syntax with expression init.");
        } else 
            ptr->tryParseEmpty(lexer);
    }
    /*if (!ptr->tryParse(lexer, TK_SEMICOLON) || !ptr->tryParseExpression(lexer) 
        || !ptr->tryParse(lexer, TK_SEMICOLON) || !ptr->tryParseExpression(lexer) 
        || !ptr->tryParse(lexer, TK_RPAREN) || !ptr->tryParseStatement(lexer)) 
        PARSEFAIL("For: Invalid for body.");
    */
    if (!ptr->tryParse(lexer, TK_SEMICOLON)) PARSEFAIL("For: Expect first semicolon.");
    peek = lexer.peek();
    if (peek.tokenKind != TK_SEMICOLON) {
        if (!ptr->tryParseExpression(lexer)) PARSEFAIL("For: Invalid condition expression.");
    } else ptr->tryParseEmpty(lexer);
    if (!ptr->tryParse(lexer, TK_SEMICOLON)) PARSEFAIL("For: Expect second semicolon.");
    peek = lexer.peek();
    if (peek.tokenKind != TK_RPAREN) {
        if (!ptr->tryParseExpression(lexer)) PARSEFAIL("For: Invalid post-loop expression.");
    } else ptr->tryParseEmpty(lexer);
    if (!ptr->tryParse(lexer, TK_RPAREN)) PARSEFAIL("For: Expect RParen.");
    if (!ptr->tryParseStatement(lexer)) PARSEFAIL("For: Invalid body statement.");
    if (DEBUG) cout << "ok parsed for" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseWhile(MontLexer& lexer) {
    if (DEBUG) cout << "try parse while " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_WHILE;
    Token peek = lexer.peek();
    if (peek.tokenKind == TK_WHILE) {
        // While LParen expression RParen statement
        ptr->expansion = NE_WHILE_STANDARD;
        if (!ptr->tryParse(lexer, TK_WHILE) || !ptr->tryParse(lexer, TK_LPAREN)
            || !ptr->tryParseExpression(lexer) || !ptr->tryParse(lexer, TK_RPAREN) 
            || !ptr->tryParseStatement(lexer))
            PARSEFAIL("While: Invalid while-loop syntax.");
    } else {
        // Do statement While LParen expression RParen Semicolon
        ptr->expansion = NE_WHILE_DO;
        if (!ptr->tryParse(lexer, TK_DO) || !ptr->tryParseStatement(lexer) 
            || !ptr->tryParse(lexer, TK_WHILE)
            || !ptr->tryParse(lexer, TK_LPAREN) || !ptr->tryParseExpression(lexer) 
            || !ptr->tryParse(lexer, TK_RPAREN) || !ptr->tryParse(lexer, TK_SEMICOLON)) 
            PARSEFAIL("While: Invalid do-while syntax.");
    }
    if (DEBUG) cout << "ok parsed while" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseBlockitem(MontLexer& lexer) {
    if (DEBUG) cout << "try parse blockitem " << lexer.peek() << endl;
    Token peek = lexer.peek();
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_BLOCKITEM;
    if (isTypeToken(peek)) {
        ptr->expansion = NE_BLOCKITEM_DECLARATION;
        if (!ptr->tryParseDeclaration(lexer) || !ptr->tryParse(lexer, TK_SEMICOLON))
            PARSEFAIL("Blockitem: Illegal variable declaration.");
        if (DEBUG) cout << "ok parsed blockitem declaration" << endl;
        addChildren(ptr); return true;
    } else {
        ptr->expansion = NE_BLOCKITEM_STATEMENT;
        if (!ptr->tryParseStatement(lexer))
            PARSEFAIL("Blockitem: Illegal statement.");
        if (DEBUG) cout << "ok parsed blockitem statement" << endl;
        addChildren(ptr); return true;
    }
}

bool MontNode::tryParseStatement(MontLexer& lexer) {
    if (DEBUG) cout << "try parse statement " << lexer.peek() << endl;
    Token peek = lexer.peek();
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_STATEMENT;
    if (peek.tokenKind == TK_RETURN) {
        ptr->tryParse(lexer, TK_RETURN); ptr->expansion = NE_STATEMENT_RETURN;
        peek = lexer.peek();
        if (peek.tokenKind != TK_SEMICOLON) {
            if (!ptr->tryParseExpression(lexer) || !ptr->tryParse(lexer, TK_SEMICOLON)) 
                PARSEFAIL("Statement: Illegal return statement.");
        } else ptr->tryParse(lexer, TK_SEMICOLON);
        if (DEBUG) cout << "ok parsed return statement" << endl;
        addChildren(ptr); return true;
    } else if (peek.tokenKind == TK_SEMICOLON) {
        ptr->tryParse(lexer, TK_SEMICOLON); ptr->expansion = NE_STATEMENT_EMPTY;
        if (DEBUG) cout << "ok parsed statement empty" << endl;
        addChildren(ptr); return true;  
    } else if (peek.tokenKind == TK_LBRACE) {
        ptr->expansion = NE_STATEMENT_CODEBLOCK;
        if (!ptr->tryParseCodeblock(lexer)) 
            PARSEFAIL("Statement: Expect codeblock with LBrace.");
        if (DEBUG) cout << "ok parsed statement codeblock" << endl;
        addChildren(ptr); return true;
    } else if (peek.tokenKind == TK_IF) {
        ptr->expansion = NE_STATEMENT_IF;
        if (!ptr->tryParseIf(lexer)) 
            PARSEFAIL("Statement: Illegal if statement.");
        if (DEBUG) cout << "ok parsed statement if" << endl;
        addChildren(ptr); return true;
    } else if (peek.tokenKind == TK_FOR) {
        ptr->expansion = NE_STATEMENT_FOR;
        if (!ptr->tryParseFor(lexer)) 
            PARSEFAIL("Statement: Illegal for statement.");
        if (DEBUG) cout << "ok parsed statement for" << endl;
        addChildren(ptr); return true;
    } else if (peek.tokenKind == TK_WHILE || peek.tokenKind == TK_DO) {
        ptr->expansion = NE_STATEMENT_WHILE;
        if (!ptr->tryParseWhile(lexer)) 
            PARSEFAIL("Statement: Illegal while loop statement.");
        if (DEBUG) cout << "ok parsed statement whileloop" << endl;
        addChildren(ptr); return true;
    } else if (peek.tokenKind == TK_BREAK) {
        ptr->expansion = NE_STATEMENT_BREAK;
        if (!ptr->tryParse(lexer, TK_BREAK) || !ptr->tryParse(lexer, TK_SEMICOLON)) 
            PARSEFAIL("Statement: Illegal break statement.");
        if (DEBUG) cout << "ok parsed statement break" << endl;
        addChildren(ptr); return true; 
    } else if (peek.tokenKind == TK_CONTINUE) {
        ptr->expansion = NE_STATEMENT_CONTINUE;
        if (!ptr->tryParse(lexer, TK_CONTINUE) || !ptr->tryParse(lexer, TK_SEMICOLON)) 
            PARSEFAIL("Statement: Illegal continue statement.");
        if (DEBUG) cout << "ok parsed statement continue" << endl;
        addChildren(ptr); return true; 
    } else {
        ptr->expansion = NE_STATEMENT_EXPRESSION;
        if (!ptr->tryParseExpression(lexer) || !ptr->tryParse(lexer, TK_SEMICOLON)) 
            PARSEFAIL("Statement: Illegal expression statement.");
        if (DEBUG) cout << "ok parsed expression statement" << endl;
        addChildren(ptr); return true;
    }
}

bool MontNode::tryParseCodeblock(MontLexer& lexer){
    if (DEBUG) cout << "try parse codeblock " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_CODEBLOCK;
    if (!ptr->tryParse(lexer, TK_LBRACE)) 
        PARSEFAIL("Codeblock: LBrace expected.");
    while (true) {
        Token peek = lexer.peek();
        if (peek.tokenKind == TK_RBRACE) break;
        if (!ptr->tryParseBlockitem(lexer)) 
            PARSEFAIL("Codeblock: Illegal blockitem.");
    }
    ptr->tryParse(lexer, TK_RBRACE); 
    if (DEBUG) cout << "ok parsed codeblock" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseExprlist(MontLexer& lexer) {
    if (DEBUG) cout << "try parse exprlist " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_EXPRLIST;
    Token peek = lexer.peek();
    if (peek.tokenKind != TK_RPAREN) {
        if (!ptr->tryParseExpression(lexer)) 
            PARSEFAIL("Exprlist: Expect expression.");
        peek = lexer.peek();
        while (peek.tokenKind == TK_COMMA) {
            ptr->tryParse(lexer, TK_COMMA);
            if (!ptr->tryParseExpression(lexer)) 
                PARSEFAIL("Exprlist: Expect expression.");
            peek = lexer.peek();
        }
    }
    if (DEBUG) cout << "ok parsed exprlist" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseParameters(MontLexer& lexer) {
    if (DEBUG) cout << "try parse parameters " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_PARAMETERS;
    Token peek = lexer.peek();
    if (peek.tokenKind != TK_RPAREN) {
        if (!ptr->tryParseType(lexer) || !ptr->tryParse(lexer, TK_IDENTIFIER)) 
            PARSEFAIL("Parameters: Expect parameter declaration.");
        peek = lexer.peek();
        while (peek.tokenKind == TK_COMMA) {
            ptr->tryParse(lexer, TK_COMMA);
            if (!ptr->tryParseType(lexer) || !ptr->tryParse(lexer, TK_IDENTIFIER)) 
                PARSEFAIL("Parameters: Expect parameter declartion.");
            peek = lexer.peek();
        }
    }
    if (DEBUG) cout << "ok parsed parameters" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseFunction(MontLexer& lexer) {
    if (DEBUG) cout << "try parse function " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_FUNCTION;
    Token peek = lexer.peek();
    if (peek.tokenKind == TK_VOID) {
        ptr->tryParse(lexer, TK_VOID);
    } else {
        if (!ptr->tryParseType(lexer))
            PARSEFAIL("Function: Expect type.");
    }
    if (!ptr->tryParse(lexer, TK_IDENTIFIER) || !ptr->tryParse(lexer, TK_LPAREN) 
        || !ptr->tryParseParameters(lexer) || !ptr->tryParse(lexer, TK_RPAREN))
        PARSEFAIL("Function: Illegal function definition.");
    peek = lexer.peek();
    if (peek.tokenKind == TK_SEMICOLON) {
        ptr->expansion = NE_FUNCTION_DECLARATION;
        ptr->tryParse(lexer, TK_SEMICOLON);
    } else {
        ptr->expansion = NE_FUNCTION_DEFINITION;
        if (!ptr->tryParseCodeblock(lexer))
            PARSEFAIL("Function: Illegal function body.");
    }
    if (DEBUG) cout << "ok parsed function" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseGlobdecl(MontLexer& lexer) {
    if (DEBUG) cout << "try parse globdecl " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_GLOBDECL;
    if (!ptr->tryParseType(lexer) || !ptr->tryParse(lexer, TK_IDENTIFIER))
        PARSEFAIL("Globdecl: Expect type and identifier.");
    Token peek = lexer.peek();
    if (peek.tokenKind == TK_ASSIGN) {
        if (!ptr->tryParse(lexer, TK_ASSIGN) || !ptr->tryParseValue(lexer))
            PARSEFAIL("Globdecl: Expect plain value.");
    }
    if (!ptr->tryParse(lexer, TK_SEMICOLON))
        PARSEFAIL("Globdecl: Expect semicolon.");
    if (DEBUG) cout << "ok parsed globdecl" << endl;
    addChildren(ptr); return true;
}

bool MontNode::tryParseProgram(MontLexer& lexer) {
    if (DEBUG) cout << "try parse program " << lexer.peek() << endl;
    Mnp ptr = new MontNode(lexer); ptr->kind = NK_PROGRAM;
    Token peek = lexer.peek();
    while (peek.tokenKind != TK_EOF) {
        peek = lexer.nextToken();
        Token p2 = lexer.nextToken();
        Token p3 = lexer.peek(); lexer.putback(p2); lexer.putback(peek);
        if (p3.tokenKind == TK_LPAREN) {
            if (!ptr->tryParseFunction(lexer)) 
                PARSEFAIL("Program: Expect function declaration.");
        } else {
            if (!ptr->tryParseGlobdecl(lexer))
                PARSEFAIL("Program: Expect global variable declartion.");
        }
        peek = lexer.peek();
    }
    if (!ptr->tryParse(lexer, TK_EOF))
        PARSEFAIL("Program: Expect EOF.");
    if (DEBUG) cout << "ok parsed program" << endl;
    addChildren(ptr); return true;
}

MontParser::MontParser(){
    program = new MontNode();
    program->setKind(NK_ROOT);
}

bool MontParser::parse(MontLexer& lexer) {
    bool result = program->tryParseProgram(lexer);
    return result;
}

ostream& operator <<(ostream& stream, MontParser& parser) {
    parser.program->output("", true, stream);
    return stream;
}

void MontNode::output(string tab, bool lastchild, ostream& out) {
    if (kind==NK_ROOT) {
        children[0]->output(tab, lastchild, out);
        return;
    }
    out << tab; out << "o-";
    if (kind==NK_TOKEN) {
        MontTokenNode* n = (MontTokenNode*) this;
        out << (*n).getToken() << endl;
        return;
    }
    out << "(" << row << ":" << column << ") ";
    switch (kind) {
        case NK_ROOT:       out << "root"; break;
        case NK_PROGRAM:    out << "program"; break;
        case NK_FUNCTION:   out << "function [frameSize=" << memorySize << "]"; break;
        case NK_CODEBLOCK:  out << "codeblock"; break;
        case NK_STATEMENT:  out << "statement"; break;
        case NK_EXPRESSION: out << "expression"; break;
        case NK_TYPE:       out << "type"; break;
        case NK_VALUE:      out << "value"; break;
        case NK_UNARY:      out << "unary"; break;
        case NK_MULTIPLICATIVE: out << "multiplicative"; break;
        case NK_ADDITIVE:   out << "additive"; break;
        case NK_LOGICAL_AND:out << "logical_and"; break;
        case NK_LOGICAL_OR: out << "logical_or"; break;
        case NK_EQUALITY:   out << "equality"; break;
        case NK_RELATIONAL: out << "relational"; break;
        case NK_PRIMARY:    out << "primary"; break;
        case NK_ASSIGNMENT: out << "assignment"; break;
        case NK_DECLARATION:out << "declaration"; break;
        case NK_BLOCKITEM:  out << "blockitem"; break;
        case NK_CONDITIONAL:out << "conditional"; break;
        case NK_IF:         out << "if"; break;
        case NK_UNDEFINED:  out << "undefined"; break;
        case NK_FOR:        out << "for"; break;
        case NK_WHILE:      out << "while"; break;
        case NK_PARAMETERS: out << "parameters"; break;
        case NK_EXPRLIST:   out << "exprlist"; break;
        case NK_POSTFIX:    out << "postfix"; break;
        case NK_GLOBDECL:   out << "globdecl"; break;
        case NK_EMPTY:      out << "empty"; break;
        default: out << "???"; break;
    }
    if (expansion != NE_NONE) out << " - ";
    switch (expansion) {
        case NE_ADDITIVE_LEAF: out << "leaf"; break;
        case NE_ADDITIVE_INNER: out << "inner"; break;
        case NE_ASSIGNMENT_ASSIGN: out << "assign"; break;
        case NE_ASSIGNMENT_VALUE: out << "value"; break;
        case NE_BLOCKITEM_DECLARATION: out << "declaration"; break;
        case NE_BLOCKITEM_STATEMENT: out << "statement"; break;
        case NE_CONDITIONAL_INNER: out << "inner"; break;
        case NE_CONDITIONAL_LEAF: out << "leaf"; break;
        case NE_DECLARATION_INIT: out << "init"; break;
        case NE_DECLARATION_SIMPLE: out << "simple"; break;
        case NE_EQUALITY_INNER: out << "inner"; break;
        case NE_EQUALITY_LEAF: out << "leaf"; break;
        case NE_FOR_EXPRESSION: out << "expression"; break;
        case NE_FOR_DECLARATION: out << "declaration"; break;
        case NE_FUNCTION_DEFINITION: out << "definition"; break;
        case NE_FUNCTION_DECLARATION: out << "declaration"; break;
        case NE_IF_ELSE: out << "else"; break;
        case NE_IF_SIMPLE: out << "simple"; break;
        case NE_LAND_INNER: out << "inner"; break;
        case NE_LAND_LEAF: out << "leaf"; break;
        case NE_LOR_INNER: out << "inner"; break;
        case NE_LOR_LEAF: out << "leaf"; break;
        case NE_MULTIPLICATIVE_INNER: out << "inner"; break;
        case NE_MULTIPLICATIVE_LEAF: out << "leaf"; break;
        case NE_POSTFIX_CALL: out << "call"; break;
        case NE_POSTFIX_PRIMARY: out << "primary"; break;
        case NE_PRIMARY_IDENTIFIER: out << "identifier"; break;
        case NE_PRIMARY_PAREN: out << "paren"; break;
        case NE_PRIMARY_VALUE: out << "value"; break;
        case NE_RELATIONAL_INNER: out << "inner"; break;
        case NE_RELATIONAL_LEAF: out << "leaf"; break;
        case NE_STATEMENT_EMPTY: out << "empty"; break;
        case NE_STATEMENT_EXPRESSION: out << "expression"; break;
        case NE_STATEMENT_RETURN: out << "return"; break;
        case NE_STATEMENT_CODEBLOCK: out << "codeblock"; break;
        case NE_STATEMENT_IF: out << "if"; break;
        case NE_STATEMENT_BREAK: out << "break"; break;
        case NE_STATEMENT_CONTINUE: out << "continue"; break;
        case NE_STATEMENT_FOR: out << "for"; break;
        case NE_STATEMENT_WHILE: out << "while"; break;
        case NE_UNARY_OPERATION: out << "operation"; break;
        case NE_UNARY_POSTFIX: out << "postfix"; break;
        case NE_WHILE_DO: out << "do"; break;
        case NE_WHILE_STANDARD: out << "standard"; break;
    }
    if (MontParser::outputType) {
        out << " : ";
        switch (datatype) {
            case DT_BOOL: out << "bool"; break;
            case DT_CHAR: out << "char"; break;
            case DT_INT: out << "int"; break;
            case DT_VOID: out << "void"; break;
        }
    }
    //out << " {" << endl;
    out << endl;
    int cnt = children.size();
    string newtab = tab + (lastchild ? "  " : "| ");
    for (int i=0;i<cnt-1;i++) 
        children[i]->output(newtab, false, out);
    if (cnt>0)
        children[cnt-1]->output(newtab, true, out);
    //out << sp << "}" << endl;
}