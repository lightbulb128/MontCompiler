#include "montAssembler.h"

MontAssembler::MontAssembler(){}

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
    write("main:", 0);
    int s = conc.irs.size();
    for (int i=0;i<s;i++) 
        write(conc.irs[i].toAssembly());
    return true;
}