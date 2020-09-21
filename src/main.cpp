#include <iostream>

#include "MiniDecafLexer.h"
#include "MiniDecafParser.h"

using namespace antlr4;
using namespace std;

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "[error] Input sourcefile required: ./MiniDecaf <sourcefile>\n";
        return 1;
    }
    ifstream sourceFile;
    sourceFile.open(argv[1]);

    // lexer & parser part of antlr4
    ANTLRInputStream input(sourceFile);
    MiniDecafLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    MiniDecafParser parser(&tokens);

    // customized pass: allocator, typer, codegen and etc.

    MiniDecafParser::ProgContext* treeNode = parser.prog();
    return 0;
}

