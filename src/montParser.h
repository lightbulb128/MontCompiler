/*
program     :   function EOF
function    :   type Identifier LParen RParen codeblock
codeblock   :   LBrace (statement)* RBrace
statement   :   type Identifier Semicolon
            |   expression Semicolon
            |   Return expression Semicolon
type        :   Integer | Char
expression  :   value
value       :   CharValue | IntValue
*/

#ifndef MONTPARSER_H
#define MONTPARSER_H

#include <vector>
#include <string>
#include <iostream>
#include "montLexer.h"

using std::vector;
using std::string;

class MontNode;
typedef MontNode* MontNodePtr;

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
protected:
    MontNodeKind kind;
    MontNodeExpansion expansion;
    vector<MontNodePtr> children;
    static string errorInfo;
public:
    MontNode(){children = vector<MontNodePtr>();kind = NK_UNDEFINED;expansion = NE_NONE;}
    MontNodeKind getKind(){return kind;}
    void setKind(MontNodeKind k){kind=k;}
    virtual ~MontNode();
    virtual void putback(MontLexer& lexer);
    static bool appendErrorInfo(string str, int row, int column){
        errorInfo += "(" + std::to_string(row) + ":" + std::to_string(column) + ") " 
            + str + "\n"; 
        return false;
    }
    static string getErrorInfo(){return errorInfo;}
    static void resetErrorInfo(){errorInfo = "";}
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
    MontTokenNode(Token tk){requirement = tk.tokenKind; token = tk; kind = NK_TOKEN;}
    void putback(MontLexer& lexer) override;
    Token getToken(){return token;}
};

class MontParser {
private:
    MontNodePtr program;
public:
    MontParser();
    bool parse(MontLexer& lexer);
    string getErrorMessage(){return MontNode::getErrorInfo();}
    friend std::ostream& operator << (std::ostream& stream, MontParser& parser);
};

#endif