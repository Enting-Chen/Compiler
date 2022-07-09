# ifndef TRANSLATER_H
# define TRANSLATER_H

# include "LexicalAnalyser.h"
# include "SyntaxAnalyser.h"
# include <iostream>
# include <stack>
# include <vector>
# include <string>
using namespace std;

struct qt_node {
    string op, p1, p2, result;
};

class Translator : public Parser {
    private:
        int tmp_var_count;
        int var_addr;
        stack<string> sem;
        vector<symbol> symbol_table;
        void handle_action( string action, string & w );
        void print_qt_sequence_to_file( string out_file );
        void print_symbol_table_to_file( string out_file );

    public:
        vector<qt_node> qt_sequence;
        Translator( Parser &sa, string in_file, vector<symbol> &symbol_table );
        int ll1_translate( vector<pair<TOKEN_TYPE, string>> & token_sequence );
};

void Translator::print_qt_sequence_to_file( string out_file ){
    ofstream out_file_stream( out_file );
    for ( auto &qt_node : qt_sequence ) {
        out_file_stream << qt_node.op << " " << qt_node.p1 << " " << qt_node.p2 << " " << qt_node.result << endl;
    }
    out_file_stream.close();
}

void Translator::print_symbol_table_to_file( string out_file ){
    ofstream out_file_stream( out_file );
    for ( auto &symbol : symbol_table ) {
        out_file_stream << symbol.name << " "
                        << symbol_type_string[symbol.type] << " "
                        << symbol_cat_string[symbol.cat] << " "
                        << symbol.addr << endl;
    }
    out_file_stream.close();
}

void Translator::handle_action( string action, string & w ){
    // remove first and last character from action
    if ( action.substr(0, 3) == "GEQ" ){
        qt_node q;
        q.p2 = sem.top();
        sem.pop();
        q.p1 = sem.top();
        sem.pop();
        q.op = action[4];
        q.result = "t" + to_string(tmp_var_count++);
        sem.push(q.result);
        qt_sequence.push_back(q);
    }
    else if ( action == "ASSIGN" ){
        qt_node q;
        q.op = "=";
        q.p1 = sem.top();
        sem.pop();
        q.p2 = "_";
        q.result = sem.top();
        sem.pop();
        qt_sequence.push_back(q);
    }
    else if ( action == "PUSH" ){
        sem.push( w );
    }
    else if ( action == "SYMBOL-TABLE" ) {
        for ( auto & symbol : symbol_table ) {
            if ( symbol.name == w ){
                symbol.type = INT;
                symbol.cat = VAR;
                symbol.addr = var_addr;
                var_addr += 4;
            }
        }
    }
    else if ( action == "IF" ){
        qt_sequence.push_back( qt_node{ "if", sem.top(), "_", "_" } );
        sem.pop();
    }
    else if ( action == "ELSE" ){
        qt_sequence.push_back( qt_node{ "else", "_", "_", "_" } );
    }
    else if ( action == "IF-END" ){
        qt_sequence.push_back( qt_node{ "if-end", "_", "_", "_" } );
    }
    else if ( action == "WHILE" ){
        qt_sequence.push_back( qt_node{ "while", "_", "_", "_" } );
    }
    else if ( action  == "DO" ){
        qt_sequence.push_back( qt_node{ "do", sem.top(), "_", "_" } );
        sem.pop();
    }
    else if ( action == "WHILE-END"){
        qt_sequence.push_back( qt_node{ "while-end", "_", "_", "_" } );
    }
}

Translator::Translator( Parser & sa, string in_file, vector<symbol> & _symbol_table ) {
    // get_grammer
    tmp_var_count = 0;
    var_addr = 0;
    table = sa.table;
    non_terminals = sa.non_terminals;
    symbol_table = _symbol_table;
    get_grammar( in_file );
}

int Translator::ll1_translate( vector<pair<TOKEN_TYPE, string>> & token_sequence ){
    token_sequence.push_back( make_pair( TOKEN_TYPE::EOF_, "$" ) );
    stack<string> stack1;
    stack<string> id_stack;
    stack1.push("$");
    stack1.push("program");
    int w = 0;
    while( !stack1.empty()){
        string x = stack1.top();
        stack1.pop();
        if ( is_terminal(x) ){
            if ( x.front() == '\"' && x.back() == '\"' ){
                handle_action(x.substr( 1, x.size() - 2 ), token_sequence[w].second);
            }
            else if ( x == token_sequence[w].second || x == "ID" && token_sequence[w].first == TOKEN_TYPE::ID
                || x == "CONSTANT" && token_sequence[w].first == TOKEN_TYPE::CONSTANT ){
                w++;
            }
        }
        else{
            // check if make_pair(x, token_sequence[w]) is in table
            pair<string, string> key;
            key.first = x;
            if ( token_sequence[w].first == TOKEN_TYPE::ID ){
                key.second = "ID";
            }
            else if ( token_sequence[w].first == TOKEN_TYPE::CONSTANT ){
                key.second = "CONSTANT";
            }
            else {
                key.second = token_sequence[w].second;
            }
            int i = table[key];
            // push reverse production to stack1
            if ( productions_vector[i].second[0] == "\\epsilon" ){
                continue;
            }
            for ( int j = productions_vector[i].second.size() - 1; j >= 0; j-- ){
                string s = productions_vector[i].second[j];
                stack1.push(s);
            }
        }    
    }
    print_qt_sequence_to_file( "qt_sequence.txt" );
    print_symbol_table_to_file( "symbol_table.txt" );
    return EXIT_SUCCESS;    
}

# endif // TRANSLATER_H