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
        case IR_PUSH:
            return string("PUSH ") + to_string(num);
        case IR_RET:
            return "RET";
        default:
            return "IRERROR";
    }
}

string MontIntermediate::toAssembly(){
    switch (code) {
        case IR_LNOT: // lw t1,0(sp); seqz t1,t1; sw t1,0(sp)
            return string("lw t1, 0(sp)\nseqz t1,t1\nsw t1,0(sp)\n");
        case IR_NEG: // lw t1,0(sp); neg t1,t1; sw t1,0(sp)
            return string("lw t1, 0(sp)\nneg t1,t1\nsw t1,0(sp)\n");
        case IR_NOT: // lw t1,0(sp); not t1,t1; sw t1,0(sp)
            return string("lw t1, 0(sp)\nnot t1,t1\nsw t1,0(sp)\n");
        case IR_PUSH: // addi sp, sp, -4 ; li t1, X ; sw t1, 0(sp)
            return string("addi sp, sp, -4\nli t1, ") + to_string(num) + "\nsw t1, 0(sp)\n";
        case IR_RET:  // lw a0, 0(sp) ; addi sp, sp, 4 ; jr ra
            return string("lw a0, 0(sp)\naddi sp, sp, 4\njr ra\n");
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
                case NE_UNARY_VALUE: { // unary : value
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
