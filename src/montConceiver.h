#ifndef MONTCONCEIVER_H
#define MONTCONCEIVER_H

#include "montLog.h"
#include "montParser.h"
#include <vector>
using std::vector;

#include <string>
using std::string;
using std::to_string;

#include <iostream>

class MontAssembler;

enum IntermediateType {
    IR_PUSH,
    IR_RET
};

struct MontIntermediate {
public:
    IntermediateType code;
    int num; string str;
private:
    MontIntermediate(IntermediateType c, int intarg = 0, string strarg = ""):
        code(c), num(intarg), str(strarg){}
public:
    static MontIntermediate simple(IntermediateType type){return MontIntermediate(type);}
    static MontIntermediate intcode(IntermediateType type, int num){return MontIntermediate(type,num);}
    static MontIntermediate strcode(IntermediateType type, string str){return MontIntermediate(type,0,str);}
    string toAssembly();
    string toString();
    friend std::ostream& operator <<(std::ostream& stream, MontIntermediate ir);
};

// Convert AST to IR. Basically a stack machine.
class MontConceiver {
private:
    friend class MontAssembler;
    vector<MontIntermediate> irs;
    static MontLog logger;
    static bool appendErrorInfo(string str, int row, int column) {
        logger.log("(" + to_string(row) + ":" + to_string(column) + ") " + str);
        return false;
    }
    static bool appendErrorInfo(string str, MontNode* node) {
        return appendErrorInfo(str, node->row, node->column);
    }
    static void clearErrorInfo(){logger.clear();}
public:
    static string getErrorInfo(){return logger.get();}
    void add(MontIntermediate m){irs.push_back(m);}
    MontConceiver();
    bool visit(MontNodePtr node);
    bool visitChildren(MontNodePtr node);
    bool visitChild(MontNodePtr node, int id){return visit(node->children[id]);}
    Token getTokenChild(MontNodePtr node, int id){
        MontTokenNode* ptr = (MontTokenNode*) node->children[id];
        return ptr->getToken();
    }
    bool conceive(MontParser& p) {return visit(p.program);}
    friend std::ostream& operator <<(std::ostream& stream, MontConceiver& c);
};

#endif