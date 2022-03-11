#include<iostream>
#include<iomanip>
#include<vector>
#include<string>
#include<cstring>
#include<fstream>
#include<cctype>
#include<algorithm>
#include<map>
using namespace std;

const vector<string> Codes = {"IDENTIFIER", "CHARACTER", "STRING", "CONSTANT"};
const vector<string> Keywords
    = {"auto", "break", "case", "char", "const", "continue", "default", "do", 
    "double", "else", "enum", "extern", "float", "for", "goto", "if", "int", 
    "long", "main", "register", "return", "short", "signed", "sizeof", "static", 
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"};
const vector<char> SingleChar= {'=', '+', '-', '*', '/', '%', '>', '<', '(', ')',  
                                '{', '}', ';', ',', '.', '&', '|', '^', '~', '!', '?', ':', '[', ']'};
const vector<string> DoubleChar = {"==", ">=", "<=", "+=", "-=", "/=", "*=", "++",
                                    "--", "&&", "||"};
map<string, int> CodeMap;
map<string, int> KeywordMap;
map<char, int> SingleCharMap;
map<string, int> DoubleCharMap;

void initialize_maps(){
    int count = 0;
    for (int i = 0; i < Codes.size(); i++, count++)
        CodeMap[Codes[i]] = count;
    for (int i = 0; i < Keywords.size(); i++, count++)
        KeywordMap[Keywords[i]] = count;
    for (int i = 0; i < SingleChar.size(); i++, count++)
        SingleCharMap[SingleChar[i]] = count;
    for (int i = 0; i < DoubleChar.size(); i++, count++)
        DoubleCharMap[DoubleChar[i]] = count;
}

int LexicalAnalyzer( string in_file, string out_file ){
    ifstream input_file(in_file);
    if (!input_file.is_open()) {
        cerr << "Could not open the inpupt file - '"
             << in_file << "'" << endl;
        return EXIT_FAILURE;
    }   

    ofstream output_file(out_file);
    if (!output_file.is_open()){
        cerr << "Could not open the output file - '"
             << out_file << "'" << endl;
        return EXIT_FAILURE;
    }
    char ch = input_file.get();
    vector<char> bytes;
    while ( input_file.peek() != EOF ){
        while ( isspace(ch) ){
            input_file.get(ch);
        }
        // cout << ch << endl;
        string token;
        if ( isalpha(ch) ){
            token.push_back(ch);
            input_file.get(ch);
            while( isalnum(ch) ) {
                token.push_back( ch );
                input_file.get(ch);
            }
            auto it = KeywordMap.find(token);
            if ( it == KeywordMap.end()){
                output_file << token << '\t' 
                << '<' << setw(2) << setfill('0') << CodeMap["IDENTIFIER"] << '>' << endl;
            }
            else{
                output_file << token << '\t'
                << '<' << setw(2) << setfill('0') 
                << KeywordMap[token] << '>' << endl;
            }
        }
        // CHARACTER
        else if ( ch == '\'' ){
            token.push_back(ch);
            // cout << "CHARACTER" << endl;
            input_file.get(ch);
            token.push_back(ch);
            if ( ch == '\\' ){
                input_file.get(ch);
                if ( ch == 'n' || ch == 't' || ch == '\\' || ch == '0'  ){
                    token.push_back(ch);
                    input_file.get(ch);
                    if ( ch == '\'' ){
                        token.push_back(ch);
                        input_file.get(ch);
                        output_file << token << '\t' << '<' << setw(2) << setfill('0') 
                        << CodeMap["CHARACTER"] << '>' << endl;
                    }
                    else{
                        cerr << "Invalid character - '" << ch << "'" << endl;
                        return EXIT_FAILURE;
                    }
                }
                else{
                    cerr << "Invalid character - '" << ch << "'" << endl;
                    return EXIT_FAILURE;
                }
            }
            else{
                input_file.get(ch);
                if ( ch == '\'' ){
                    token.push_back(ch);
                    input_file.get(ch);
                    output_file << token << '\t'
                    << '<' << setw(2) << setfill('0') << CodeMap["CHARACTER"] << '>' << endl;
                }
                else{
                    cerr << "Invalid character - '" << token << "'" << endl;
                    return EXIT_FAILURE;
                }
            }
        }
        else if ( ch == '"' ){
            // STRING
            // cout << "STRING" << endl;
            token.push_back(ch);
            input_file.get(ch);
            while ( ch != '"' ){
                token.push_back(ch);
                input_file.get(ch);
            }
            token.push_back(ch);
            input_file.get(ch);
            output_file << token << '\t' 
            << '<' << setw(2) << setfill('0') << CodeMap["STRING"] << '>' << endl;
        }
        else if ( isdigit( ch )){
            token.push_back(ch);
            input_file.get(ch);
            while ( isdigit(ch) ){
                token.push_back(ch);
                input_file.get(ch);
            }
            if ( ch == '.' ){
                token.push_back(ch);
                input_file.get(ch);
                while ( isdigit(ch) ){
                    token.push_back(ch);
                    input_file.get(ch);
                }
                output_file << token << '\t'
                << '<' << setw(2) << setfill('0') << CodeMap["CONSTANT"] << '>' << endl;
            }
            else{
                output_file << token << '\t'
                << '<' << setw(2) << setfill('0') << CodeMap["CONSTANT"] << '>' << endl;
            }
        }
        // else if ch in SingleChar
        else if ( find(SingleChar.begin(), SingleChar.end(), ch) != SingleChar.end() ){
            //cout << ch << endl;
            token.push_back(ch);
            input_file.get(ch);
            token.push_back(ch);
            // if ch in DoubleChar
            if ( DoubleCharMap.find(token) != DoubleCharMap.end() ){
                input_file.get(ch);
                output_file << token << '\t' << '<' << setw(2) << setfill('0')
                << DoubleCharMap[token] << '>' << endl;
            }
            else{
                token.pop_back();
                output_file << token << '\t' << '<' << setw(2) << setfill('0')
                << SingleCharMap[token[0]] << '>' << endl;
            }
        }
        // error 
        else{
            output_file << ch << '\t'
            << '<' << setw(2) << setfill('0') << "ERROR" << '>' << endl;
            input_file.get(ch);
            cerr << "Invalid character - '" << ch << "'" << endl;
            return EXIT_FAILURE;
        }
    }
    input_file.close();
    output_file.close();
    return EXIT_SUCCESS;
}

int main( int argc, char *argv[] ){
    if ( argc != 3 ){
        cerr << "Usage: " << argv[0] << " <input file> <output file>" << endl;
        return EXIT_FAILURE;
    }
    initialize_maps();
    return LexicalAnalyzer( argv[1], argv[2] );
}