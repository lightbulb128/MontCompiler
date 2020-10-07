#ifndef MONTASSEMBLER_H
#define MONTASSEMBLER_H

#include <iostream>
using std::ostream;
using std::endl;

#include "montConceiver.h"
#include "montLog.h"

class MontAssembler {
private:
    static MontLog logger;
    static bool appendErrorInfo(string str) {logger.log(str); return false;}
    static void clearErrorInfo(){logger.clear();}
    ostream* stream;
public:
    static string getErrorInfo(){return logger.get();}
    MontAssembler();
    void setStream(ostream* stream) {this->stream = stream;}
    void write(const string& str, int indent); // indent means 4*spaces
    void write(const string& str){write(str, 1);}
    void write(){*stream << endl;}
    bool assemble(MontConceiver& conc);
};

#endif