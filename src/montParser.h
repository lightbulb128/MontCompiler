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
    NK_TYPE,
    NK_VALUE,
    NK_TOKEN,
    NK_UNDEFINED
};

enum MontNodeExpansion {
    NE_NONE, 
    NE_STATEMENT_VARDEFINE,
    NE_STATEMENT_EXPRESSION,
    NE_STATEMENT_RETURN
};

class MontNode {
    friend class MontConceiver;
protected:
    MontNodeKind kind;
    MontNodeExpansion expansion;
    vector<MontNodePtr> children;
    // static string errorInfo;
    int row, column;
public:
    MontNode(MontLexer& lexer){
        children = vector<MontNodePtr>();kind = NK_UNDEFINED;expansion = NE_NONE;
        lexer.peek(); row = lexer.getCurrentRow(); column = lexer.getCurrentColumn();
    }
    MontNode(){
        children = vector<MontNodePtr>();kind = NK_UNDEFINED;expansion = NE_NONE;
        //lexer.peek(); row = lexer.getCurrentRow(); column = lexer.getCurrentColumn();
    }
    MontNodeKind getKind(){return kind;}
    void setKind(MontNodeKind k){kind=k;}
    virtual ~MontNode();
    virtual void putback(MontLexer& lexer);
    static bool appendErrorInfo(string str, int row, int column);
    //void trySetRC(MontLexer& lexer){lexer.peek(); if (children.size()==0) row=lexer.getCurrentRow(), column=lexer.getCurrentColumn();}
    void addChildren(MontNodePtr ptr); 
    bool tryParse(MontLexer& lexer, TokenKind tk);
    bool isValueToken(Token& t);
    bool tryParseValue(MontLexer& lexer);
    bool tryParseExpression(MontLexer& lexer); 
    bool isTypeToken(Token& t);
    bool tryParseType(MontLexer& lexer);
    bool tryParseStatement(MontLexer& lexer);
    bool tryParseCodeblock(MontLexer& lexer);
    bool tryParseFunction(MontLexer& lexer); 
    bool tryParseProgram(MontLexer& lexer);
    void output(int tabcount, std::ostream& stream);
};

// store nothing in children.
class MontTokenNode : public MontNode {
private: 
    TokenKind requirement;
    Token token;
public:
    MontTokenNode(Token tk){
        requirement = tk.tokenKind; token = tk; kind = NK_TOKEN;
        row = tk.row; column = tk.column;
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
};

#endif