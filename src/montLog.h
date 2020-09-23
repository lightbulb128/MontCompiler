#ifndef MONTLOG_H
#define MONTLOG_H

#include <sstream>
#include <string>

using std::ostringstream;
using std::string;

class MontLog {
private:
    ostringstream os;
public:
    MontLog(){os.clear();}
    void log(string str){
        os<<str<<std::endl;
    }
    string get(){return os.str();}
    void clear(){os.clear();}
};

#endif