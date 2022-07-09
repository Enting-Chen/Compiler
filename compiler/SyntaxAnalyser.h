/*
SyntaxAnalyser.h

syntax defintition of C, according to grammar1.txt

program -> int main ( ) { declarations stmt-sequence }
declarations -> declaration ; declarations | \epsilon
declaration -> int identifiers | \epsilon
identifiers -> ID identifiers'
identifiers' -> , identifiers | \epsilon
stmt-sequence -> statement stmt-sequence | \epsilon
statement -> if-stmt | assign-stmt | while-stmt
if-stmt -> if ( or-exp ) { stmt-sequence } else-stmt
else-stmt -> else { stmt-sequence } | \epsilon
while-stmt -> while ( or-exp ) { stmt-sequence }
assign-stmt -> ID = or-exp ;
or-exp -> and-exp or'-exp
or'-exp -> || or-exp | \epsilon
and-exp -> comparison-exp and'-exp
and'-exp -> && and-exp | \epsilon
comparison-exp -> add-sub-exp comparison'-exp
comparison'-exp -> < comparison-exp | > comparison-exp | <= comparison-exp | >= comparison-exp | == comparison-exp | != comparison-exp | \epsilon
add-sub-exp -> mul-div-exp add-sub'-exp
add-sub'-exp -> + add-sub-exp | - add-sub-exp | \epsilon
mul-div-exp -> factor mul-div'-exp
mul-div'-exp -> * mul-div-exp | / mul-div-exp | \epsilon
factor -> CONSTANT | ID | ( or-exp ) | ! factor

*/

# ifndef SYNTAX_ANALYSER_H
# define SYNTAX_ANALYSER_H

#include<iostream>
#include<string>
#include<map>
#include<vector>
#include<fstream>
#include<algorithm>
#include<sstream>
#include<set>
#include<stack>
#include<boost/algorithm/string.hpp>
using namespace std;
using namespace boost;

class Parser{
    private:
        map<string, vector<vector<string>> > productions;
        map<string, set<string> > first_set;
        map<string, set<string> > follow_set;
        vector<set<string>> select_set;

        void get_non_terminals();
        void get_productions();
        void get_first_set_of_symbol( const string & symbol );
        void get_first_set();
        void get_follow_set();
        void get_select_set();
        void get_table();
        int ll1_parsing( vector<pair<TOKEN_TYPE, string>> & token_sequence );

    protected:
        vector<pair<string, vector<string>>> productions_vector;
        int get_grammar(string filename); 
        bool is_terminal(string s){
            return non_terminals.find(s) == non_terminals.end();
        }

    public:
        set<string> non_terminals;
        map<pair<string, string>, int> table;

        int parse(vector<pair<TOKEN_TYPE, string>> & token_sequence);
};

// get the grammar from file
int Parser::get_grammar( string file_name ){
    ifstream file( file_name );
    if( !file ){
        cout << "打开文件失败" << endl;
        return -1;
    }
    for( string s; getline( file, s ) && s != ""; ){
        vector<string> production;
        // split by '|' and '->'
        split( production, s, is_any_of(" ") );
        // store the result in map productions
        // copy( production.begin()+2, production.end(), back_inserter(productions[production[0]]) );
        productions[production[0]].push_back( vector<string>() );
        productions_vector.push_back( make_pair( production[0], vector<string>() ) );
        for ( int i = 2; i < production.size(); i++ ){
            if ( production[i] != "|" ){
                productions_vector.back().second.push_back( production[i] );
                productions[production[0]].back().push_back( production[i] );
            }
            else{
                productions_vector.push_back( make_pair( production[0], vector<string>() ) );
                productions[production[0]].push_back( vector<string>() );
            }
        }
        non_terminals.insert( production[0] );
    }
    return 0;  
}

void Parser::get_first_set_of_symbol( const string & symbol ){
    if ( first_set.find( symbol ) != first_set.end() || symbol == "\\epsilon" ){
        return;
    }
    // if symbol is terminal, add to first_set
    if ( is_terminal( symbol ) ){
        first_set[symbol].insert( symbol );
        return;
    }
    // if symbol is non-terminal, get first_set of its production
    for ( auto & production : productions[symbol] ){
        // cout << symbol << " : " << production[0] << endl;
        get_first_set_of_symbol( production[0] );
        // add to first_set
        copy( first_set[production[0]].begin(), first_set[production[0]].end(), inserter( first_set[symbol], first_set[symbol].end() ) );
    }
}

void Parser::get_first_set(){
    for ( auto & it : productions ){
        get_first_set_of_symbol( it.first );
        //cout << productions_ll1.begin()->first << endl;
    }
}

void Parser::get_follow_set(){
    follow_set["program"].insert("$");
    // get follow_set of each non-terminal
    while(true){
        map<string, set<string> > old_follow_set = follow_set;
        for ( auto & it : productions ){
            //cout << it.first << "--\n";
            for ( auto & production : it.second ){
                for ( int i = 1; i < production.size(); i++ ){
                    // if production[i-1] is non-terminal
                    if ( !is_terminal( production[i-1] ) ){
                        get_first_set_of_symbol( production[i] );
                        // add to follow_set
                        copy( first_set[production[i]].begin(), first_set[production[i]].end(), inserter( follow_set[production[i-1]], follow_set[production[i-1]].end() ) );
                        if ( is_terminal( production[i]) == false && productions[production[i]].back().back() == "\\epsilon" && follow_set.find( production[i] ) != follow_set.end()){
                            copy( follow_set[production[i]].begin(), follow_set[production[i]].end(), inserter( follow_set[production[i-1]], follow_set[production[i-1]].end() ) );
                        }
                    }
                }
                string symbol = production.back();
                if ( !is_terminal(symbol) ){
                    //if ( productions[symbol].back().back() == "\\epsilon" ){
                        // add to follow_set
                        // if symbol exists in follow_set, add to follow_set
                        if ( follow_set.find( it.first ) != follow_set.end() ){
                            copy( follow_set[it.first].begin(), follow_set[it.first].end(), inserter( follow_set[symbol], follow_set[symbol].end() ) );
                        }
                    //}
                } 
            }
        } 
        // if follow_set is unchanged, break
        if ( old_follow_set == follow_set ){
            break;
        }       
    }
}

void Parser::get_select_set(){
    for ( auto & production : productions_vector ){
        string first_token = production.second[0];
        if ( first_token == "\\epsilon" ){
            select_set.push_back(follow_set[production.first]);
            continue;
        }
        select_set.push_back(first_set[first_token]);
    }
}

void Parser::get_table(){
    for ( int i = 0; i < select_set.size(); i++ ){
        for ( auto & it : select_set[i] ){
            table[make_pair(productions_vector[i].first, it)] = i;
        }
    }
}

void print_error( vector<pair<TOKEN_TYPE, string>> & token_sequence, stack<string> & stack1, int w ){
    cout << "error" << endl;
    cout << "input: ";
    for ( auto & input : token_sequence ){
        cout << input.second << " ";
    }
    cout << endl;
    cout << "stack1: ";
    while ( !stack1.empty() ){
        cout << stack1.top() << " ";
        stack1.pop();
    }
    cout << "\nw: " << token_sequence[w].second;
    cout << endl;
    cout << endl;
}

int Parser::ll1_parsing( vector<pair<TOKEN_TYPE, string>> & token_sequence ){
    token_sequence.push_back( make_pair( TOKEN_TYPE::EOF_, "$" ) );
    stack<string> stack1;
    stack1.push("$");
    stack1.push("program");
    int w = 0;
    while( !stack1.empty()){
        while ( stack1.top().front() == '\"' && stack1.top().back() == '\"' ){
            stack1.pop();
        }
        string x = stack1.top();
        stack1.pop();
        if ( is_terminal(x) ){
            if ( x == token_sequence[w].second || x == "ID" && token_sequence[w].first == TOKEN_TYPE::ID
                || x == "CONSTANT" && token_sequence[w].first == TOKEN_TYPE::CONSTANT ){
                w++;
            }
            else {
                cout << "error: stack top is terminal, but is not equal to current symbol" << endl;
                cout << "x: " << x << endl;
                stack1.push( x );
                print_error( token_sequence, stack1, w );
                return EXIT_FAILURE;
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
            if ( table.find(key) == table.end() ){
                cout << "error: stack top is non-terminal, but cannot find table entry" << endl;
                stack1.push( x );
                print_error( token_sequence, stack1, w );
                return EXIT_FAILURE;
            }
            int i = table[key];
            // push reverse production to stack1
            if ( productions_vector[i].second[0] == "\\epsilon" ){
                continue;
            }
            for ( int j = productions_vector[i].second.size() - 1; j >= 0; j-- ){
                stack1.push(productions_vector[i].second[j]);
            }
        }    
    }
    return EXIT_SUCCESS;
}

int Parser::parse( vector<pair<TOKEN_TYPE, string>> & token_sequence ){
    get_grammar( "grammar1.txt" );
    get_first_set();
    get_follow_set();
    get_select_set();
    get_table();
    return ll1_parsing( token_sequence );
}

# endif // SYNTAX_ANALYSER_H