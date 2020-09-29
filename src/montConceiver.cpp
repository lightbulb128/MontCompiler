#include "montConceiver.h"
#include "montParser.h"
#include "montLexer.h"

#define IRSIM(c) (MontIntermediate::simple(c))
#define IRINT(c, v) (MontIntermediate::intcode(c, v))
#define INSTR(c, s) (MontIntermediate::strcode(c, s))

MontLog MontConceiver::logger = MontLog();

using std::ostream;

string MontIntermediate::toString(){
    switch (code) {
        case IR_ADD: return "ADD";
        case IR_DIV: return "DIV";
        case IR_LNOT: return "LNOT";
        case IR_MUL: return "MUL";
        case IR_NEG: return "NEG";
        case IR_NOT: return "NOT";
        case IR_PUSH: return string("PUSH ") + to_string(num);
        case IR_REM: return "REM";
        case IR_RET: return "RET";
        case IR_SUB: return "SUB";
        default: return "IRERROR";
    }
}

string MontIntermediate::toAssembly(){
    switch (code) {
        case IR_ADD:
            return string("lw t1, 4(sp)\nlw t2, 0(sp)\nadd t1, t1, t2\naddi sp, sp, 4\nsw t1, 0(sp)");
        case IR_DIV:
            return string("lw t1, 4(sp)\nlw t2, 0(sp)\ndiv t1, t1, t2\naddi sp, sp, 4\nsw t1, 0(sp)");
        case IR_LNOT: // lw t1,0(sp); seqz t1,t1; sw t1,0(sp)
            return string("lw t1, 0(sp)\nseqz t1,t1\nsw t1,0(sp)\n");
        case IR_MUL:
            return string("lw t1, 4(sp)\nlw t2, 0(sp)\nmul t1, t1, t2\naddi sp, sp, 4\nsw t1, 0(sp)");
        case IR_NEG: // lw t1,0(sp); neg t1,t1; sw t1,0(sp)
            return string("lw t1, 0(sp)\nneg t1,t1\nsw t1,0(sp)\n");
        case IR_NOT: // lw t1,0(sp); not t1,t1; sw t1,0(sp)
            return string("lw t1, 0(sp)\nnot t1,t1\nsw t1,0(sp)\n");
        case IR_PUSH: // addi sp, sp, -4 ; li t1, X ; sw t1, 0(sp)
            return string("addi sp, sp, -4\nli t1, ") + to_string(num) + "\nsw t1, 0(sp)\n";
        case IR_REM:
            return string("lw t1, 4(sp)\nlw t2, 0(sp)\nrem t1, t1, t2\naddi sp, sp, 4\nsw t1, 0(sp)");
        case IR_RET:  // lw a0, 0(sp) ; addi sp, sp, 4 ; jr ra
            return string("lw a0, 0(sp)\naddi sp, sp, 4\njr ra\n");
        case IR_SUB:
            return string("lw t1, 4(sp)\nlw t2, 0(sp)\nsub t1, t1, t2\naddi sp, sp, 4\nsw t1, 0(sp)");
        default:
            return "nop";
    }
}

ostream& operator <<(ostream& out, MontIntermediate ir){
    out << ir.toString(); return out;
}

MontConceiver::MontConceiver() {
    irs = vector<MontIntermediate>();
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
        case NK_CODEBLOCK: { 
            return visitChildren(node);
            break;
        }
        case NK_EXPRESSION: {
            return visitChild(node, 0);
            break;
        }
        case NK_FUNCTION: {
            Token identifier = getTokenChild(node, 1);
            if (identifier.identifier!="main") 
                return appendErrorInfo("Function: Function name not 'main'.", node);
            return visitChild(node, 4);
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
            else 
                return appendErrorInfo("Primary: Undefined primary syntax.", node->row, node->column);
            break;
        }
        case NK_PROGRAM: {
            return visitChild(node, 0);
            break;
        }
        case NK_ROOT: {
            return visitChild(node, 0);
            break;
        }
        case NK_STATEMENT: {
            switch (node->expansion) {
                case NE_STATEMENT_EXPRESSION: { // expression Semicolon
                    return visitChild(node, 0);
                    break;
                }
                case NE_STATEMENT_RETURN: { // Return expression Semicolon
                    flag = visitChild(node, 1);
                    add(IRSIM(IR_RET));
                    return flag;
                    break;
                }
                case NE_STATEMENT_VARDEFINE: { 
                    // TODO var define
                    return true;
                    break;
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
