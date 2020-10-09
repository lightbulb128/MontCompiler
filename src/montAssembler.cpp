#include "montAssembler.h"

MontAssembler::MontAssembler(){}

const bool allowComments = false;
const bool assemblyComments = false;

void MontAssembler::write(const string& str, int indent) {
    ostream& os = *stream; bool newline = true;
    for (char c : str) {
        if (c=='\n') {os << endl; newline=true;}
        else {
            if (newline) for (int i=0;i<indent;i++) os << "    ";
            newline = false; os << c; 
        }
    }
    if (!newline) os << endl;
}

bool MontAssembler::assemble(MontConceiver& conc){

    write(".text");
    write(".globl main");
    int s = conc.irs.size();
    for (int i=0;i<s;i++) {
        if (conc.irs[i].code == IR_COMMENT && !allowComments) continue; 
        if (assemblyComments) {
            IntermediateType code = conc.irs[i].code;
            if (code!=IR_COMMENT && code!=IR_LABEL)
                write("# " + conc.irs[i].toString());
        }
        write(conc.irs[i].toAssembly(), conc.irs[i].code == IR_LABEL ? 0 : 1);
    }
    
    s = conc.data.size();
    if (s>0) {
        write();
        write(".data");
        write(".align 4");
        for (int i=0;i<s;i++) {
            write(".globl " + conc.data[i].name);
            write(".size " + conc.data[i].name + ", 4");
            write(conc.data[i].name + ":", 0);
            write(".word " + to_string(conc.dataValues[i]));
        }
    }

    s = conc.bss.size();
    if (s>0) {
        write();
        write(".bss");
        for (int i=0;i<s;i++) 
            write(".comm " + conc.bss[i].name + ", " + to_string(conc.bss[i].size) + ", 4");
    }
    return true;

}