#include "montConceiver.h"
#include "montParser.h"
#include "montLexer.h"
#include <assert.h>

#define IRSIM(c) (MontIntermediate::simple(c))
#define IRINT(c, v) (MontIntermediate::intcode(c, (v)))
#define IRSTR(c, s) (MontIntermediate::strcode(c, (s)))
#define IRCOM(c, v, s) (MontIntermediate::comcode(c, v, s))
#define NRC node->row, node->column
#define CRC node->kind << "-" << node->expansion << " (" << node->row << ":" << node->column << ")"
#define LW2 string("lw t1, 4(sp)\nlw t2, 0(sp)\n")
#define LW1 string("lw t1, 0(sp)\n")
#define SW1 string("sw t1, 0(sp)\n")
#define BSP string("addi sp, sp, 4\n")
#define getLabel(desc) ".L"+to_string(labelId)+"_"+(desc)
#define DETERMINECHILD(id) {flag = visitChild(node, id, asLvalue); setNodeTypeFromChild(node, id); return flag;}

MontLog MontConceiver::logger = MontLog();

typedef MontNodePtr Mnp;

const bool DEBUG = false;

using std::ostream;
using std::cerr;
using std::endl;

string MontIntermediate::toString(){
    switch (code) {
        case IR_ADD: return "ADD";
        case IR_BEQZ: return "BEQZ " + str;
        case IR_BNEZ: return "BNEZ " + str;
        case IR_BR: return "BR " + str;
        case IR_BUILDFRAME: return "BUILDFRAME " + to_string(num);
        case IR_DIV: return "DIV";
        case IR_EQ: return "EQ";
        case IR_FRAMEADDR: return "FRAMEADDR " + to_string(num);
        case IR_GE: return "GE";
        case IR_GLOBADDR: return "GLOBADDR " + str;
        case IR_GT: return "GT";
        case IR_LABEL: return str + ":";
        case IR_LAND: return "LAND";
        case IR_LE: return "LE";
        case IR_LOAD: return "LOAD";
        case IR_LNOT: return "LNOT";
        case IR_LOR: return "LOR";
        case IR_LT: return "LT";
        case IR_POP: return "POP";
        case IR_MUL: return "MUL";
        case IR_NEG: return "NEG";
        case IR_NEQ: return "NEQ";
        case IR_NOT: return "NOT";
        case IR_PUSH: return string("PUSH ") + to_string(num);
        case IR_REM: return "REM";
        case IR_RET: return "RET";
        case IR_RETV: return "RETV";
        case IR_STORE: return "STORE";
        case IR_SUB: return "SUB";
        case IR_BOOL: return "BOOL";
        case IR_CALL: return "CALL " + str + " " + to_string(num);
        case IR_CALLV: return "CALLV " + str + " " + to_string(num);
        case IR_SWAP: return "SWAP";
        default: return "IRERROR";
    }
}

inline string _L(string s){return s+"\n";}

string MontIntermediate::toAssembly(){
    switch (code) {
        case IR_ADD: return LW2 + _L("add t1, t1, t2") + BSP + SW1;
        case IR_BEQZ: return LW1 + BSP + _L("beqz t1, " + str);
        case IR_BNEZ: return LW1 + BSP + _L("bnez t1, " + str);
        case IR_BOOL: return LW1 + _L("snez t1, t1") + SW1;
        case IR_BR: return _L("j " + str);
        case IR_BUILDFRAME: return 
                _L("sw ra, -4(sp)") + 
                _L("sw fp, -8(sp)") + 
                _L("ori fp, sp, 0") + 
                _L("addi sp, sp, -" + to_string(num + 8));
        case IR_CALL: return 
                _L("call " + str) +
                _L("addi sp, sp, " + to_string(num*4-4)) +
                _L("sw a0, 0(sp)");
        case IR_CALLV: return 
                _L("call " + str) +
                _L("addi sp, sp, " + to_string(num*4));
        case IR_DIV: return LW2 + _L("div t1, t1, t2") + BSP + SW1;
        case IR_EQ: return LW2 + 
                _L("sub t1, t1, t2") + 
                _L("seqz t1, t1") + BSP + SW1;
        case IR_FRAMEADDR: return 
                _L("addi sp, sp, -4") +
                _L("addi t1, fp, " + to_string(-12-num*4)) +
                _L("sw t1, 0(sp)");
        case IR_GE: return LW2 + 
                _L("slt t1, t1, t2") +
                _L("xori t1, t1, 1") + BSP + SW1;
        case IR_GLOBADDR: return 
                _L("addi sp, sp, -4") +
                _L("la t1, " + str) + SW1;
        case IR_GT: return LW2 + _L("sgt t1, t1, t2") + BSP + SW1;
        case IR_LABEL: return str + ":";
        case IR_LAND: return LW2 +
                _L("snez t1, t1") + 
                _L("snez t2, t2") + 
                _L("and t1, t1, t2") + BSP + SW1;
        case IR_LE:
            return LW2 + 
                _L("sgt t1, t1, t2") +
                _L("xori t1, t1, 1") +BSP+SW1;
        case IR_LNOT: return LW1 + _L("seqz t1,t1") + SW1;
        case IR_LOAD: return 
                _L("lw t1, 0(sp)") +
                _L("lw t1, 0(t1)") +
                _L("sw t1, 0(sp)");
        case IR_LOR:
            return LW2 + 
                _L("or t1, t1, t2") + 
                _L("snez t1, t1")+BSP+SW1;
        case IR_LT: return LW2 + _L("slt t1, t1, t2")+BSP+SW1;
        case IR_MUL: return LW2 + _L("mul t1, t1, t2")+BSP+SW1;
        case IR_NEG: return LW1 + _L("neg t1,t1") + SW1;
        case IR_NEQ: return LW2 + 
                _L("sub t1, t1, t2") + 
                _L("snez t1, t1") + BSP + SW1;
        case IR_NOT: return LW1 + _L("not t1,t1") + SW1;
        case IR_POP: return BSP;
        case IR_PUSH: return 
                _L("addi sp, sp, -4") + 
                _L("li t1, " + to_string(num)) + SW1;
        case IR_REM: return LW2 + _L("rem t1, t1, t2")+BSP+SW1;
        case IR_RET: return 
                _L("lw a0, 0(sp)") +
                _L("addi sp, sp, 4") + 
                _L("ori sp, fp, 0") +
                _L("lw ra, -4(sp)") +
                _L("lw fp, -8(sp)") + 
                _L("jr ra"); 
        case IR_RETV: return  
                _L("ori sp, fp, 0") +
                _L("lw ra, -4(sp)") +
                _L("lw fp, -8(sp)") + 
                _L("jr ra"); 
        case IR_STORE: return LW2 + _L("sw t1, 0(t2)") + BSP;
        case IR_SUB: return LW2 + _L("sub t1, t1, t2")+BSP+SW1;
        case IR_SWAP: return LW2 +
                _L("sw t1, 0(sp)") +
                _L("sw t2, 4(sp)");
        default: return "nop";
    }
}

ostream& operator <<(ostream& out, MontIntermediate ir){
    if (ir.code != IR_LABEL) out << "    " << ir.toString(); 
    else out << ir.toString();
    return out;
}

MontConceiver::MontConceiver() {
    irs = vector<MontIntermediate>();
    frames = vector<MontStackFrame>();
    functions = vector<MontFunction>();
}

bool MontConceiver::visitChildren(MontNodePtr ptr, bool asLvalue){
    for (auto i = ptr->children.begin(); i != ptr->children.end(); i++) 
        if (!visit(*i, asLvalue)) return false;
    return true;
}

bool MontConceiver::visit(MontNodePtr node, bool asLvalue) {

    bool flag = true;

    switch (node->kind) {
        // Please add according to alphabetic order
        case NK_ADDITIVE: {
            if (DEBUG) std::cout << "conceiving additive " << CRC << endl; 
            if (node->expansion == NE_ADDITIVE_LEAF) 
                DETERMINECHILD(0)
            else if (node->expansion == NE_ADDITIVE_INNER) {
                if (!visitChild(node, 0, false) || !visitChild(node, 2, false)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind != TK_PLUS && op.tokenKind != TK_MINUS)
                    return appendErrorInfo("Additive: Expect operator token.", node->row, node->column);
                Mnp a = node->children[0], b = node->children[2];
                if (isVoid(a) || isVoid(b)) 
                    return appendErrorInfo("Additive: Addition with void.", NRC);
                else if (op.tokenKind == TK_PLUS) {
                    if (isBroadint(a) && isBroadint(b)) {
                        parseType(MontType(DT_INT), a->datatype);
                        parseType(MontType(DT_INT), b->datatype);
                        add(IRSIM(IR_ADD)); node->datatype = MontType(DT_INT);
                    } else if (isInt(a) && isPointer(b)) { // imm adr
                        add(IRSIM(IR_SWAP)); // adr imm
                        add(IRINT(IR_PUSH, 4)); // adr imm 4
                        add(IRSIM(IR_MUL)); // adr 4imm
                        add(IRSIM(IR_ADD)); node->datatype = b->datatype;
                    } else if (isPointer(a) && isInt(b)) { // adr imm
                        add(IRINT(IR_PUSH, 4)); // adr imm 4
                        add(IRSIM(IR_MUL)); // adr 4imm
                        add(IRSIM(IR_ADD)); node->datatype = a->datatype;
                    } else if (isPointer(a) && isPointer(b)) 
                        return appendErrorInfo("Additive: Adding two pointers.", NRC);
                    else 
                        return appendErrorInfo("Additive: Addition of operand types undefined.", NRC);
                } else if (op.tokenKind == TK_MINUS) {
                    if (isBroadint(a) && isBroadint(b)) {
                        parseType(MontType(DT_INT), a->datatype);
                        parseType(MontType(DT_INT), b->datatype);
                        add(IRSIM(IR_SUB)); node->datatype = MontType(DT_INT);
                    } else if (isInt(a) && isPointer(b)) 
                        return appendErrorInfo("Additive: Integer minus pointer.", NRC);
                    else if (isPointer(a) && isInt(b)) { // adr imm
                        add(IRINT(IR_PUSH, 4)); // adr imm 4
                        add(IRSIM(IR_MUL)); // adr 4imm
                        add(IRSIM(IR_SUB)); node->datatype = a->datatype;
                    } else if (isPointer(a) && isPointer(b)) {
                        if (a->datatype != b->datatype) 
                            return appendErrorInfo("Addition: Pointer types of substraction do not match.", NRC);
                        add(IRSIM(IR_SUB));
                        add(IRINT(IR_PUSH, 4));
                        add(IRSIM(IR_DIV));
                    } else 
                        return appendErrorInfo("Additive: Substraction of operand types undefined.", NRC);
                }
                node->setLvalue(false);
                return true;
            }
            else
                return appendErrorInfo("Additive: Undefined additive syntax.", node->row, node->column);
            break;
        }
        case NK_ASSIGNMENT: { 
            if (DEBUG) std::cout << "conceiving assignment " << CRC << endl; 
            if (node->expansion == NE_ASSIGNMENT_VALUE) 
                DETERMINECHILD(0)
            else if (node->expansion == NE_ASSIGNMENT_ASSIGN) { // unary Assign expression
                MontType type;
                flag = visitChild(node, 2, false);
                if (isVoid(node->children[2])) 
                    return appendErrorInfo("Assignent: Expression is void.", NRC);
                //cerr << type << node->children[2]->datatype << endl;
                if (!flag) return false;
                if (!visitChild(node, 0, true)) return false;
                if (!node->children[0]->isLvalue()) 
                    return appendErrorInfo("Assignment: Assign to rvalue.", NRC);
                type = node->children[0]->datatype;
                if (DEBUG) std::cout << "assignment " << node->children[2]->datatype << " to " << type << endl;
                if (type != node->children[2]->datatype)
                    return appendErrorInfo("Assignment: Value type does not match.", NRC);
                if (!parseType(type, node->children[2]->datatype))
                    return appendErrorInfo("Assignment: Failed to cast implicitly.",NRC);
                setNodeType(node, type); node->setLvalue(false);
                add(IRSIM(IR_STORE));
                return true;
            } else 
                return appendErrorInfo("Assignment: Undefined assignment syntax.", NRC);
        }
        case NK_BLOCKITEM: {
            if (DEBUG) std::cout << "conceiving blockitem " << CRC << endl; 
            if (node->expansion == NE_BLOCKITEM_DECLARATION) 
                return visitChild(node, 0, false);
            else if (node->expansion == NE_BLOCKITEM_STATEMENT) 
                return visitChild(node, 0, false);
            else 
                return appendErrorInfo("Blockitem: Undefined blockitem syntax.", NRC);
        }
        case NK_CODEBLOCK: { 
            if (DEBUG) std::cout << "conceiving codeblock " << CRC << endl; 
            return visitChildren(node, false);
            break;
        }
        case NK_CONDITIONAL: {
            if (DEBUG) std::cout << "conceiving conditional " << CRC << endl; 
            if (node->expansion == NE_CONDITIONAL_LEAF)  // logical_or 
                DETERMINECHILD(0)
            else if (node->expansion == NE_CONDITIONAL_INNER) { // logical_or Question expression Colon conditional
                int labelId = labelCounter ++;
                if (!visitChild(node, 0, false)) return false;
                Mnp cond = node->children[0];
                if (isVoid(cond))
                    return appendErrorInfo("Conditional: Void condition.", NRC);
                if (isPointer(cond))
                    return appendErrorInfo("Conditional: Pointer condition.", NRC);
                add(IRSTR(IR_BEQZ, getLabel("CONDITIONAL_FALSE")));
                if (!visitChild(node, 2, false)) return false;
                add(IRSTR(IR_BR, getLabel("CONDITIONAL_END")));
                add(IRSTR(IR_LABEL, getLabel("CONDITIONAL_FALSE")));
                if (!visitChild(node, 4, false)) return false;
                add(IRSTR(IR_LABEL, getLabel("CONDITIONAL_END")));
                Mnp a = node->children[2], b = node->children[4];
                if (isVoid(a) || isVoid(b)) 
                    return appendErrorInfo("Conditional: Selection with void.", NRC);
                if (a->datatype != b->datatype) 
                    return appendErrorInfo("Conditional: Selection type inconsistent.", NRC);
                setNodeTypeFromChild(node, 2); node->setLvalue(false);
                return true;
            } else 
                return appendErrorInfo("Conditional: Undefined conditional syntax.", NRC);
        }
        case NK_DECLARATION: { // type Identifier [Assign expression]
            if (DEBUG) std::cout << "conceiving declaration " << CRC << endl; 
            MontType type = getType(node->children[0]);
            Token name = getTokenChild(node, 1);
            int id = checkRedeclaration(name.identifier);
            if (id!=-1) 
                return appendErrorInfo("Declaration: Variable redeclared: " + name.identifier + ".", NRC);
            if (node->expansion == NE_DECLARATION_INIT) {
                pushVariable(name.identifier, type, node->memorySize);
                flag = visitChild(node, 3, false);
                if (type != node->children[3]->datatype)
                    return appendErrorInfo("Declaration: Value type does not match.",NRC);
                if (!parseType(type, node->children[3]->datatype))
                    return appendErrorInfo("Declaration: Failed to cast implicitly.",NRC);
                if (!flag) return false;
            } else if (node->expansion == NE_DECLARATION_SIMPLE) {
                pushVariable(name.identifier, type, node->memorySize);
                add(IRINT(IR_PUSH, 0));
            }
            else if (node->expansion == NE_DECLARATION_ARRAY) { // 产生数组大小那么多个默认值0
                // type Identifier LB Value RB LB value RB
                int dim = (node->children.size()-2)/3;
                for (int i=dim-1;i>=0;i--) {
                    int nid = i*3+3; int arraylength;
                    MontNode::isValueInteger(node->children[nid], &arraylength);
                    type = MontType(new MontType(type), arraylength);
                }
                if (DEBUG) std::cout << "declare array " << type << endl;
                pushVariable(name.identifier, type, node->memorySize);
                int cnt = node->memorySize/4;
                for (int i=0;i<cnt;i++) 
                    add(IRINT(IR_PUSH, 0));
            } else return appendErrorInfo("Declaration: Undefined declaration syntax.", NRC);
            getVariable(name.identifier, nullptr); 
            add(IRSIM(IR_STORE));
            add(IRSIM(IR_POP));
            //if (DEBUG) std::cout << "ok conceived declaration" << endl;
            return true;
        }
        case NK_EMPTY: {
            if (DEBUG) std::cout << "conceiving empty " << CRC << endl; 
            return true;
        }
        case NK_EQUALITY: {
            if (DEBUG) std::cout << "conceiving equality " << CRC << endl; 
            if (node->expansion == NE_EQUALITY_LEAF) 
                DETERMINECHILD(0)
            else if (node->expansion == NE_EQUALITY_INNER) {
                if (!visitChild(node, 0, false) || !visitChild(node, 2, false)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_EQUAL)
                    add(IRSIM(IR_EQ));
                else if (op.tokenKind == TK_NOT_EQUAL)
                    add(IRSIM(IR_NEQ));
                else return appendErrorInfo("Equality: Expect operator ==, != token.", node->row, node->column);
                Mnp a = node->children[0], b = node->children[2];
                if (a->datatype != b->datatype)
                    return appendErrorInfo("Equality: Types do not match.", NRC);
                if (isVoid(a) || isVoid(b)) 
                    return appendErrorInfo("Equality: Equality with void.", NRC);
                else node->datatype = MontType(DT_BOOL); 
                node->setLvalue(false);
                return true;
            }
            else
                return appendErrorInfo("Equality: Undefined equality syntax.", node->row, node->column);
            break;
        }
        case NK_EXPRESSION: {
            if (DEBUG) std::cout << "conceiving expression " << CRC << endl; 
            DETERMINECHILD(0)
            break;
        }
        case NK_EXPRLIST: {
            if (DEBUG) std::cout << "conceiving exprlist " << CRC << endl; 
            return appendErrorInfo("Exprlist: Should not conceive exprlist internly.",NRC);
        }
        case NK_FOR: { // For ( pre ; expr ; expr ) statement
            if (DEBUG) std::cout << "conceiving for " << CRC << endl; 
            if (node->expansion != NE_FOR_DECLARATION && node->expansion != NE_FOR_EXPRESSION)
                return appendErrorInfo("For: Undefined for syntax.", NRC); 
            int labelId = labelCounter ++; pushLoop(labelId);
            pushFrame(false);
            if (!visitChild(node, 2, false)) return false;
            if (node->expansion == NE_FOR_EXPRESSION && node->children[2]->kind != NK_EMPTY && node->children[2]->datatype != DT_VOID) 
                add(IRSIM(IR_POP));
            add(IRSTR(IR_LABEL, getLabel("LOOP_BEGIN")));
            if (!visitChild(node, 4, false)) return false;
            if (node->children[4]->kind != NK_EMPTY) {
                Mnp cond = node->children[4];
                if (isVoid(cond))
                    return appendErrorInfo("For: Void condition.", NRC);
                if (isPointer(cond))
                    return appendErrorInfo("For: Pointer condition.", NRC);
                add(IRSTR(IR_BEQZ, getLabel("LOOP_BREAK")));
            }
            pushFrame(false);
            if (!visitChild(node, 8, false)) return false;
            add(IRSTR(IR_LABEL, getLabel("LOOP_CONTINUE")));
            if (!visitChild(node, 6, false)) return false;
            if (node->children[6]->kind != NK_EMPTY && node->children[6]->datatype != DT_VOID)
                add(IRSIM(IR_POP));
            popFrame();
            add(IRSTR(IR_BR, getLabel("LOOP_BEGIN")));
            add(IRSTR(IR_LABEL, getLabel("LOOP_BREAK")));
            popFrame();
            return true;
            break;
        }
        case NK_FUNCTION: { // type|Void Identifier LP parameters RP codeblock|Semicolon
            if (DEBUG) std::cout << "conceiving function " << CRC << endl; 
            if (node->expansion != NE_FUNCTION_DECLARATION && node->expansion != NE_FUNCTION_DEFINITION)
                return appendErrorInfo("Function: Undefined function syntax.", NRC);
            bool isDefinition = node->expansion == NE_FUNCTION_DEFINITION;
            MontType type = MontType();
            if (node->children[0]->kind == NK_TYPE) 
                type = getType(node->children[0]);
            Token identifier = getTokenChild(node, 1);
            if (checkGlobal(identifier.identifier, false)) 
                return appendErrorInfo("Function: Function name conflicts with global variable.", NRC);
            int functionId = getFunction(identifier.identifier);
            MontFunction newfunc = MontFunction(identifier.identifier, type);
            // 产生参数列表
            Mnp plist = node->children[3]; // type identifier Comma type identifier Comma ...
            int parameterCount = (plist->children.size()+1)/3;
            vector<string> parameterNames = vector<string>();
            for (int i=0;i<parameterCount;i++) {
                MontType ptype = getType(plist->children[i*3]);
                Token nametoken = getTokenChild(plist, i*3+1);
                for (int j=0;j<i;j++) 
                    if (parameterNames[j]==nametoken.identifier)
                        return appendErrorInfo("Function: Parameter identifier repetitive.", NRC);
                parameterNames.push_back(nametoken.identifier);
                if (ptype.isVoid())
                    return appendErrorInfo("Function: Void parameter.", NRC);
                newfunc.addPara(ptype);
            }
            // 检测是否与已经声明的函数兼容
            if (functionId != -1) {
                MontFunction& oldfunc = functions[functionId];
                if (oldfunc.defined && isDefinition) 
                    return appendErrorInfo("Function: Function " + newfunc.name + " redefined.",NRC);
                if (!oldfunc.checkConsistency(newfunc))
                    return appendErrorInfo("Function: Function parameters inconsistant with previous declaration.", NRC);
                if (isDefinition) oldfunc.defined = true;
            } else {
                functions.push_back(newfunc);
                functionId = functions.size()-1;
            }
            if (isDefinition) {
                currentFunction = functionId;
                int oldPointer = variablePointer;
                add(IRSTR(IR_LABEL, identifier.identifier));
                add(IRINT(IR_BUILDFRAME, node->memorySize)); 
                pushFrame(true);
                // 添加参数作为局部变量
                for (int i=0;i<parameterCount;i++) {
                    Token parameterIdentifier = getTokenChild(plist, i*3+1);
                    pushParameter(parameterIdentifier.identifier, newfunc.para[i], i);
                }
                flag = visitChild(node, 5, false);
                // cerr << frames[frames.size()-1];
                popFrame();
                currentFunction = -1;
                // 如若最后一条指令不是ret，则添加一个ret，默认返回值为0.
                if (type != DT_VOID && irs[irs.size()-1].code != IR_RET) {
                    add(IRINT(IR_PUSH, 0));
                    add(IRSIM(IR_RET));
                } else if (type == DT_VOID && irs[irs.size()-1].code != IR_RETV) {
                    add(IRSIM(IR_RETV));
                }
                variablePointer = oldPointer;
            }
            return flag;
            break;
        }
        case NK_GLOBDECL: { // type Identifier (Assign Value)? Semicolon
            if (DEBUG) std::cout << "conceiving globdecl " << CRC << endl; 
            string name = getTokenChild(node, 1).identifier; 
            MontType type = getType(node->children[0]);
            if (checkGlobal(name, true)) 
                return appendErrorInfo("Globdecl: Global variable conflicts with a defined variable/function.", NRC);
            if (node->expansion == NE_GLOBDECL_SIMPLE) {
                pushBSS(name, type, 4);
            } else if (node->expansion == NE_GLOBDECL_INIT) {
                MontType vtype;
                int v = getValue(node->children[3], &vtype);
                if (vtype!=type)
                    return appendErrorInfo("Globdecl: Init value type does not match.", NRC);
                pushData(name, type, v);
            } else if (node->expansion == NE_GLOBDECL_ARRAY) {
                // type Identifier LB Value RB LB value RB Semicolon
                int dim = (node->children.size()-3)/3;
                for (int i=dim-1;i>=0;i--) {
                    int nid = i*3+3; int arraylength;
                    MontNode::isValueInteger(node->children[nid], &arraylength);
                    type = MontType(new MontType(type), arraylength);
                }
                if (DEBUG) std::cout << "globdecl array " << type << endl;
                int cnt = node->memorySize/4;
                pushBSS(name, type, node->memorySize);
            } else 
                return appendErrorInfo("Globdecl: Undefined globdecl syntax.", NRC);
            return true;
        }
        case NK_IF: {
            if (DEBUG) std::cout << "conceiving if " << CRC << endl; 
            int labelId = labelCounter ++;
            if (node->expansion == NE_IF_SIMPLE) { // If LParen expression RParen statement
                if (!visitChild(node, 2, false)) return false;
                Mnp cond = node->children[2];
                if (isVoid(cond))
                    return appendErrorInfo("If: Void condition.", NRC);
                if (isPointer(cond))
                    return appendErrorInfo("If: Pointer condition.", NRC);
                add(IRSTR(IR_BEQZ, getLabel("IF_END")));
                if (!visitChild(node, 4, false)) return false;
                add(IRSTR(IR_LABEL, getLabel("IF_END")));
                return true;
            } else if (node->expansion == NE_IF_ELSE) { // If LParen expression RParen statement Else statement
                if (!visitChild(node, 2, false)) return false;
                Mnp cond = node->children[2];
                if (isVoid(cond))
                    return appendErrorInfo("If: Void condition.", NRC);
                if (isPointer(cond))
                    return appendErrorInfo("If: Pointer condition.", NRC);
                add(IRSTR(IR_BEQZ, getLabel("IF_ELSE")));
                if (!visitChild(node, 4, false)) return false;
                add(IRSTR(IR_BR, getLabel("IF_END")));
                add(IRSTR(IR_LABEL, getLabel("IF_ELSE")));
                if (!visitChild(node, 6, false)) return false;
                add(IRSTR(IR_LABEL, getLabel("IF_END")));
                return true;
            } else 
                return appendErrorInfo("If: Undefined if syntax.", NRC);
        }
        case NK_LOGICAL_AND: {
            if (DEBUG) std::cout << "conceiving logical_and " << CRC << endl; 
            if (node->expansion == NE_LAND_LEAF) 
                DETERMINECHILD(0)
            else if (node->expansion == NE_LAND_INNER) {
                if (!visitChild(node, 0, false) || !visitChild(node, 2, false)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_LAND)
                    add(IRSIM(IR_LAND));
                else return appendErrorInfo("Logical and: Expect operator && token.", node->row, node->column);
                Mnp a = node->children[0], b = node->children[2];
                if (isVoid(a) || isVoid(b)) 
                    return appendErrorInfo("Logical and: Operation with void.", NRC);
                if (isPointer(a) || isPointer(b))
                    return appendErrorInfo("Additive: Pointer in operation.", NRC);
                else node->datatype = MontType(DT_BOOL);
                node->setLvalue(false);
                return true;
            }
            else
                return appendErrorInfo("Logical and: Undefined logical_and syntax.", node->row, node->column);
            break;
        }
        case NK_LOGICAL_OR: {
            if (DEBUG) std::cout << "conceiving logical_or " << CRC << endl; 
            if (node->expansion == NE_LOR_LEAF) 
                DETERMINECHILD(0)
            else if (node->expansion == NE_LOR_INNER) {
                if (!visitChild(node, 0, false) || !visitChild(node, 2, false)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_LOR)
                    add(IRSIM(IR_LOR));
                else return appendErrorInfo("Logical or: Expect operator || token.", node->row, node->column);
                Mnp a = node->children[0], b = node->children[2];
                if (isVoid(a) || isVoid(b)) 
                    return appendErrorInfo("Logical or: Operation with void.", NRC);
                if (isPointer(a) || isPointer(b))
                    return appendErrorInfo("Additive: Pointer in operation.", NRC);
                else node->datatype = MontType(DT_BOOL);
                node->setLvalue(false);
                return true;
            }
            else
                return appendErrorInfo("Logical or: Undefined logical_or syntax.", node->row, node->column);
            break;
        }
        case NK_MULTIPLICATIVE: {
            if (DEBUG) std::cout << "conceiving multiplicative " << CRC << endl; 
            if (node->expansion == NE_MULTIPLICATIVE_LEAF) 
                DETERMINECHILD(0)
            else if (node->expansion == NE_MULTIPLICATIVE_INNER) {
                if (!visitChild(node, 0, false) || !visitChild(node, 2, false)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_ASTERISK)
                    add(IRSIM(IR_MUL));
                else if (op.tokenKind == TK_LSLASH)
                    add(IRSIM(IR_DIV));
                else if (op.tokenKind == TK_PERCENT)
                    add(IRSIM(IR_REM));
                else return appendErrorInfo("Multiplicative: Expect operator token.", node->row, node->column);
                Mnp a = node->children[0], b = node->children[2];
                if (isVoid(a) || isVoid(b)) 
                    return appendErrorInfo("Multiplicative: Multiplicative with void.", NRC);
                if (isPointer(a) || isPointer(b))
                    return appendErrorInfo("Additive: Pointer in operation.", NRC);
                else node->datatype = MontType(DT_INT);
                node->setLvalue(false);
                return true;
            } 
            else 
                return appendErrorInfo("Multiplicative: Undefined multiplicative syntax.", node->row, node->column);
            break;
        }
        case NK_PARAMETERS: {
            if (DEBUG) std::cout << "conceiving parameters " << CRC << endl; 
            return appendErrorInfo("Parameters: Should not conceive parameters internly.",NRC);
        }
        case NK_POSTFIX: {
            if (DEBUG) std::cout << "conceiving postfix " << CRC << endl; 
            if (node->expansion == NE_POSTFIX_PRIMARY) 
                DETERMINECHILD(0)
            else if (node->expansion == NE_POSTFIX_CALL) { // Identifier LParen exprlist RParen
                string name = getTokenChild(node, 0).identifier;
                int functionId = getFunction(name);
                if (functionId==-1)
                    return appendErrorInfo("Postfix: Undeclared function: " + name + ".", NRC);
                Mnp exprlist = node->children[2]; // expr comma expr comma ...
                int exprcount = (exprlist->children.size()+1)/2;
                MontFunction& func = functions[functionId];
                if (exprcount != func.para.size()) 
                    return appendErrorInfo("Postfix: Function parameter count does not match.", NRC);
                for (int i=exprcount-1;i>=0;i--) {
                    if (!visitChild(exprlist, 2*i, false)) return false;
                    if (func.para[i] != exprlist->children[2*i]->datatype)
                        return appendErrorInfo("Postfix: Argument type does not match.",NRC);
                    if (!parseType(func.para[i], exprlist->children[2*i]->datatype))
                        return appendErrorInfo("Postfix: Failed to cast implicitly.", NRC);
                }
                if (func.ret == DT_VOID) 
                    add(IRCOM(IR_CALLV, exprcount, name));
                else 
                    add(IRCOM(IR_CALL, exprcount, name));
                setNodeType(node, func.ret); node->setLvalue(false);
                return true;
            } else if (node->expansion == NE_POSTFIX_ARRAY) { // postfix LB expr RB
                if (!visitChild(node, 0, true)) return false;
                //std::cout << node->children[0]->datatype;
                if (!visitChild(node, 2, false)) return false;
                if (!isBroadptr(node->children[0])) 
                    return appendErrorInfo("Postfix: Indexing on non-broadptr.", NRC);
                if (!isInt(node->children[2]))
                    return appendErrorInfo("Postfix: Index not integer.", NRC);
                if (isPointer(node->children[0])) 
                    add(IRINT(IR_PUSH, 4));
                else 
                    add(IRINT(IR_PUSH, node->children[0]->datatype.pointer->size));
                setNodeType(node, *(node->children[0]->datatype.pointer));
                add(IRSIM(IR_MUL));
                add(IRSIM(IR_ADD));
                if (!asLvalue) {
                    add(IRSIM(IR_LOAD));
                    node->setLvalue(false);
                } else node->setLvalue(true);
                return true;
            } else 
                return appendErrorInfo("Postfix: Undefined postfix syntax.", NRC);
        }
        case NK_PRIMARY: {
            if (DEBUG) std::cout << "conceiving primary " << CRC << endl; 
            if (node->expansion == NE_PRIMARY_VALUE) 
                DETERMINECHILD(0)
            else if (node->expansion == NE_PRIMARY_PAREN)
                DETERMINECHILD(1)
            else if (node->expansion == NE_PRIMARY_IDENTIFIER) {
                Token name = getTokenChild(node, 0);
                MontType type;
                flag = getVariable(name.identifier, &type); 
                if (!flag) return appendErrorInfo("Primary: Undefined identifier: " + name.identifier + ".", NRC);
                setNodeType(node, type);
                if (!asLvalue) {
                    add(IRSIM(IR_LOAD));
                    node->setLvalue(false);
                } else {
                    node->setLvalue(true);
                }
                //if (DEBUG) std::cout << "ok conceived primary" << endl;
                return true;
            } else 
                return appendErrorInfo("Primary: Undefined primary syntax.", node->row, node->column);
            break;
        }
        case NK_PROGRAM: {
            if (DEBUG) std::cout << "conceiving program " << CRC << endl; 
            int s = node->children.size();
            int mainId = -1;
            for (int i=0;i<s-1;i++) {
                Mnp fnode = node->children[i];
                if (fnode->kind != NK_FUNCTION) continue;
                Token identifier = getTokenChild(fnode, 1);
                if (identifier.identifier == "main") {mainId = i; break;}
            }
            if (mainId == -1) 
                return appendErrorInfo("Program: No main function found.", NRC);
            for (int i=0;i<s-1;i++) {
                if (!visitChild(node, i, false)) return false;
            }
            if (DEBUG) std::cout << "program conceived" << endl; 
            return true;
            break;
        }
        case NK_RELATIONAL: {
            if (DEBUG) std::cout << "conceiving relational " << CRC << endl; 
            if (node->expansion == NE_RELATIONAL_LEAF) 
                DETERMINECHILD(0)
            else if (node->expansion == NE_RELATIONAL_INNER) {
                if (!visitChild(node, 0, false) || !visitChild(node, 2, false)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_GREATER)
                    add(IRSIM(IR_GT));
                else if (op.tokenKind == TK_GREATER_EQUAL)
                    add(IRSIM(IR_GE));
                else if (op.tokenKind == TK_LESS) 
                    add(IRSIM(IR_LT));
                else if (op.tokenKind == TK_LESS_EQUAL)
                    add(IRSIM(IR_LE));
                else return appendErrorInfo("Relational: Expect operator >, >=, <, <= token.", node->row, node->column);
                Mnp a = node->children[0], b = node->children[2];
                if (isVoid(a) || isVoid(b)) 
                    return appendErrorInfo("Relational and: Relational with void.", NRC);
                if (isPointer(a) || isPointer(b))
                    return appendErrorInfo("Additive: Pointer in operation.", NRC);
                else node->datatype = MontType(DT_BOOL);
                node->setLvalue(false);
                return true;
            }
            else
                return appendErrorInfo("Relational: Undefined relational syntax.", node->row, node->column);
            break;
        }
        case NK_ROOT: {
            if (DEBUG) std::cout << "conceiving root " << CRC << endl; 
            return visitChild(node, 0, false);
            break;
        }
        case NK_STATEMENT: {
            if (DEBUG) std::cout << "conceiving statement " << CRC << endl; 
            switch (node->expansion) {
                case NE_STATEMENT_EXPRESSION: { // expression Semicolon
                    flag = visitChild(node, 0, false);
                    if (!flag) return false;
                    if (node->children[0]->datatype != DT_VOID)
                        add(IRSIM(IR_POP));
                    break;
                }
                case NE_STATEMENT_RETURN: { // Return expression? Semicolon
                    if (currentFunction == -1)
                        return appendErrorInfo("Statement: Return not in function scope.", NRC);
                    MontFunction& func = getCurrentFunction();
                    if (func.ret == DT_VOID && node->children.size()!=2) 
                        return appendErrorInfo("Statement: Returning value on void function.", NRC);
                    if (func.ret != DT_VOID && node->children.size()!=3) 
                        return appendErrorInfo("Statement: Returning void on value function.", NRC); 
                    if (func.ret != DT_VOID) { 
                        flag = visitChild(node, 1, false);
                        if (isVoid(node->children[1])) 
                            return appendErrorInfo("Statement: Returning void value.", NRC);
                        if (func.ret != node->children[1]->datatype)
                            return appendErrorInfo("Statement: Returning wrong type.", NRC);
                        if (!parseType(func.ret, node->children[1]->datatype))
                            return appendErrorInfo("Statement: Failed to cast implicitly the return value", NRC);
                        add(IRSIM(IR_RET));
                    } else {
                        add(IRSIM(IR_RETV));
                    }
                    return flag;
                    break;
                }
                case NE_STATEMENT_CODEBLOCK: {
                    pushFrame(false);
                    flag = visitChild(node, 0, false);
                    popFrame();
                    return flag;
                    break;
                }
                case NE_STATEMENT_IF: {
                    return visitChild(node, 0, false);
                    break;
                }
                case NE_STATEMENT_EMPTY: {
                    return true; 
                    break;
                }
                case NE_STATEMENT_FOR: {
                    return visitChild(node, 0, false);
                }
                case NE_STATEMENT_WHILE: {
                    return visitChild(node, 0, false);
                }
                case NE_STATEMENT_BREAK: {
                    int labelId = getCurrentLoop();
                    if (labelId == -1) 
                        return appendErrorInfo("Statement: Break not in a loop.", NRC);
                    add(IRSTR(IR_BR, getLabel("LOOP_BREAK")));
                    return true;
                }
                case NE_STATEMENT_CONTINUE: {
                    int labelId = getCurrentLoop();
                    if (labelId == -1) 
                        return appendErrorInfo("Statement: Continue not in a loop.", NRC);
                    add(IRSTR(IR_BR, getLabel("LOOP_CONTINUE")));
                    return true;
                }
                default: {
                    return appendErrorInfo("Statement: Undefined statement type.", node->row, node->column);
                }
            }
            break;
        }
        case NK_TOKEN: {
            if (DEBUG) std::cout << "conceiving token " << CRC << endl; 
            return true;
            break;
        }
        case NK_TYPE:{
            if (DEBUG) std::cout << "conceiving type " << CRC << endl; 
            return true;
            break;
        }
        case NK_UNARY: {
            if (DEBUG) std::cout << "conceiving unary " << CRC << endl; 
            switch (node->expansion) {
                case NE_UNARY_OPERATION: { // unary : (Exclamation|Minus|Tilde|Asterisk|And) unary
                    Token op = getTokenChild(node, 0);
                    if (op.tokenKind == TK_MINUS) {
                        if (!visitChild(node, 1, false)) return false;
                        if (isVoid(node->children[1])) return appendErrorInfo("Unary: Unary with void.", NRC);
                        if (isBroadptr(node->children[1])) return appendErrorInfo("Unary: Unary with broadptr.", NRC);
                        add(IRSIM(IR_NEG)); 
                        if (node->children[0]->datatype.isBool()) node->datatype = MontType(DT_INT);
                        else setNodeTypeFromChild(node, 1); 
                        node->setLvalue(false); return true;
                    } else if (op.tokenKind == TK_EXCLAMATION) {
                        if (!visitChild(node, 1, false)) return false;
                        if (isVoid(node->children[1])) return appendErrorInfo("Unary: Unary with void.", NRC);
                        if (isBroadptr(node->children[1])) return appendErrorInfo("Unary: Unary with broadptr.", NRC);
                        add(IRSIM(IR_LNOT));
                        node->datatype = MontType(DT_BOOL);
                        node->setLvalue(false); return true;
                    } else if (op.tokenKind == TK_TILDE) {
                        if (!visitChild(node, 1, false)) return false;
                        if (isVoid(node->children[1])) return appendErrorInfo("Unary: Unary with void.", NRC);
                        if (isBroadptr(node->children[1])) return appendErrorInfo("Unary: Unary with broadptr.", NRC);
                        add(IRSIM(IR_NOT));
                        if (node->children[0]->datatype.isBool()) node->datatype = MontType(DT_INT);
                        else setNodeTypeFromChild(node, 1);
                        node->setLvalue(false); return true;
                    } else if (op.tokenKind == TK_AND) {
                        if (!visitChild(node, 1, true)) return false;
                        if (isVoid(node->children[1])) return appendErrorInfo("Unary: Unary with void.", NRC);
                        if (isArray(node->children[1])) return appendErrorInfo("Unary: Getting address of array.", NRC);
                        if (!node->children[1]->isLvalue()) 
                            return appendErrorInfo("Unary: And symbol followed by rvalue.", NRC);
                        // 当visitchild之后必定在栈顶就是一个地址，因而无需做任何操作。
                        node->datatype = MontType(new MontType(node->children[1]->datatype));
                        node->setLvalue(false);
                        return true;
                    } else if (op.tokenKind == TK_ASTERISK) {
                        if (!visitChild(node, 1, false)) return false;
                        if (isVoid(node->children[1])) return appendErrorInfo("Unary: Unary with void.", NRC);
                        if (!isBroadptr(node->children[1]))
                            return appendErrorInfo("Unary: Asterisk symbol followed by non-pointer.", NRC);
                        if (!asLvalue) 
                            add(IRSIM(IR_LOAD));
                        node->datatype = *(node->children[1]->datatype.pointer);
                        node->setLvalue(true);
                        return true;
                    } else
                        return appendErrorInfo("Unary: No operator.", node->row, node->column); 
                    break;
                } 
                case NE_UNARY_POSTFIX: { // unary : value
                    DETERMINECHILD(0)
                    break;
                }
                case NE_UNARY_CAST: { // LP type RP unary
                    if (!visitChild(node, 3, false)) return false;
                    MontType type = getType(node->children[1]);
                    if (!parseType(type, node->children[3]->datatype)) return false;
                    node->datatype = type;
                    return true;
                    break;
                }
                default: {
                    return appendErrorInfo("Unary: Undefined unary syntax.", NRC);
                }
            }           
            break;
        }
        case NK_VALUE:{
            if (DEBUG) std::cout << "conceiving value " << CRC << endl; 
            Token valueToken = getTokenChild(node, 0);
            int value = valueToken.value;
            if (valueToken.tokenKind == TK_TRUE) value = 1;
            else if (valueToken.tokenKind == TK_FALSE) value = 0;
            setNodeType(node, getTypeFromValue(valueToken));
            node->setLvalue(false);
            add(IRINT(IR_PUSH, value));
            return true;
            break;
        }
        case NK_WHILE: { // While ( expr ) statement | Do statement While ( expr ) ; 
            if (DEBUG) std::cout << "conceiving while " << CRC << endl; 
            if (node->expansion != NE_WHILE_STANDARD && node->expansion != NE_WHILE_DO)
                return appendErrorInfo("While: Undefined while syntax.", NRC); 
            int labelId = labelCounter ++; pushLoop(labelId);
            pushFrame(false);
            bool doloop = node->expansion == NE_WHILE_DO;
            if (doloop)
                if (!visitChild(node, 1, false)) return false;
            add(IRSTR(IR_LABEL, getLabel("LOOP_BEGIN")));
            if (!visitChild(node, doloop ? 4 : 2, false)) return false;
            Mnp cond = node->children[doloop ? 4 : 2];
            if (isVoid(cond))
                return appendErrorInfo("While: Void condition.", NRC);
            if (isPointer(cond))
                return appendErrorInfo("While: Pointer condition.", NRC);
            add(IRSTR(IR_BEQZ, getLabel("LOOP_BREAK")));
            pushFrame(false);
            if (!visitChild(node, doloop ? 1 : 4, false)) return false;
            add(IRSTR(IR_LABEL, getLabel("LOOP_CONTINUE")));
            popFrame();
            add(IRSTR(IR_BR, getLabel("LOOP_BEGIN")));
            add(IRSTR(IR_LABEL, getLabel("LOOP_BREAK")));
            popFrame();
            break;
        }
        default:{
            return appendErrorInfo("Node: Undefined node type. Got " + to_string(node->kind), node->row, node->column);
        }
    }
    return true;

}

ostream& operator <<(ostream& out, MontConceiver& con){
    int s = con.irs.size();
    out << "Intermediate Code: " << endl;
    for (int i=0;i<s;i++) 
        out << con.irs[i] << std::endl;
    
    out << endl << "Data section: " << endl;
    s = con.data.size();
    for (int i=0;i<s;i++) 
        out << con.data[i].name << " : " << con.data[i].type << " = " << con.dataValues[i] << endl;

    out << endl << "BSS section: " << endl;
    s = con.bss.size();
    for (int i=0;i<s;i++) 
        out << con.bss[i].name << " : " << con.bss[i].type << endl;
    return out;
}

void MontConceiver::pushVariable(string name, MontType type, int memorySize){
    int size = frames.size();
    frames[size-1].push(name, variablePointer, type, memorySize);
    variablePointer+=memorySize/4;
}

void MontConceiver::pushFrame(bool blocking){
    if (blocking) variablePointer = 0;
    frames.push_back(MontStackFrame(blocking));
}

void MontConceiver::popFrame(){
    assert(frames.size() > 0);
    int s = frames.size();
    if (!frames[s-1].blocking) variablePointer-=frames[s-1].identifiers.size(); 
    frames.pop_back();
}

bool MontConceiver::getVariable(string name, MontType* type){
    int fs = frames.size();
    for (int i=fs-1;i>=0;i--) {
        MontStackFrame& f = frames[i];
        int s = f.identifiers.size();
        for (int j=s-1;j>=0;j--) 
            if (f.identifiers[j].name == name) {
                if (type!=nullptr) *type = f.identifiers[j].type;
                int loc = f.identifiers[j].location;
                add(IRINT(IR_FRAMEADDR, loc));
                return true;
            }
        if (f.blocking) break;
    }
    int s = data.size();
    for (int i=0;i<s;i++) 
        if (data[i].name == name) {
            if (type!=nullptr) *type = data[i].type;
            add(IRSTR(IR_GLOBADDR, name));
            return true;
        }
    s = bss.size();
    for (int i=0;i<s;i++) 
        if (bss[i].name == name) {
            if (type!=nullptr) *type = bss[i].type;
            add(IRSTR(IR_GLOBADDR, name));
            return true;
        }
    return false;
}

int MontConceiver::checkRedeclaration(string name){
    int fs = frames.size();
    MontStackFrame& f = frames[fs-1];
    int s = f.identifiers.size();
    for (int j=s-1;j>=0;j--) 
        if (f.identifiers[j].name == name) return f.identifiers[j].location;
    return -1;
}

MontType MontConceiver::getType(MontNodePtr node) {
    if (node->expansion == NE_TYPE_BASIC) {
        Token token = getTokenChild(node, 0);
        if (token.tokenKind == TK_INT) return MontType(DT_INT);
        else if (token.tokenKind == TK_BOOL) return MontType(DT_BOOL);
        else if (token.tokenKind == TK_CHAR) return MontType(DT_CHAR);
        else return MontType();
    } else if (node->expansion == NE_TYPE_POINTER) {
        return MontType(new MontType(getType(node->children[0])));
    } else 
        return MontType();
}

MontType MontConceiver::getTypeFromValue(Token token) {
    switch (token.tokenKind) {
        case TK_INT_VALUE: return DT_INT;
        case TK_CHAR_VALUE: return DT_CHAR;
        case TK_TRUE: case TK_FALSE: return DT_BOOL;
        default: return DT_VOID;
    }
    return DT_VOID;
}

int MontConceiver::getFunction(string name) {
    int s = functions.size();
    for (int i=0;i<s;i++) 
        if (functions[i].name == name) return i;
    return -1;
}

void MontConceiver::pushParameter(string name, MontType type, int index) {
    // 对于局部变量，其位置loc表示其在栈中位置为 fp - 12 - 4*loc
    // 对于参数，其栈中位置为 fp + 4*index，从而知道其loc应当为-3-index
    int size = frames.size();
    frames[size-1].push(name, -3-index, type, 4);
}

ostream& operator <<(ostream& out, MontStackFrame& frame) {
    int s = frame.identifiers.size();
    if (s==0) out << "No variables in frame." << endl;
    for (int i=0;i<s;i++) 
        out << frame.identifiers[i] << endl;
    return out;
}

ostream& operator <<(ostream& out, MontVariable& variable) {
    out << "[";
    if (variable.size!=4) out << variable.size <<": ";
    out << variable.type;
    out << " " << variable.name << " @ " << variable.location << "]";
    return out;
}

bool MontConceiver::parseType(MontType dest, MontType src) {
    if (DEBUG) std::cout << "parse type " << src << " to " << dest << endl;
    if (src.isVoid() || dest.isVoid()) return appendErrorInfo("Cast: Casting void.", -1, -1); // 禁止出现有void参与转换
    if (dest.isBool() && !src.isBool()) add(IRSIM(IR_BOOL)); // 将整数和字符转换为布尔
    return true;
}

bool MontConceiver::checkGlobal(string name, bool checkfunction){
    int s = data.size();
    for (int i=0;i<s;i++) if (data[i].name==name) return true;
    s = bss.size();
    for (int i=0;i<s;i++) if (bss[i].name==name) return true;
    if (checkfunction) {
        s = functions.size();
        for (int i=0;i<s;i++) if (functions[i].name==name) return true;
    }
    return false;
}

void MontConceiver::pushData(string name, MontType type, int value) {
    data.push_back(MontVariable(name, 0, type, 4));
    dataValues.push_back(value);
}

void MontConceiver::pushBSS(string name, MontType type, int memorySize) {
    bss.push_back(MontVariable(name, 0, type, memorySize));
}

int MontConceiver::getValue(Mnp ptr, MontType* type) {
    Token token = getTokenChild(ptr, 0);
    switch (token.tokenKind) {
        case TK_CHAR_VALUE: 
            if (type) *type = MontType(DT_CHAR);
            return token.value;
        case TK_INT_VALUE: 
            if (type) *type = MontType(DT_INT);
            return token.value;
        case TK_TRUE: 
            if (type) *type = MontType(DT_BOOL);
            return 1;
        case TK_FALSE: 
            if (type) *type = MontType(DT_BOOL);
            return 0;
    }
    return 0;
}

bool MontConceiver::visitChild(MontNodePtr node, int id, bool asLvalue){
    bool f = visit(node->children[id], asLvalue);
    if (DEBUG) 
        if (f) std::cout << "ok conceived " << node->children[id]->kind << "-" << node->children[id]->expansion << endl;
    return f;
}