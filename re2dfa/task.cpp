#include "api.hpp"
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <map>

using namespace  std;

set<int64_t> set_union(set<int64_t> a, set<int64_t> b){
    for (auto it = b.begin(); it != b.end(); it++){
        a.insert(*it);
    }
    return a;
}

set<int64_t> set_mul(set<int64_t> a, set<int64_t> b, bool nul, char mode){
    if (nul){
        return set_union(a, b);
    }
    else{
        if (mode == 'f')
            return a;
        else
            return b;
    }

}

bool vector_contains(vector<set<int64_t>> marked_states, set<int64_t> state){
    for (auto it = marked_states.begin(); it != marked_states.end(); it++){
        if (state == *it){
            return true;
        }
    }
    return false;
}

set<int64_t> set_intersection(set<int64_t> st1, set<int64_t> st2){
    set<int64_t> st_intersection;
    st_intersection.clear();
    for (auto it = st1.begin(); it != st1.end(); it++){
        if (st2.find(*it) != st2.end()){
            st_intersection.insert(*it);
        }
    }
    return st_intersection;
}

void set_union_main(set<int64_t> & st_main, set<int64_t> st){
    for (auto it = st.begin(); it != st.end(); it++){
        st_main.insert(*it);
    }
}

string find_key(set<int64_t> state, map<string, set<int64_t>> statechar2stateset){
    for (auto & it : statechar2stateset){
        if (state == it.second){
            return it.first;
        }
    }
}

class Tree_Node{
public:
    struct Tree_Node * left;
    struct Tree_Node * right;
    bool nullable;
    set<int64_t> firstpos;
    set<int64_t> lastpos;
    string operation;

    Tree_Node(string operation="-", set<int64_t> firstpos=set<int64_t>(), set<int64_t> lastpos=set<int64_t>(), bool nullable=false){
        if (operation != "special"){
            this->nullable = nullable;
            this->firstpos = firstpos;
            this->lastpos = lastpos;
        }
        else{
            this->nullable = true;
            this->firstpos = set<int64_t>();
            this->lastpos = set<int64_t>();
        }
        this->operation = operation;
        this->left = NULL;
        this->right = NULL;
    }
    ~Tree_Node(){}
};

void go_through_parse_Tree(vector<set<int64_t>> & followpos, Tree_Node * Node){
    if (Node->left != NULL && Node->right != NULL && Node->operation == "."){
        set<int64_t> st1 = Node->left->lastpos;
        set<int64_t> st2 = Node->right->firstpos;
        for (auto it1 = st1.begin(); it1 != st1.end(); it1++){
            for (auto it2 = st2.begin(); it2 != st2.end(); it2++){
                followpos[*it1 - 1].insert(*it2);
            }
        }
        go_through_parse_Tree(followpos, Node->left);
        go_through_parse_Tree(followpos, Node->right);
    }
    else if (Node->operation == "*" && Node->left != NULL){
        set<int64_t> st1 = Node->lastpos;
        set<int64_t> st2 = Node->firstpos;
        for (auto it1 = st1.begin(); it1 != st1.end(); it1++){
            for (auto it2 = st2.begin(); it2 != st2.end(); it2++){
                followpos[*it1 - 1].insert(*it2);
            }
        }
        go_through_parse_Tree(followpos, Node->left);
    }
    else if (Node->operation == "|"){
        go_through_parse_Tree(followpos, Node->left);
        go_through_parse_Tree(followpos, Node->right);
    }
}

Tree_Node * S(string, int &, Alphabet, int64_t &);
Tree_Node * A(string, int &, Alphabet, int64_t &);
Tree_Node * B(string, int &, Alphabet, int64_t &);
Tree_Node * C(string, int &, Alphabet, int64_t &);
Tree_Node * D(string, int &, Alphabet, int64_t &);

Tree_Node * D(string s, int & i, Alphabet our_language, int64_t & number){
    Tree_Node * left_Node = NULL;
    if (our_language.has_char(s[i]) || s[i] == '#' || s[i] == '>'){
        set<int64_t> val;
        val.insert(number);
        if (s[i] != '>'){
            left_Node = new Tree_Node("-", val, val);
            number++;
        }
        else
            left_Node = new Tree_Node("special");
        i++;
    }
    else if (s[i] == '('){
        string new_s("");
        int forward_i = i + 1;
        int brackets_amount = 1;
        while(true){
            if (s[forward_i] == ')'){
                brackets_amount--;
                if (brackets_amount == 0){
                    break;
                }
            }
            else if (s[forward_i] == '('){
                brackets_amount++;
            }
            new_s = new_s + s[forward_i];
            forward_i++;
        }
        i = forward_i + 1;
        int new_i = 0;
        left_Node = A(new_s, new_i, our_language, number);
    }
    return left_Node;
}

Tree_Node * C(string s, int & i, Alphabet our_language, int64_t & number){
    Tree_Node * left_Node = D(s, i, our_language, number);
    if (i < s.length()){
        if (s[i] == '*'){
            i++;
            Tree_Node * middle_Node = new Tree_Node("*", set<int64_t>(), set<int64_t>(), true);
            middle_Node->left = left_Node;
            middle_Node->right = NULL;
            middle_Node->firstpos = left_Node->firstpos;
            middle_Node->lastpos = left_Node->lastpos;
            return middle_Node;
        }
    }
    return left_Node;
}

Tree_Node * B(string s, int & i, Alphabet our_language, int64_t & number){
    Tree_Node * left_Node = C(s, i, our_language, number);
    Tree_Node * operation_Node = NULL;
    Tree_Node * right_Node = NULL;
    if (i < s.length()){
        if (our_language.has_char(s[i]) || s[i] == '#' || s[i] == '('){
            operation_Node = new Tree_Node(".");
            operation_Node->left = left_Node;
            right_Node = B(s, i, our_language, number);
            operation_Node->right = right_Node;
            operation_Node->nullable = operation_Node->left->nullable && operation_Node->right->nullable;
            bool nullable1 = operation_Node->left->nullable;
            bool nullable2 = operation_Node->right->nullable;
            operation_Node->firstpos = set_mul(operation_Node->left->firstpos, operation_Node->right->firstpos, nullable1, 'f');
            operation_Node->lastpos = set_mul(operation_Node->left->lastpos, operation_Node->right->lastpos, nullable2, 'l');
            return operation_Node;
        }
    }
    return left_Node;
}

Tree_Node * A(string s, int & i, Alphabet our_language, int64_t & number){
    Tree_Node * left_Node = B(s, i, our_language, number);
    Tree_Node * operation_Node = NULL;
    Tree_Node * right_Node = NULL;
    if (i < s.length()){
        if (s[i] == '|'){
            i++;
            operation_Node = new Tree_Node("|");
            operation_Node->left = left_Node;
            right_Node = A(s, i, our_language, number);
            operation_Node->right = right_Node;
            operation_Node->nullable = operation_Node->left->nullable || operation_Node->right->nullable;
            operation_Node->firstpos = set_union(operation_Node->left->firstpos, operation_Node->right->firstpos);
            operation_Node->lastpos = set_union(operation_Node->left->lastpos, operation_Node->right->lastpos);
            return operation_Node;
        }
    }
    return left_Node;
}

Tree_Node * S(string s, int & i, Alphabet our_language, int64_t & number){
    if (s.length() == 3){
        Tree_Node * Node = new Tree_Node();
        set<int64_t> val;
        val.insert(1);
        Node->firstpos = val;
        Node->lastpos = val;
        Node->nullable = false;
        Node->operation = "-";
        return Node;
    }
    else{
        return A(s, i, our_language, number);
    }
}

DFA re2dfa(const std::string &s) {

    if (s.length() == 0){
        string new_s = "a";
        DFA res = DFA(Alphabet(new_s));
        res.create_state("0", true);
        res.set_initial("0");
        return res;
    }

    Alphabet our_language = Alphabet(s);
    DFA res = DFA(Alphabet(s));
    string s_ended = "(" + s + ")#";
    map<char, set<int64_t>> re_numbered;
    re_numbered.clear();
    int64_t number = 1;

    int64_t re_length = s_ended.length();
    for (int i = 0; i < re_length; i++){
        if (our_language.has_char(s_ended[i]) || s_ended[i] == '#'){
            re_numbered[s_ended[i]].insert(number);
            number++;
        }
    }

    // add eps to re
    int i = 0;
    int cur_re_length = s_ended.length();
    while (true){
        if (i == cur_re_length){
            break;
        }
        else if (s_ended[i] == '(' && s_ended[i + 1] == '|'){
            s_ended.insert(i + 1, ">");
            i = i + 2;
            cur_re_length += 1;
        }
        else if (s_ended[i] == '|' && (s_ended[i + 1] == '|' || s_ended[i + 1] == ')')){
            s_ended.insert(i + 1, ">");
            i = i + 2;
            cur_re_length += 1;
        }
        else{
            i++;
        }
    }

    int64_t max_number = number - 1;

    // create parse tree
    number = 1;
    i = 0;
    Tree_Node * parse_Tree = S(s_ended, i, our_language, number);

    // create followpos table
    vector<set<int64_t>> followpos;
    followpos.clear();
    for (int j = 0; j < max_number; j++){
        followpos.push_back(set<int64_t>());
    }
    go_through_parse_Tree(followpos, parse_Tree);

    // create DFA
    int64_t cur_state = 1;
    vector<set<int64_t>> dfa_states;
    dfa_states.clear();
    dfa_states.push_back(parse_Tree->firstpos);
    vector<set<int64_t>> marked_states;
    marked_states.clear();
    map<string, set<int64_t>> statechar2stateset;
    statechar2stateset["0"] = dfa_states[0];
    if (dfa_states[0].find(max_number) != dfa_states[0].end()){
        res.create_state("0", true);
    }
    else{
        res.create_state("0");
    }
    res.set_initial("0");
    bool new_state_added = true;
    vector<set<int64_t>> added_states;
    while(new_state_added){
        new_state_added = false;
        added_states.clear();
        for (auto & state : dfa_states){
            if (!vector_contains(marked_states, state)){
                string key_state = find_key(state, statechar2stateset);
                marked_states.push_back(state);
                set<int64_t> where_can_go;
                set<int64_t> where_can_go_state;
                set<int64_t> a_state;
                for (auto & a : our_language){
                    a_state.clear();
                    where_can_go.clear();
                    where_can_go_state.clear();
                    where_can_go = re_numbered[a];
                    where_can_go_state = set_intersection(where_can_go, state);
                    for (auto & can_go_state : where_can_go_state){
                        set_union_main(a_state, followpos[can_go_state - 1]);
                    }
                    if (a_state.size() != 0){
                        if (!vector_contains(dfa_states, a_state) && !vector_contains(added_states, a_state)){
                            new_state_added = true;
                            added_states.push_back(a_state);
                            statechar2stateset[to_string(cur_state)] = a_state;
                            if (a_state.find(max_number) != a_state.end()){
                                res.create_state(to_string(cur_state), true);
                            }
                            else{
                                res.create_state(to_string(cur_state));
                            }
                            res.set_trans(key_state, a, to_string(cur_state));
                            cur_state++;
                        }
                        else{
                            string key_state_a_state = find_key(a_state, statechar2stateset);
                            res.set_trans(key_state, a, key_state_a_state);
                        }
                    }
                }
            }
        }
        for (auto & it : added_states){
            dfa_states.push_back(it);
        }
    }

    return res;
}