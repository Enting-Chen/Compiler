#include <iostream>
#include "LexicalAnalyser.h"
#include "SyntaxAnalyser.h"
#include "translater.h"
#include "CodeGen.h"
using namespace std;

int main( int argc, char* argv[] ) {
    if ( argc < 2 ) {
        cout << "Usage: " << argv[0] << " <input_file> " << endl;
        return 0;
    }
    string in_file = argv[ 1 ];
    LexicalAnalyser la;
    int la_result = la.analyse( in_file );
    if ( la_result == EXIT_FAILURE ) {
        cout << "Lexical Analyser failed" << endl;
        return 1;
    }
    cout << "Lexical Analyser succeeded, outputs are in " << in_file 
        << ".la1 and " << in_file << ".la2\n" << endl;
    Parser sa;
    int sa_result = sa.parse( la.token_sequence_string );
    if ( sa_result == EXIT_FAILURE ) {
        cout << "Syntax Analyser failed" << endl;
        return 1;
    }
    cout << "Syntax Analyser succeeded\n" << endl;
    Translator translator( sa, "grammar2.txt", la.symbol_table );
    int translator_result = translator.ll1_translate( la.token_sequence_string );
    if ( translator_result == EXIT_FAILURE ) {
        cout << "Translator failed" << endl;
        return 1;
    }
    cout << "Translator succeeded, outputs are in qt_sequence.txt" << endl;
    ASM asm_generator( la.symbol_table, translator.qt_sequence );
    int asm_result = asm_generator.code_gen();
    if ( asm_result == EXIT_FAILURE ) {
        cout << "Code Generator failed" << endl;
        return 1;
    }
    cout << "\nCode Generator succeeded, MIPS codes are in mips.asm" << endl;
    return 0;
}