#include "api.hpp"
#include <string>
#include <map>
#include <iostream>

using namespace std;

string add_brackets(string s){
	if (s.length() > 1)
		return "(" + s + ")";
	return s;
}

set<string> get_in_states(string state, set<string> &dfa_states, map<string, set<string>> &state2state){
	set<string> in_states;
	in_states.clear();
	for (auto & in_state : dfa_states)
		if (state2state[in_state].find(state) != state2state[in_state].end() && in_state != state)
			in_states.insert(in_state);
	return in_states;
}

void remove_state(string state, set<string> &dfa_states, map<string, set<string>> &state2state,map<pair<string, string>, string> &move2re){
	set<string> out_states = state2state[state];
	if (out_states.find(state) != out_states.end())
		out_states.erase(out_states.find(state));
	set<string> in_states = get_in_states(state, dfa_states, state2state);
	string state_iteration = "";
	set<string> empty_set;
	empty_set.clear();
	pair<string, string> inner_move_state = pair<string, string>(state, state);
	if (move2re.find(inner_move_state) != move2re.end())
		state_iteration = add_brackets(move2re[inner_move_state]) + "*";
	for (auto & in_state : in_states){
		state2state[in_state].erase(state2state[in_state].find(state));
		pair<string, string> in2state = pair<string, string>(in_state, state);;
		for (auto & out_state : out_states){
			pair<string, string> in2out = pair<string, string>(in_state, out_state);
			pair<string, string> state2out = pair<string, string>(state, out_state);
			string in2state_re = add_brackets(move2re[in2state]);
			string state2out_re = add_brackets(move2re[state2out]);
			if (move2re.find(in2out) != move2re.end())
				move2re[in2out] += ("|" + in2state_re + state_iteration + state2out_re);
			else{
				if (state2state.find(in_state) == state2state.end())
					state2state[state] = empty_set;
				state2state[in_state].insert(out_state);
				move2re[in2out] = in2state_re + state_iteration + state2out_re;
			}
		}
		move2re.erase(in2state);
	}
	for (auto & out_state : out_states){
		pair<string, string> state2out = pair<string, string>(state, out_state);
		move2re.erase(state2out);
	}
	dfa_states.erase(dfa_states.find(state));
	state2state.erase(state);
}

std::string dfa2re(DFA &d){

	map<string, set<string>> state2state;
	map<pair<string, string>, string> move2re;
	set<string> empty_set;
	empty_set.clear();
	state2state.clear();
	move2re.clear();
	Alphabet our_language = d.get_alphabet();
	set<string> dfa_states = d.get_states();
	int64_t dfa_length = dfa_states.size() + 1;
	string final_state = "final_state";

	for (auto & state : dfa_states){
		for (auto & symbol : our_language){
			if (d.has_trans(state, symbol)){
				string out_state = d.get_trans(state, symbol);
				if (state2state.find(state) == state2state.end())
					state2state[state] = empty_set;
				state2state[state].insert(out_state);
				pair<string, string> move = pair<string, string>(state, out_state);
				if (move2re.find(move) == move2re.end())
					move2re[move] = string(1, symbol);
				else
					move2re[move] +=  ("|" + string(1, symbol));
			}
		}
		if (d.is_final(state)){
			if (state2state.find(state) == state2state.end())
				state2state[state] = empty_set;
			state2state[state].insert("final_state");
			pair<string, string> state2finstate = pair<string, string>(state, final_state);
			move2re[state2finstate] = "";
		}
	}

	dfa_states.insert(final_state);

	while (dfa_length != 2){
		for (auto & state : dfa_states){
			if (!(state == final_state || d.is_initial(state))){
				remove_state(state, dfa_states, state2state, move2re);
				dfa_length--;
				break;
			}
		}
	}

	string final_re;
	string initial_state = d.get_initial_state();
	pair<string, string> ini2ini = pair<string, string>(initial_state, initial_state);
	pair<string, string> ini2fin = pair<string, string>(initial_state, final_state);
	pair<string, string> fin2fin = pair<string, string>(final_state, final_state);
	pair<string, string> fin2ini = pair<string, string>(final_state, initial_state);
	string ini2ini_re = "";
	if (move2re.find(ini2ini) != move2re.end())
		ini2ini_re = add_brackets(move2re[ini2ini]);
	string fin2fin_re = "";
	if (move2re.find(fin2fin) != move2re.end())
		fin2fin_re = add_brackets(move2re[fin2fin]) + "*";
	string ini2fin_re = "";
	if (move2re.find(ini2fin) != move2re.end())
		ini2fin_re = add_brackets(move2re[ini2fin]);
	string fin2ini_re = "";
	if (move2re.find(fin2ini) != move2re.end())
		fin2ini_re = add_brackets(move2re[fin2ini]);
	if (fin2ini_re.length() > 0)
		final_re = "("+ini2ini_re+"|"+ini2fin_re+fin2fin_re+fin2ini_re+")*"+ini2fin_re+fin2fin_re;
	else
		final_re = "("+ini2ini_re+")*"+ini2fin_re+fin2fin_re;

	return final_re;
}
