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

map<string, bool> is_terminal;
map<string, vector<string> > productions;
map<string, vector<vector<string>> > productions_ll1;
vector<pair<string, vector<string>>> productions_ll1_vector;
map<string, set<string> > first_set;
map<string, set<string> > follow_set;
vector<set<string>> select_set;
map<pair<string, string>, int> table;

// get the grammar from file
int get_grammar( string file_name ){
    ifstream file( file_name );
    if( !file ){
        cout << "打开文件失败" << endl;
        return -1;
    }
    string line;
    // get the terminal symbols
    getline( file, line );
    stringstream ss( line );
    for ( string s; ss >> s; ){
        is_terminal[s] = true;
    }
    // read in productions
    for( string s; getline( file, s ) && s != ""; ){
        vector<string> production;
        // split by '|' and '->'
        split( production, s, is_any_of("|->") );
        // store the result in map productions
        copy( production.begin()+2, production.end(), back_inserter(productions[production[0]]) );
        is_terminal[production[0]] = false;
    }
    return 0;  
}

// remove left recursion, convert the grammar to LL(1) grammar and 
void convert_to_ll1(){
    int n = productions.size();
    for ( auto & it : productions ){
        stringstream ss(it.second[1]);
        vector<string> production = {istream_iterator<string>(ss), istream_iterator<string>()};
        if ( production[0] == it.first ){
            // production pop front
            production.erase( production.begin() );
            production.push_back( it.first + '1' );
            // production to string, push_back to productions
            productions_ll1[it.first + '1'].push_back( production );
            productions_ll1[it.first + '1'].push_back( vector<string>{"@"} );
            productions_ll1[it.first].push_back(vector<string>{productions[it.first][0], it.first + "1"}); 
        }
        else{
            // copy to productions_ll1
            for ( auto & itt : it.second ){
                stringstream ss(itt);
                vector<string> production_ll1 = {istream_iterator<string>(ss), istream_iterator<string>()};
                productions_ll1[it.first].push_back( production_ll1 );
            }
        }
    }
    // convert to vector
    for ( auto & it : productions_ll1 ){
        for ( auto & it1 : it.second ){
            productions_ll1_vector.push_back( make_pair(it.first, it1) );
        }
    }
    // print productions
    for (auto & production : productions_ll1){
        cout << production.first << " -> ";
        for (auto & p : production.second){
            cout << p[0];
            for (int i = 1; i < p.size(); i++){
                cout << " " << p[i];
            }
            cout << " | ";
        }
        cout << endl;
    }
    //cout << productions_ll1.begin()->first << endl;
}

void get_first_set_of_symbol( const string & symbol ){
    if ( first_set.find( symbol ) != first_set.end() || symbol == "@" ){
        return;
    }
    // if symbol is terminal, add to first_set
    if ( is_terminal[symbol] ){
        first_set[symbol].insert( symbol );
        return;
    }
    // if symbol is non-terminal, get first_set of its production
    for ( auto & production : productions_ll1[symbol] ){
        // cout << symbol << " : " << production[0] << endl;
        get_first_set_of_symbol( production[0] );
        // add to first_set
        copy( first_set[production[0]].begin(), first_set[production[0]].end(), inserter( first_set[symbol], first_set[symbol].end() ) );
    }
}

void get_first_set(){
    for ( auto & it : productions_ll1 ){
        get_first_set_of_symbol( it.first );
        //cout << productions_ll1.begin()->first << endl;
    }
    // print first_set
    cout << endl;
    for ( auto & it : first_set ){
        cout << it.first << " -> ";
        for ( auto & s : it.second ){
            cout << s << " ";
        }
        cout << endl;
    }
    cout << endl;
    cout << productions_ll1.begin()->first << endl;
}

void get_follow_set(){
    follow_set["E"].insert("#");
    // get follow_set of each non-terminal
    while(true){
        map<string, set<string> > old_follow_set = follow_set;
        for ( auto & it : productions_ll1 ){
            for ( auto & production : it.second ){
                for ( int i = 1; i < production.size(); i++ ){
                    // if production[i-1] is non-terminal
                    if ( is_terminal[production[i-1]] == false ){
                        get_first_set_of_symbol( production[i] );
                        // add to follow_set
                        copy( first_set[production[i]].begin(), first_set[production[i]].end(), inserter( follow_set[production[i-1]], follow_set[production[i-1]].end() ) );
                        if ( is_terminal[production[i]] == false && productions_ll1[production[i]].back().back() == "@" && follow_set.find( production[i] ) != follow_set.end()){
                            copy( follow_set[production[i]].begin(), follow_set[production[i]].end(), inserter( follow_set[production[i-1]], follow_set[production[i-1]].end() ) );
                        }
                    }
                }
                string symbol = production.back();
                if ( symbol != "@" && !is_terminal[symbol] ){
                    if ( productions_ll1[symbol].back().back() == "@" ){
                        // add to follow_set
                        // if symbol exists in follow_set, add to follow_set
                        if ( follow_set.find( it.first ) != follow_set.end() ){
                            copy( follow_set[it.first].begin(), follow_set[it.first].end(), inserter( follow_set[symbol], follow_set[symbol].end() ) );
                        }
                    }
                } 
            }
        } 
        // if follow_set is unchanged, break
        if ( old_follow_set == follow_set ){
            break;
        }       
    }
    for ( auto & it : follow_set ){
        cout << it.first << " -> ";
        for ( auto & s : it.second ){
            cout << s << " ";
        }
        cout << endl;
    }
}

void get_select_set(){
    for ( auto & production : productions_ll1_vector ){
        string first_token = production.second[0];
        if ( first_token == "@" ){
            select_set.push_back(follow_set[production.first]);
            continue;
        }
        select_set.push_back(first_set[first_token]);
    }
    // print select_set
    cout << endl;
    int i = 0;
    for ( auto & it : select_set ){
        // print production
        cout << productions_ll1_vector[i].first << " -> ";
        for ( auto & s : productions_ll1_vector[i].second ){
            cout << s << " ";
        }
        cout << endl;
        // print select_set
        cout << "select_set: ";
        for ( auto & s : it ){
            cout << s << " ";
        }
        cout << endl;
        i++;
    }
}

void get_table(){
    for ( int i = 0; i < select_set.size(); i++ ){
        for ( auto & it : select_set[i] ){
            table[make_pair(productions_ll1_vector[i].first, it)] = i;
        }
    }
    // print table
    cout << endl;
    int i = 0;
    for ( auto & it : table ){
        cout << it.first.first << "," << it.first.second << ": " << it.second << "\t";
        i++;
        // 5 in a row
        if ( i % 5 == 0 ){
            cout << endl;
        }
    }
}

void print_error( vector<string> & input_vector, stack<string> & stack, int w ){
    cout << "error" << endl;
    cout << "input: ";
    for ( auto & input : input_vector ){
        cout << input << " ";
    }
    cout << endl;
    cout << "stack: ";
    while ( !stack.empty() ){
        cout << stack.top() << " ";
        stack.pop();
    }
    cout << "\nw: " << input_vector[w];
    cout << endl;
    cout << endl;
}

bool ll1_parsing(){
    string input;
    cout << "please input your expression: ";
    getline(cin, input);
    vector<string> input_vector;
    for ( int i = 0; i < input.size(); i++ ){
        if ( input[i] == '-' || input[i] == '+' ){
            input_vector.push_back("w0");
        } 
        else if ( input[i] == '*' || input[i] == '/' ){
            input_vector.push_back("w1");
        }
        else if ( input[i] == '(' || input[i] == ')' ){
            string s = "";
            input_vector.push_back(s + input[i]);
        }
        else if ( isalnum(input[i]) || input[i] == '.' ){
            input_vector.push_back("i");
            while ( i < input.size() && ( isalnum(input[i]) || input[i] == '.') ){
                i++;
            }
            i--;
        }
    }
    input_vector.push_back("#");
    stack<string> stack;
    stack.push("#");
    stack.push("E");
    int w = 0;
    while( !stack.empty()){
        string x = stack.top();
        stack.pop();
        if ( is_terminal[x] ){
            if ( x != input_vector[w] ){
                cout << "error: stack top is terminal, but is not equal to current symbol" << endl;
                cout << "x: " << x << endl;
                stack.push( x );
                print_error( input_vector, stack, w );
                return false;
            }
            w++;
        }
        else{
            // check if make_pair(x, input_vector[w]) is in table
            if ( table.find(make_pair(x, input_vector[w])) == table.end() ){
                cout << "error: stack top is non-terminal, but cannot find table entry" << endl;
                stack.push( x );
                print_error( input_vector, stack, w );
                return false;
            }
            int i = table[make_pair(x, input_vector[w])];
            // push reverse production to stack
            if ( productions_ll1_vector[i].second[0] == "@" ){
                continue;
            }
            for ( int j = productions_ll1_vector[i].second.size() - 1; j >= 0; j-- ){
                stack.push(productions_ll1_vector[i].second[j]);
            }
        }    
    }
    return true;
}

int main(){
    get_grammar("grammar.txt");
    convert_to_ll1();
    get_first_set();
    get_follow_set();
    get_select_set();
    get_table();
    // ll1_parsing
    cout << "\n\nll1_parsing" << endl;
    bool result = ll1_parsing();
    if ( result ){
        cout << "\nsuccess" << endl;
    }
    else{
        cout << "\nerror" << endl;
    }
    return 0;
}