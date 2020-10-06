#ifndef MONTCONCEIVER_H
#define MONTCONCEIVER_H

#include "montLog.h"
#include "montParser.h"
#include <vector>
#include <stack>
using std::vector;
using std::stack;

#include <string>
using std::string;
using std::to_string;

#include <iostream>

class MontAssembler;

enum IntermediateType {
    IR_PUSH, // int
    IR_RET,
    IR_NEG,
    IR_NOT,
    IR_LNOT,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_REM,
    IR_EQ,
    IR_NEQ,
    IR_LT,
    IR_GT,
    IR_LE,
    IR_GE,
    IR_LAND,
    IR_LOR,
    IR_FRAMEADDR, // int
    IR_LOAD,
    IR_STORE,
    IR_POP, // int
    IR_LABEL, // str, Function mark, for example Foo
    IR_BUILDFRAME, // int, buildframe 的整数参数应当是不包括储存ra和fp以外的栈帧大小，例如函数若不包括任何局部变量则参数应为0.
    IR_BEQZ, // str
    IR_BNEZ, // str
    IR_BR // str
};

struct MontVariable {
    string name;
    int location;
    MontVariable(string n, int loc) : name(n), location(loc) {}
};

struct MontFunction {
    vector<MontDatatype> para;
    string name;
    MontDatatype ret;
    bool declared;
    bool defined; 
    MontFunction(string name, MontDatatype ret) : name(name), ret(ret) {
        para = vector<MontDatatype>();
        declared = true; defined = false;
    }
    void addPara(MontDatatype p) {para.push_back(p);}
};

// blocking为真的栈帧为实际上的栈帧顶部。
struct MontStackFrame {
    vector<MontVariable> identifiers;
    bool blocking;
    MontStackFrame(bool blocking=false): blocking(blocking) {identifiers = vector<MontVariable>();}
    void push(string name, int loc){identifiers.push_back(MontVariable(name, loc));}
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
    static MontLog logger;
    static bool appendErrorInfo(string str, int row, int column) {
        logger.log("(" + to_string(row) + ":" + to_string(column) + ") " + str);
        return false;
    }
    static bool appendErrorInfo(string str, MontNode* node) {
        return appendErrorInfo(str, node->row, node->column);
    }
    static void clearErrorInfo(){logger.clear();}
    vector<MontIntermediate> irs;
    vector<MontStackFrame> frames;
    int variablePointer; // 指示当前要加入的变量是第几个局部变量，从0开始。
    int labelCounter; // 指示加入的label的名称。
    stack<int> loops;
    void pushVariable(string name);
    void pushFrame(bool blocking);
    void popFrame();
    int getVariable(string name); // 未查询到结果时返回-1，查询到结果应当为声明该变量时对应的variablePointer
    int checkRedeclaration(string name); // 仅查询本块内，即本frame中的
    void pushLoop(int id){loops.push(id);}
    int getCurrentLoop(){return (loops.size()>0) ? loops.top() : -1;}
    void popLoop(){loops.pop();}
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
    bool conceive(MontParser& p) {
        variablePointer = 0;
        labelCounter = 0;
        irs = vector<MontIntermediate>();
        frames = vector<MontStackFrame>(); 
        loops = stack<int>();
        return visit(p.program);
    }
    friend std::ostream& operator <<(std::ostream& stream, MontConceiver& c);
};

#endif