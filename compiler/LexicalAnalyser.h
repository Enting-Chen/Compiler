/* 
Lexical Analyser

1. ID/KEYWORD -> letter (digit|letter)*
2. NUMBER -> digit digit*
3. STRING: any character except "
4. blank includes '\n' '\f' '\v' ('\r' if read in binary)
5. comment -> /* any character */

# ifndef LEXICAL_ANALYSIS_H
# define LEXICAL_ANALYSIS_H

# include<iostream>
# include<iomanip>
# include<map>
# include<vector>
# include<string>
# include<fstream>
# include<set>
# include<algorithm>
using namespace std;

enum TOKEN_TYPE{
    KEYWORD,
    DELIMITER,
    ID,
    CONSTANT,
    EOF_
};

enum SYMBOL_TYPE{
    NULL_TYPE, // uninitialized
    INT
};

enum SYMBOL_CAT{
    NULL_CAT, // uninitialized
    VAR
};

map<TOKEN_TYPE, string> token_type_string = {
    {KEYWORD, "KEYWORD"},
    {DELIMITER, "DELIMITER"},
    {ID, "ID"},
    {CONSTANT, "CONSTANT"},
    {EOF_, "EOF_"}
};

map<SYMBOL_TYPE, string> symbol_type_string = {
    {NULL_TYPE, "NULL_TYPE"},
    {INT, "INT"}
};

map<SYMBOL_CAT, string> symbol_cat_string = {
    {NULL_CAT, "NULL_CAT"},
    {VAR, "VAR"}
};

const set<string> Keywords
    = {"else", "if", "int", "main", "while"};
const set<string> Delimiters
    = {"(", ")", ";", ",", "=", "+", "-", "*", "/", "{", "}",
    "||", "&&", "!", "<", ">", "<", ">"};

struct symbol{
    string name;
    SYMBOL_TYPE type;
    SYMBOL_CAT cat;
    int addr;
};

// lexical analyser
class LexicalAnalyser {
    private:
        // symbol table
        vector<string> keyword_table;
        vector<string> delimiter_table;
        vector<int> constant_table;
        // TOKEN sequence
        vector<pair<TOKEN_TYPE, int>> token_sequence;

    public:
        vector<symbol> symbol_table;
        vector<pair<TOKEN_TYPE, string>> token_sequence_string;
        int analyse(string in_file );
        int print_results( string out_file1, string out_file2 );      
};

int LexicalAnalyser::analyse(string in_file) {
    ifstream input_file(in_file);
    if (!input_file.is_open()) {
        cerr << "Could not open the inpupt file - '"
             << in_file << "'" << endl;
        return EXIT_FAILURE;
    }   
    char ch = input_file.get();
    while ( input_file.peek() != EOF ){
        while ( isspace(ch) ){
            input_file.get(ch);
        }
        string token;
        if ( isalpha(ch) ){
            token.push_back(ch);
            input_file.get(ch);
            while( isalnum(ch) ) {
                token.push_back( ch );
                input_file.get(ch);
            }
            // if token is in set<string> Keywords, store in keyword_table
            if ( Keywords.find(token) != Keywords.end() ){
                auto it = std::find(keyword_table.begin(), keyword_table.end(), token);  
                int i = std::distance(keyword_table.begin(), it);
                if ( it == keyword_table.end() ){
                    keyword_table.push_back(token);
                }
                token_sequence.push_back(make_pair(KEYWORD, i+1));
                token_sequence_string.push_back(make_pair(KEYWORD, token));
            }
            else{
                // check if token is in symbol_table
                int i;
                for ( i = 0; i < symbol_table.size(); i++ ){
                    if ( symbol_table[i].name == token ){
                        break;
                    }
                }
                if ( i == symbol_table.size() ){
                    symbol_table.push_back(symbol{token, NULL_TYPE, NULL_CAT, -1});
                }
                token_sequence.push_back(make_pair(ID, i+1));
                token_sequence_string.push_back(make_pair(ID, token));
            }
        }
        else if ( isdigit( ch )){
            token.push_back(ch);
            input_file.get(ch);
            while ( isdigit(ch) ){
                token.push_back(ch);
                input_file.get(ch);
            }
            // store in constant_table
            auto it = std::find(constant_table.begin(), constant_table.end(), stoi(token));
            int i = std::distance(constant_table.begin(), it);
            if ( it == constant_table.end() ){
                constant_table.push_back(stoi(token));
            }
            token_sequence.push_back(make_pair(CONSTANT, i+1));
            token_sequence_string.push_back(make_pair(CONSTANT, token));
        }
        // if ch is in set<string> Delimiters, store in delimiter_table
        else if ( ch == '(' || ch == ')' || ch == ';' || ch == ',' || ch == '=' || ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '{' || ch == '}' || ch == '|' || ch == '&' || ch == '!' || ch == '=' || ch == '<' || ch == '>' || ch == '<' || ch == '>' || ch == '"' ){
            while ( !isalnum(ch) && !isspace(ch) && find(Delimiters.begin(), Delimiters.end(), token) == Delimiters.end() ){
                token.push_back(ch);
                input_file.get(ch);
            }
            auto it = std::find(delimiter_table.begin(), delimiter_table.end(), token);
            int i = std::distance(delimiter_table.begin(), it);
            if ( it == delimiter_table.end() ){
                delimiter_table.push_back(token);
            }
            token_sequence.push_back(make_pair(DELIMITER, i+1));
            token_sequence_string.push_back(make_pair(DELIMITER, token));
        }
        // error 
        else{
            cerr << ch << '\t'
            << '<' << setw(2) << setfill('0') << "ERROR" << '>' << endl;
            input_file.get(ch);
            cerr << "Invalid character - '" << ch << "'" << endl;
            return EXIT_FAILURE;
        }
    }
    input_file.close();
    return print_results(in_file + ".la1", in_file + ".la2");
}

int LexicalAnalyser::print_results(string out_file1, string out_file2) {
    ofstream output_file(out_file1);
    if (!output_file.is_open()) {
        cerr << "Could not open the output file - '"
             << out_file1 << "'" << endl;
        return EXIT_FAILURE;
    }
    // print keyword_table
    output_file << "Keyword Table:" << endl;
    for (int i = 0; i < keyword_table.size(); i++) {
        output_file << setw(4) << i+1 << ": " << keyword_table[i] << endl;
    }
    // print delimiter_table
    output_file << "\nDelimiter Table:" << endl;
    for (int i = 0; i < delimiter_table.size(); i++) {
        output_file << setw(4) << i+1 << ": " << delimiter_table[i] << endl;
    }
    // print symbol_table
    output_file << "\nSymbol Table:" << endl;
    for (int i = 0; i < symbol_table.size(); i++) {
        output_file << setw(4) << i+1 << ": " << symbol_table[i].name << endl;
    }
    // print constant_table
    output_file << "\nConstant Table:" << endl;
    for (int i = 0; i < constant_table.size(); i++) {
        output_file << setw(4) << i+1 << ": " << constant_table[i] << endl;
    }
    ofstream output_file2(out_file2);
    // print token_sequence
    output_file2 << "Token Sequence:" << endl;
    for (int i = 0; i < token_sequence.size(); i++) {
        switch (token_sequence[i].first) {
            case KEYWORD:
                output_file2 << "(k, " << token_sequence[i].second << ")";
                break;
            case DELIMITER:
                output_file2 << "(p, " << token_sequence[i].second << ")";
                break;
            case ID:
                output_file2 << "(i, " << token_sequence[i].second << ")";
                break;
            case CONSTANT:
                output_file2 << "(c, " << token_sequence[i].second << ")";
                break;
            default:
                output_file2 << "UNKNOWN";
                break;
        }
        output_file2 << endl;
    }
    return EXIT_SUCCESS;
}

# endif