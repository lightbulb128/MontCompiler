#include "montConceiver.h"
#include "montParser.h"
#include "montLexer.h"
#include <assert.h>

#define IRSIM(c) (MontIntermediate::simple(c))
#define IRINT(c, v) (MontIntermediate::intcode(c, (v)))
#define IRSTR(c, s) (MontIntermediate::strcode(c, (s)))
#define NRC node->row, node->column
#define LW2 string("lw t1, 4(sp)\nlw t2, 0(sp)\n")
#define LW1 string("lw t1, 0(sp)\n")
#define SW1 string("sw t1, 0(sp)\n")
#define BSP string("addi sp, sp, 4\n")
#define getLabel(desc) ".L"+to_string(labelId)+"_"+(desc)

MontLog MontConceiver::logger = MontLog();

using std::ostream;

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
        case IR_STORE: return "STORE";
        case IR_SUB: return "SUB";
        default: return "IRERROR";
    }
}

inline string _L(string s){return s+"\n";}

string MontIntermediate::toAssembly(){
    switch (code) {
        case IR_ADD: return LW2 + _L("add t1, t1, t2") + BSP + SW1;
        case IR_BEQZ: return LW1 + BSP + _L("beqz t1, " + str);
        case IR_BNEZ: return LW1 + BSP + _L("bnez t1, " + str);
        case IR_BR: return _L("j " + str);
        case IR_BUILDFRAME: return 
                _L("sw ra, -4(sp)") + 
                _L("sw fp, -8(sp)") + 
                _L("ori fp, sp, 0") + 
                _L("addi sp, sp, -" + to_string(num + 8));
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
        case IR_STORE: return LW2 + _L("sw t1, 0(t2)") + BSP;
        case IR_SUB: return LW2 + _L("sub t1, t1, t2")+BSP+SW1;
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
}

bool MontConceiver::visitChildren(MontNodePtr ptr){
    for (auto i = ptr->children.begin(); i != ptr->children.end(); i++) 
        if (!visit(*i)) return false;
    return true;
}

bool MontConceiver::visit(MontNodePtr node) {

    bool flag = true;

    switch (node->kind) {
        // Please add according to alphabetic order
        case NK_ADDITIVE: {
            if (node->expansion == NE_ADDITIVE_LEAF) 
                return visitChild(node,0);
            else if (node->expansion == NE_ADDITIVE_INNER) {
                if (!visitChild(node, 0) || !visitChild(node, 2)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_PLUS)
                    add(IRSIM(IR_ADD));
                else if (op.tokenKind == TK_MINUS)
                    add(IRSIM(IR_SUB));
                else return appendErrorInfo("Additive: Expect operator token.", node->row, node->column);
                return true;
            }
            else
                return appendErrorInfo("Additive: Undefined additive syntax.", node->row, node->column);
            break;
        }
        case NK_ASSIGNMENT: { 
            if (node->expansion == NE_ASSIGNMENT_VALUE)
                return visitChild(node, 0);
            else if (node->expansion == NE_ASSIGNMENT_ASSIGN) { // Identifier Assign expression
                Token name = getTokenChild(node, 0);
                int id = getIdentifier(name.identifier);
                if (id==-1) 
                    return appendErrorInfo("Assignment: Undefined identifier: " + name.identifier + ".", NRC);
                flag = visitChild(node, 2);
                if (!flag) return false;
                add(IRINT(IR_FRAMEADDR, id));
                add(IRSIM(IR_STORE));
                return true;
            } else 
                return appendErrorInfo("Assignment: Undefined assignment syntax.", NRC);
        }
        case NK_BLOCKITEM: {
            if (node->expansion == NE_BLOCKITEM_DECLARATION) 
                return visitChild(node, 0);
            else if (node->expansion == NE_BLOCKITEM_STATEMENT) 
                return visitChild(node, 0);
            else 
                return appendErrorInfo("Blockitem: Undefined blockitem syntax.", NRC);
        }
        case NK_CODEBLOCK: { 
            return visitChildren(node);
            break;
        }
        case NK_CONDITIONAL: {
            if (node->expansion == NE_CONDITIONAL_LEAF) { // logical_or 
                return visitChild(node, 0);
            } else if (node->expansion == NE_CONDITIONAL_INNER) { // logical_or Question expression Colon conditional
                int labelId = labelCounter ++;
                if (!visitChild(node, 0)) return false;
                add(IRSTR(IR_BEQZ, getLabel("CONDITIONAL_FALSE")));
                if (!visitChild(node, 2)) return false;
                add(IRSTR(IR_BR, getLabel("CONDITIONAL_END")));
                add(IRSTR(IR_LABEL, getLabel("CONDITIONAL_FALSE")));
                if (!visitChild(node, 4)) return false;
                add(IRSTR(IR_LABEL, getLabel("CONDITIONAL_END")));
                return true;
            } else 
                return appendErrorInfo("Conditional: Undefined conditional syntax.", NRC);
        }
        case NK_DECLARATION: { // type Identifier [Assign Expression]
            Token name = getTokenChild(node, 1);
            int id = checkRedeclaration(name.identifier);
            if (id!=-1) 
                return appendErrorInfo("Declaration: Variable redeclared: " + name.identifier + ".", NRC);
            pushIdentifier(name.identifier, IT_VARIABLE);
            if (node->expansion == NE_DECLARATION_INIT) {
                flag = visitChild(node, 3);
                if (!flag) return false;
            } else if (node->expansion == NE_DECLARATION_SIMPLE) 
                add(IRINT(IR_PUSH, 0));
            else return appendErrorInfo("Declaration: Undefined declaration syntax.", NRC);
            id = getIdentifier(name.identifier);
            add(IRINT(IR_FRAMEADDR, id));
            add(IRSIM(IR_STORE));
            add(IRSIM(IR_POP));
            return true;
        }
        case NK_EQUALITY: {
            if (node->expansion == NE_EQUALITY_LEAF) 
                return visitChild(node,0);
            else if (node->expansion == NE_EQUALITY_INNER) {
                if (!visitChild(node, 0) || !visitChild(node, 2)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_EQUAL)
                    add(IRSIM(IR_EQ));
                else if (op.tokenKind == TK_NOT_EQUAL)
                    add(IRSIM(IR_NEQ));
                else return appendErrorInfo("Equality: Expect operator ==, != token.", node->row, node->column);
                return true;
            }
            else
                return appendErrorInfo("Equality: Undefined equality syntax.", node->row, node->column);
            break;
        }
        case NK_EXPRESSION: {
            return visitChild(node, 0);
            break;
        }
        case NK_FOR: { // For ( pre ; expr ; expr ) statement
            if (node->expansion != NE_FOR_DECLARATION && node->expansion != NE_FOR_EXPRESSION)
                return appendErrorInfo("For: Undefined for syntax.", NRC); 
            int labelId = labelCounter ++; pushLoop(labelId);
            pushFrame(false);
            if (!visitChild(node, 2)) return false;
            if (node->expansion == NE_FOR_EXPRESSION) 
                add(IRSIM(IR_POP));
            add(IRSTR(IR_LABEL, getLabel("LOOP_BEGIN")));
            if (!visitChild(node, 4)) return false;
            add(IRSTR(IR_BEQZ, getLabel("LOOP_BREAK")));
            pushFrame(false);
            if (!visitChild(node, 8)) return false;
            add(IRSTR(IR_LABEL, getLabel("LOOP_CONTINUE")));
            if (!visitChild(node, 6)) return false;
            popFrame();
            add(IRSTR(IR_BR, getLabel("LOOP_BEGIN")));
            add(IRSTR(IR_LABEL, getLabel("LOOP_BREAK")));
            popFrame();
            return true;
            break;
        }
        case NK_FUNCTION: {
            Token identifier = getTokenChild(node, 1);
            if (identifier.identifier!="main") 
                flag = appendErrorInfo("Function: Function name not 'main'.", node);
            else {
                int oldPointer = variablePointer;
                add(IRSTR(IR_LABEL, identifier.identifier));
                add(IRINT(IR_BUILDFRAME, node->memorySize)); 
                pushFrame(true);
                flag = visitChild(node, 4);
                popFrame();
                // 如若最后一条指令不是ret，则添加一个ret，默认返回值为0.
                if (irs[irs.size()-1].code != IR_RET) {
                    add(IRINT(IR_PUSH, 0));
                    add(IRSIM(IR_RET));
                }
                variablePointer = oldPointer;
            }
            return flag;
            break;
        }
        case NK_IF: {
            int labelId = labelCounter ++;
            if (node->expansion == NE_IF_SIMPLE) { // If LParen expression RParen statement
                if (!visitChild(node, 2)) return false;
                add(IRSTR(IR_BEQZ, getLabel("IF_END")));
                if (!visitChild(node, 4)) return false;
                add(IRSTR(IR_LABEL, getLabel("IF_END")));
                return true;
            } else if (node->expansion == NE_IF_ELSE) { // If LParen expression RParen statement Else statement
                if (!visitChild(node, 2)) return false;
                add(IRSTR(IR_BEQZ, getLabel("IF_ELSE")));
                if (!visitChild(node, 4)) return false;
                add(IRSTR(IR_BR, getLabel("IF_END")));
                add(IRSTR(IR_LABEL, getLabel("IF_ELSE")));
                if (!visitChild(node, 6)) return false;
                add(IRSTR(IR_LABEL, getLabel("IF_END")));
                return true;
            } else 
                return appendErrorInfo("If: Undefined if syntax.", NRC);
        }
        case NK_LOGICAL_AND: {
            if (node->expansion == NE_LAND_LEAF) 
                return visitChild(node,0);
            else if (node->expansion == NE_LAND_INNER) {
                if (!visitChild(node, 0) || !visitChild(node, 2)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_LAND)
                    add(IRSIM(IR_LAND));
                else return appendErrorInfo("Logical and: Expect operator && token.", node->row, node->column);
                return true;
            }
            else
                return appendErrorInfo("Logical and: Undefined logical_and syntax.", node->row, node->column);
            break;
        }
        case NK_LOGICAL_OR: {
            if (node->expansion == NE_LOR_LEAF) 
                return visitChild(node,0);
            else if (node->expansion == NE_LOR_INNER) {
                if (!visitChild(node, 0) || !visitChild(node, 2)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_LOR)
                    add(IRSIM(IR_LOR));
                else return appendErrorInfo("Logical or: Expect operator || token.", node->row, node->column);
                return true;
            }
            else
                return appendErrorInfo("Logical or: Undefined logical_or syntax.", node->row, node->column);
            break;
        }
        case NK_MULTIPLICATIVE: {
            if (node->expansion == NE_MULTIPLICATIVE_LEAF) 
                return visitChild(node, 0);
            else if (node->expansion == NE_MULTIPLICATIVE_INNER) {
                if (!visitChild(node, 0) || !visitChild(node, 2)) return false;
                Token op = getTokenChild(node, 1);
                if (op.tokenKind == TK_ASTERISK)
                    add(IRSIM(IR_MUL));
                else if (op.tokenKind == TK_LSLASH)
                    add(IRSIM(IR_DIV));
                else if (op.tokenKind == TK_PERCENT)
                    add(IRSIM(IR_REM));
                else return appendErrorInfo("Multiplicative: Expect operator token.", node->row, node->column);
                return true;
            } 
            else 
                return appendErrorInfo("Multiplicative: Undefined multiplicative syntax.", node->row, node->column);
            break;
        }
        case NK_PRIMARY: {
            if (node->expansion == NE_PRIMARY_VALUE) 
                return visitChild(node, 0);
            else if (node->expansion == NE_PRIMARY_PAREN)
                return visitChild(node, 1); 
            else if (node->expansion == NE_PRIMARY_IDENTIFIER) {
                Token name = getTokenChild(node, 0);
                int id = getIdentifier(name.identifier);
                if (id==-1) return appendErrorInfo("Primary: Undefined identifier: " + name.identifier + ".", NRC);
                add(IRINT(IR_FRAMEADDR, id));
                add(IRSIM(IR_LOAD));
            } else 
                return appendErrorInfo("Primary: Undefined primary syntax.", node->row, node->column);
            break;
        }
        case NK_PROGRAM: {
            return visitChild(node, 0);
            break;
        }
        case NK_RELATIONAL: {
            if (node->expansion == NE_RELATIONAL_LEAF) 
                return visitChild(node,0);
            else if (node->expansion == NE_RELATIONAL_INNER) {
                if (!visitChild(node, 0) || !visitChild(node, 2)) return false;
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
                return true;
            }
            else
                return appendErrorInfo("Relational: Undefined relational syntax.", node->row, node->column);
            break;
        }
        case NK_ROOT: {
            return visitChild(node, 0);
            break;
        }
        case NK_STATEMENT: {
            switch (node->expansion) {
                case NE_STATEMENT_EXPRESSION: { // expression Semicolon
                    flag = visitChild(node, 0);
                    if (!flag) return false;
                    add(IRSIM(IR_POP));
                    break;
                }
                case NE_STATEMENT_RETURN: { // Return expression Semicolon
                    flag = visitChild(node, 1);
                    add(IRSIM(IR_RET));
                    return flag;
                    break;
                }
                case NE_STATEMENT_CODEBLOCK: {
                    pushFrame(false);
                    flag = visitChild(node, 0);
                    popFrame();
                    return flag;
                    break;
                }
                case NE_STATEMENT_IF: {
                    return visitChild(node, 0);
                    break;
                }
                case NE_STATEMENT_EMPTY: {
                    return true; 
                    break;
                }
                case NE_STATEMENT_FOR: {
                    return visitChild(node, 0);
                }
                case NE_STATEMENT_WHILE: {
                    return visitChild(node, 0);
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
            return true;
            break;
        }
        case NK_TYPE:{
            return true;
            break;
        }
        case NK_UNARY: {
            switch (node->expansion) {
                case NE_UNARY_OPERATION: { // unary : (Exclamation|Minus|Tilde) unary
                    if (!visitChild(node, 1)) 
                        return false;
                    Token op = getTokenChild(node, 0);
                    if (op.tokenKind == TK_MINUS) 
                        add(IRSIM(IR_NEG));
                    else if (op.tokenKind == TK_EXCLAMATION)
                        add(IRSIM(IR_LNOT));
                    else if (op.tokenKind == TK_TILDE)
                        add(IRSIM(IR_NOT));
                    else
                        return appendErrorInfo("Unary: No operator.", node->row, node->column); 
                    break;
                } 
                case NE_UNARY_PRIMARY: { // unary : value
                    return visitChild(node, 0);
                    break;
                }
            }           
            break;
        }
        case NK_VALUE:{
            Token valueToken = getTokenChild(node, 0);
            int value = valueToken.value;
            add(IRINT(IR_PUSH, value));
            return true;
            break;
        }
        case NK_WHILE: { // While ( expr ) statement | Do statement While ( expr ) ; 
            if (node->expansion != NE_WHILE_STANDARD && node->expansion != NE_WHILE_DO)
                return appendErrorInfo("For: Undefined while syntax.", NRC); 
            int labelId = labelCounter ++; pushLoop(labelId);
            pushFrame(false);
            bool doloop = node->expansion == NE_WHILE_DO;
            if (doloop)
                if (!visitChild(node, 1)) return false;
            add(IRSTR(IR_LABEL, getLabel("LOOP_BEGIN")));
            if (!visitChild(node, doloop ? 4 : 2)) return false;
            add(IRSTR(IR_BEQZ, getLabel("LOOP_BREAK")));
            pushFrame(false);
            if (!visitChild(node, doloop ? 1 : 4)) return false;
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
    for (int i=0;i<s;i++) 
        out << con.irs[i] << std::endl;
    return out;
}

void MontConceiver::pushIdentifier(string name, IdentifierType type){
    int size = frames.size();
    frames[size-1].push(name, type, variablePointer);
    variablePointer++;
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

int MontConceiver::getIdentifier(string name){
    int fs = frames.size();
    for (int i=fs-1;i>=0;i--) {
        MontStackFrame& f = frames[i];
        int s = f.identifiers.size();
        for (int j=s-1;j>=0;j--) 
            if (f.identifiers[j].name == name) return f.identifiers[j].location;
        if (f.blocking) break;
    }
    return -1;
}

int MontConceiver::checkRedeclaration(string name){
    int fs = frames.size();
    MontStackFrame& f = frames[fs-1];
    int s = f.identifiers.size();
    for (int j=s-1;j>=0;j--) 
        if (f.identifiers[j].name == name) return f.identifiers[j].location;
    return -1;
}