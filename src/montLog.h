#ifndef MONTLOG_H
#define MONTLOG_H

#include <sstream>
#include <string>

using std::ostringstream;
using std::string;

class MontLog {
private:
    ostringstream os;
    int trylevel; 
public:
    MontLog(){os.clear(); trylevel=0;}
    void log(string str){
        if (trylevel==0)
            os<<str<<std::endl;
    }
    void trystart(){trylevel++;}
    void tryend(){trylevel--;}
    string get(){return os.str();}
    void clear(){os.clear();}
};

#endif