#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<algorithm>
#include<fstream>
#include<sstream>
#include<set>
#include<stack>
#include<boost/algorithm/string.hpp>
using namespace std;
using namespace boost;

map<string, bool> is_terminal;
map<string, vector<string> > productions;
map<string, vector<vector<string>> > productions_lr0;
vector<pair<string, vector<string>>> productions_lr0_vector;
vector<set<pair<string, vector<string>>>> sets;
map<pair<string, int>, pair<string, int>> table;

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

void convert_productions_from_string_to_vec(){
    int n = productions.size();
    for ( auto & it : productions ){
        stringstream ss(it.second[1]);
        vector<string> production = {istream_iterator<string>(ss), istream_iterator<string>()};
        for ( auto & itt : it.second ){
            stringstream ss(itt);
            vector<string> production_lr0 = {istream_iterator<string>(ss), istream_iterator<string>()};
            productions_lr0[it.first].push_back( production_lr0 );
        }
    }
    // convert to vector
    for ( auto & it : productions_lr0 ){
        for ( auto & it1 : it.second ){
            productions_lr0_vector.push_back( make_pair(it.first, it1) );
        }
    }
    // vec.insert( vec.begin(), "." );
    productions_lr0_vector.insert( productions_lr0_vector.begin(), make_pair("E'", vector<string>{"E"}) );
    // print productions
    for (auto & production : productions_lr0){
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
    cout << endl;
    // print productions_lr0_vector
    for (auto & production : productions_lr0_vector){
        cout << production.first << " -> ";
        for (auto & p : production.second){
            cout << p;
        }
        cout << endl;
    }
    cout << endl;
}

typedef set<pair<string, vector<string>>> Set; 
Set closure( Set & set ){
    while( true ){
        Set new_set = set;
        for ( auto & production : set ){
            // find the symbol following "."
            // vector<int>::iterator it = find(vec.begin(), vec.end(), 6);
            auto it = find(production.second.begin(), production.second.end(), ".");
            if ( (it+1) != production.second.end() && !is_terminal[*(it+1)] ){
                for ( auto & pro : productions_lr0_vector ){
                    if ( pro.first == *(it+1) ){
                        // push "." to pro.second's front
                        auto vec = pro.second;
                        vec.insert( vec.begin(), "." );
                        set.insert( make_pair(pro.first, vec) );
                    }
                }
            } 
        }
        if ( new_set == set ){
            break;
        }
    }
    // print set
    return set;
}

void goto_sets( const Set & set ){
    auto it1 = find(sets.begin(), sets.end(), set);
    int index1 = boost::distance(sets.begin(), it1);
    for ( auto & symbol_pair : is_terminal ){
        auto & symbol = symbol_pair.first;
        Set new_set;
        for ( auto & production : set ){
            auto it = find(production.second.begin(), production.second.end(), ".");
            if ( it != production.second.end() && (it+1) != production.second.end() && *(it+1) == symbol ){
                // push "." to pro.second's front
                auto vec = production.second;
                // convert it to int index
                int index = boost::distance(production.second.begin(), it);
                // swap "." and symbol
                vec[index] = symbol;
                vec[index+1] = "."; 
                // Set new_set = {make_pair(production.first, vec)};
                new_set.insert( make_pair(production.first, vec) );
            }
        }
        if ( new_set.empty() ){
            continue;
        }
        new_set = closure( new_set );
        // print new_set
        cout << "goto(" << symbol << ") = ";
        for ( auto & production : new_set ){
            cout << production.first << " -> ";
            for ( auto & p : production.second ){
                cout << p;
            }
            cout << endl;
        }
        cout << endl;
        // if closure of new_set is not in sets
        if ( find(sets.begin(), sets.end(), new_set) == sets.end() ){
            sets.push_back( new_set );
            goto_sets( new_set );
        }
        // add to table
        auto it2 = find(sets.begin(), sets.end(), new_set);
        int index2 = boost::distance(sets.begin(), it2);
        table[make_pair(symbol, index1)] = make_pair("s", index2);
    }
}

void get_sets_and_table(){
    auto vec = productions_lr0_vector[0];
    vec.second.insert( vec.second.begin(), "." );
    Set s1 = {vec};
    sets.push_back( closure(s1) );
    // print sets 
    goto_sets( s1 );
    // reduction
    for ( int i = 0; i < sets.size(); i++ ){
        for ( auto & production : sets[i] ){
            auto it = find(production.second.begin(), production.second.end(), ".");
            if ( it != production.second.end() && (it+1) == production.second.end()){
                auto vec = production;
                vec.second.pop_back();
                auto it2 = find(productions_lr0_vector.begin(), productions_lr0_vector.end(), vec);
                int index2 = boost::distance(productions_lr0_vector.begin(), it2);
                if ( index2 == 0 ){
                    table[make_pair("#", i)] = make_pair("acc", 0);
                    continue;
                }
                for ( auto & symbol : is_terminal ){
                    if ( symbol.second && table.find(make_pair(symbol.first, i)) == table.end() ){
                        table[make_pair(symbol.first, i)] = make_pair("r", index2);
                    }
                }
            }
        }
    }
    cout << "printing sets" << endl;
    cout << "size of sets: " << sets.size() << endl;
    for ( auto & set : sets ){
        cout << "{";
        for ( auto & production : set ){
            cout << production.first << " -> ";
            for ( auto & p : production.second ){
                cout << p;
                cout << " ";
            }
            cout << endl;
        }
        cout << "}" << endl;
    }
    // print table
    cout << "printing table" << endl;
    for ( auto & it : table ){
        cout << it.first.first << " " << it.first.second << " " << it.second.first << " " << it.second.second << endl;
    }
}

void print_error( vector<string> & input_vector, stack<int> & st, vector<string> & symbols, int w){
    cout << "input: ";
    for ( auto & s : input_vector ){
        cout << s << " ";
    }
    cout << endl;
    cout << "stack: ";
    while ( !st.empty() ){
        cout << st.top() << " ";
        st.pop();
    }
    cout << "symbols: ";
    for ( auto & s : symbols ){
        cout << s << " ";
    }
    cout << "w: " << input_vector[w];
    cout << endl;
}

bool lr0_parsing(){
    string input;
    cout << "please input your expression: ";
    getline(cin, input);
    vector<string> input_vector;
    for ( int i = 0; i < input.size(); i++ ){
        //cout << i << endl;
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
            //cout << "push i at " << i << endl;
            while ( i < input.size() && ( isalnum(input[i]) || input[i] == '.') ){
                i++;
            }
            i--;
            //cout << " i at " << i << endl;
        }
    }
    input_vector.push_back("#");
    cout << "input: ";
    for ( auto & s : input_vector ){
        cout << s << " ";
    }
    cout << endl;
    stack<int> stack;
    stack.push(0);
    int w = 0;
    vector<string> symbols;
    // cout << "haiyaa" << endl;
    while( !stack.empty() ){
        // cout << "haiyaa" << endl;
        // print stack top
        cout << stack.top() << " ";
        // print input
        cout << input_vector[w] << " " ;
        // print symbols
        for ( auto & s : symbols ){
            cout << s << " ";
        }
        cout << endl;
        string symbol = input_vector[w];
        int index = stack.top();
        auto it = table.find(make_pair(symbol, index));
        if ( it == table.end() ){
            cout << "error: cannot find table entry, cannot get action" << endl;
            print_error(input_vector, stack, symbols, w);
            return false;
        }
        string action = it->second.first;
        cout << "action: " << action << endl;
        int index2 = it->second.second;
        if ( action == "s" ){
            stack.push(index2);
            symbols.push_back(symbol);
            w++;
        }
        else if ( action == "r" ){
            auto vec = productions_lr0_vector[index2];
            for ( int i = 0; i < vec.second.size(); i++ ){
                symbols.pop_back();
                stack.pop();
            }
            auto it2 = table.find(make_pair(vec.first, stack.top()));
            if ( it2 == table.end() ){
                cout << "error: cannot find table entry, cannot push back non-terminal" << endl;
                print_error(input_vector, stack, symbols, w);
                return false;
            }
            int index3 = it2->second.second;
            stack.push(index3);
            symbols.push_back(vec.first);
        }
        else if ( action == "acc" ){
            cout << "accept" << endl;
            return true;
        }
        else {
            cout << "error: invalid action" << endl;
            print_error(input_vector, stack, symbols, w);
            return false;
        }
    }
    return false;
}

int main(){
    get_grammar("grammar.txt");
    convert_productions_from_string_to_vec();
    get_sets_and_table();
    lr0_parsing();
    return 0;
}