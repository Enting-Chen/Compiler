# ifndef CODE_GEN_H
# define CODE_GEN_H

# include <iostream>
# include <fstream>
# include <vector>
# include <map>
# include <stack>
# include "LexicalAnalyser.h"
# include "SyntaxAnalyser.h"
# include "translater.h"
using namespace std;

vector<string> operation_table;

map<string, string> qt_op_to_mips_op = {
    {"+", "add"},
    {"-", "sub"},
    {"*", "mul"},
    {"/", "div"},
    {"%", "rem"},
    {"==", "seq"},
    {"!=", "sne"},
    {"<", "slt"},
    {"<=", "sle"},
    {">", "sgt"},
    {">=", "sge"},
    {"&", "and"},
    {"|", "or"},
    {"!", "not"},
    {"=", "move"},
    {"+=", "add"},
    {"-=", "sub"},
    {"*=", "mul"},
    {"/=", "div"},
    {"%=", "rem"},
    {"&=", "and"},
    {"|=", "or"}
};

class ASM{
    private:
        int if_count;
        stack<string> if_stack;
        int while_count;
        stack<int> while_stack;
        vector<qt_node> qt_sequence;
        vector<symbol> symbol_table;
        string data_to_reg( string data, string reg, ofstream & out );
        bool gen_assign( qt_node & code, ofstream & out );
        bool gen_calculation( qt_node & code, ofstream & out );
    public:
        ASM( vector<symbol> & symbol_table, vector<qt_node> & qt_sequence);
        int code_gen();
};

ASM::ASM(vector<symbol> & _symbol_table , vector<qt_node> & _qt_sequence){
    symbol_table = _symbol_table;
    qt_sequence = _qt_sequence;
    if_count = 0;
    while_count = 0;
}

string ASM::data_to_reg( string data, string reg, ofstream & out ){
    if ( data[0] == 't' ){
        return "$" + data;
    }
    for ( auto & sym : symbol_table){
        if( sym.name == data ){
            out << "lw $" << reg << ", " << data << endl;
            return "$" + reg;
        } 
    }
    out << "li $" << reg << ", " << data << endl;
    return "$" + reg;
}


bool ASM::gen_assign( qt_node & code, ofstream & outfile ) {
    string r1 = data_to_reg( code.p1, "v0", outfile );
    outfile << "sw " << r1 << ", " << code.result << endl;
    return true;
}

bool ASM::gen_calculation( qt_node & code, ofstream & outfile ) {
    string r1 = data_to_reg( code.p1, "v0", outfile );
    string r2 = data_to_reg( code.p2, "v1", outfile ); 
    outfile << qt_op_to_mips_op[code.op] << " $" << code.result << ", " 
            << r1 << ", " << r2 << endl;
    return true;
}

// generate mips, using the intermediate code
int ASM::code_gen() {
    // generate mips
    // open file
    ofstream outfile( "mips.asm" );
    // write header
    outfile << ".data" << endl;
    for ( auto data : symbol_table ){
        outfile << data.name << ": .word 0" << endl;
    }
    outfile << "\n.text" << endl;
    for ( auto code : qt_sequence ) {
        if ( code.op == "=" ) {
            gen_assign( code, outfile );
        }
        else if ( code.op == "+" || code.op == "-" || code.op == "*" || code.op == "/" || code.op == "%" 
            || code.op == "==" || code.op == "!=" || code.op == "<" || code.op == "<=" || code.op == ">"
            || code.op == ">=" || code.op == "&" || code.op == "|" || code.op == "!" 
            || code.op == "+=" || code.op == "-=" || code.op == "*=" || code.op == "/=" || code.op == "%="
            || code.op == "&=" || code.op == "|=" ) {
            gen_calculation( code, outfile );
        }
        else if ( code.op == "if" ){
            string r1 = data_to_reg( code.p1, "v0", outfile );
            outfile << "beqz " << r1 << ", if_" << ++if_count << endl;
            if_stack.push( "if_" + to_string(if_count) );
        }
        else if ( code.op == "else" ){
            outfile << "j " << "else_" << ++if_count << endl;
            outfile << if_stack.top() << ":" << endl;
            if_stack.pop();
            if_stack.push( "else_" + to_string(if_count) );
        }
        else if ( code.op == "if-end" ){
            outfile << if_stack.top() << ":" << endl;
            if_stack.pop();
        }
        else if ( code.op == "while" ){
            while_stack.push( ++while_count );
            outfile << "while_" << while_stack.top() << ":" << endl;
        }
        else if ( code.op == "do" ){
            string r1 = data_to_reg( code.p1, "v0", outfile );
            outfile << "beqz " << r1 << ", while_end_" << while_stack.top() << endl;
        }
        else if ( code.op == "while-end" ){
            outfile << "j while_" << while_stack.top() << endl;
            outfile << "while_end_" << while_stack.top() << ":" << endl;
            while_stack.pop();
        }
    }
    outfile << "li $v0, 10" << endl;
    outfile << "syscall" << endl;
    return EXIT_SUCCESS;
}

# endif
