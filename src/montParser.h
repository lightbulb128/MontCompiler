#ifndef MONTPARSER_H
#define MONTPARSER_H

#include <vector>
#include <string>
#include <iostream>
#include "montLexer.h"
#include "montLog.h"

using std::vector;
using std::string;

class MontNode;
typedef MontNode* MontNodePtr;
class MontConceiver;

enum MontNodeKind {
    NK_ROOT,
    NK_PROGRAM,
    NK_FUNCTION,
    NK_CODEBLOCK,
    NK_STATEMENT, 
    NK_EXPRESSION,
    NK_LOGICAL_OR,
    NK_LOGICAL_AND,
    NK_EQUALITY,
    NK_RELATIONAL,
    NK_ADDITIVE,
    NK_MULTIPLICATIVE,
    NK_PRIMARY, 
    NK_UNARY, 
    NK_TYPE,
    NK_VALUE,
    NK_TOKEN,
    NK_DECLARATION, 
    NK_ASSIGNMENT, 
    NK_BLOCKITEM,
    NK_IF, 
    NK_CONDITIONAL, 
    NK_FOR,
    NK_WHILE,
    NK_PARAMETERS,
    NK_EXPRLIST,
    NK_POSTFIX,
    NK_UNDEFINED,
    NK_EMPTY, // 占位符，例如 for 的 expr 可能为空。
};

enum MontNodeExpansion {
    NE_NONE, 
    NE_STATEMENT_EXPRESSION,
    NE_STATEMENT_RETURN,
    NE_STATEMENT_CODEBLOCK,
    NE_STATEMENT_EMPTY,
    NE_STATEMENT_IF, 
    NE_STATEMENT_FOR,
    NE_STATEMENT_WHILE,
    NE_STATEMENT_BREAK,
    NE_STATEMENT_CONTINUE,
    NE_UNARY_POSTFIX,
    NE_UNARY_OPERATION,
    NE_PRIMARY_VALUE,
    NE_PRIMARY_PAREN,
    NE_PRIMARY_IDENTIFIER,
    NE_MULTIPLICATIVE_LEAF,
    NE_MULTIPLICATIVE_INNER,
    NE_ADDITIVE_LEAF,
    NE_ADDITIVE_INNER,
    NE_LOR_LEAF,
    NE_LOR_INNER,
    NE_LAND_LEAF,
    NE_LAND_INNER,
    NE_EQUALITY_INNER,
    NE_EQUALITY_LEAF,
    NE_RELATIONAL_LEAF,
    NE_RELATIONAL_INNER,
    NE_DECLARATION_SIMPLE,
    NE_DECLARATION_INIT,
    NE_ASSIGNMENT_VALUE,
    NE_ASSIGNMENT_ASSIGN,
    NE_BLOCKITEM_STATEMENT,
    NE_BLOCKITEM_DECLARATION,
    NE_IF_SIMPLE,
    NE_IF_ELSE,
    NE_CONDITIONAL_LEAF,
    NE_CONDITIONAL_INNER,
    NE_FOR_EXPRESSION,
    NE_FOR_DECLARATION,
    NE_WHILE_STANDARD,
    NE_WHILE_DO,
    NE_FUNCTION_DECLARATION,
    NE_FUNCTION_DEFINITION,
    NE_POSTFIX_PRIMARY,
    NE_POSTFIX_CALL
};

enum MontDatatype {
    DT_VOID,
    DT_INT,
    DT_CHAR,
    DT_BOOL
};

class MontNode {
    friend class MontConceiver;
protected:
    MontNodeKind kind;
    MontNodeExpansion expansion;
    vector<MontNodePtr> children;
    MontDatatype datatype; // 在conceiver中得到它的值。
    // static string errorInfo;
    int row, column;
    int memorySize; 
    // 用于生成栈帧，表示该树节点对应代码中所需要声明局部变量的大小（一定是4的倍数），
    // 例如 if () {A} else {B} 中 memorySize 应当是 A B 中所声明局部变量空间中的较大值。
public:
    MontNode(MontLexer& lexer){
        children = vector<MontNodePtr>();kind = NK_UNDEFINED;expansion = NE_NONE;
        lexer.peek(); row = lexer.getCurrentRow(); column = lexer.getCurrentColumn();
        memorySize = 0; datatype = DT_VOID;
    }
    MontNode(){
        children = vector<MontNodePtr>();kind = NK_UNDEFINED;expansion = NE_NONE;
        row = column = 0; memorySize = 0; datatype = DT_VOID;
        //lexer.peek(); row = lexer.getCurrentRow(); column = lexer.getCurrentColumn();
    }
    static void tryStart();
    static void tryEnd();
    void copyRC(MontNode& from) {row=from.row; column=from.column;}
    MontNodeKind getKind(){return kind;}
    void setKind(MontNodeKind k){kind=k;}
    virtual ~MontNode();
    virtual void putback(MontLexer& lexer);
    static bool appendErrorInfo(string str, int row, int column);
    //void trySetRC(MontLexer& lexer){lexer.peek(); if (children.size()==0) row=lexer.getCurrentRow(), column=lexer.getCurrentColumn();}
    void addChildren(MontNodePtr ptr); 
    bool tryParse(MontLexer& lexer, TokenKind tk);
    bool tryParseEmpty(MontLexer& lexer);
    static bool isValueToken(Token& t);
    bool tryParseValue(MontLexer& lexer);
    static bool isUnaryOperatorToken(Token& t);
    bool tryParseUnary(MontLexer& lexer);
    static bool isEqualityOperatorToken(Token& t);
    static bool isRelationalOperatorToken(Token& t);
    bool tryParseEquality(MontLexer& lexer);
    bool tryParseRelational(MontLexer& lexer);
    bool tryParseLogicalOr(MontLexer& lexer);
    bool tryParseLogicalAnd(MontLexer& lexer);
    static bool isAdditiveOperatorToken(Token& t);
    bool tryParseAdditive(MontLexer& lexer);
    static bool isMultiplicativeOperatorToken(Token& t);
    bool tryParseMultiplicative(MontLexer& lexer);
    bool tryParsePrimary(MontLexer& lexer);
    bool tryParseExpression(MontLexer& lexer); 
    static bool isTypeToken(Token& t);
    bool tryParseType(MontLexer& lexer);
    bool tryParseStatement(MontLexer& lexer);
    bool tryParseCodeblock(MontLexer& lexer);
    bool tryParseFunction(MontLexer& lexer); 
    bool tryParseProgram(MontLexer& lexer);
    bool tryParseDeclaration(MontLexer& lexer);
    bool tryParseAssignment(MontLexer& lexer);
    bool tryParseBlockitem(MontLexer& lexer);
    bool tryParseIf(MontLexer& lexer);
    bool tryParseConditional(MontLexer& lexer);
    bool tryParseFor(MontLexer& lexer);
    bool tryParseWhile(MontLexer& lexer);
    bool tryParseParameters(MontLexer& lexer);
    bool tryParseExprlist(MontLexer& lexer);
    bool tryParsePostfix(MontLexer& postfix);
    void output(string tab, bool lastchild, std::ostream& stream);
};

// store nothing in children.
class MontTokenNode : public MontNode {
private: 
    TokenKind requirement;
    Token token;
public:
    MontTokenNode(Token tk){
        requirement = tk.tokenKind; token = tk; kind = NK_TOKEN;
        row = tk.row; column = tk.column; memorySize = 0;
    }
    void putback(MontLexer& lexer) override;
    Token getToken(){return token;}
};

class MontParser {
    friend class MontConceiver;
private:
    static MontLog logger; 
    MontNodePtr program;
public:
    static bool outputType; 
    MontParser();
    bool parse(MontLexer& lexer);
    friend std::ostream& operator << (std::ostream& stream, MontParser& parser);
    static bool appendErrorInfo(string str, int row, int column){
        logger.log("(" + std::to_string(row) + ":" + std::to_string(column) + ") " 
            + str); 
        return false;
    }
    static string getErrorInfo(){return logger.get();}
    static void resetErrorInfo(){logger.clear();}
    static void tryStart(){logger.trystart();}
    static void tryEnd(){logger.tryend();}
};

#endif