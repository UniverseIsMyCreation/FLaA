#include "api.hpp"
#include <string>
#include <set>
#include <iostream>
#include <map>
#include <queue>
#include <vector>
#include <algorithm>

using namespace std;

void print_class_eq(vector<set<string>> classes_eq){
	for (auto & class_eq : classes_eq){
		for (auto & it : class_eq){
			cout << it << ' ';
		}
		cout << endl;
	}
}

pair<set<string>, set<string>> split(set<string> &class_eq, pair<set<string>, char> &move, DFA &d){
	set<string> part1, part2;
	part1.clear();
	part2.clear();
	set<string> parts_union = move.first;
	char symbol = move.second;
	for (auto & state: class_eq){
		string out_state = d.get_trans(state, symbol);
		if (parts_union.find(out_state) != parts_union.end()){
			part1.insert(state);
		}
		else{
			part2.insert(state);
		}
	}
	return pair<set<string>, set<string>>(part1, part2);
}

vector<set<string>> get_class_eq(DFA & d){

	set<string> final_states = d.get_final_states();
	set<string> dfa_states = d.get_states();
	Alphabet our_language = d.get_alphabet();
	set<string> non_final_states;
	non_final_states.clear();

	for (auto & state : dfa_states){
		if (final_states.find(state) == final_states.end()){
			non_final_states.insert(state);
		}
	}

	vector<set<string>> classes_eq;
	classes_eq.clear();
	if (!non_final_states.empty()){
		classes_eq.push_back(non_final_states);
	}
	if (!final_states.empty()){
		classes_eq.push_back(final_states);
	}
	
	queue<pair<set<string>, char>> move_queue;
	for (auto & symbol : our_language){
		for (auto & class_eq : classes_eq){
			pair<set<string>, char> move = make_pair(class_eq, symbol);
			move_queue.push(move);
		}
	}
	vector<set<string>> new_classes_eq;
	new_classes_eq.clear();
	while (!move_queue.empty()){
		pair<set<string>, char> move = move_queue.front();
		move_queue.pop();
		for (auto class_eq : classes_eq){
			if (class_eq.size() == 1){
				new_classes_eq.push_back(class_eq);
				continue;
			}
			pair<set<string>, set<string>> new_parts = split(class_eq, move, d);
			set<string> part1 = new_parts.first;
			set<string> part2 = new_parts.second;
			if (part1.size() > 0 && part2.size() > 0){
				new_classes_eq.push_back(part1);
				new_classes_eq.push_back(part2);
				for (auto symbol : our_language){
					pair<set<string>, char> part1_symbol = make_pair(part1, symbol);
					pair<set<string>, char> part2_symbol = make_pair(part2, symbol);
					move_queue.push(part1_symbol);
					move_queue.push(part2_symbol);
				}
			}
			else{
				new_classes_eq.push_back(class_eq);
			}
		}
		classes_eq = new_classes_eq;
		new_classes_eq.clear();
	}

	return classes_eq;
}

void delete_unreachable_states(DFA &clean_d, DFA &d, Alphabet &our_language){

	set<string> initial_dfa_states= d.get_states();
	string initial_dfa_state = d.get_initial_state();
	set<string> reachable_states;
	reachable_states.clear();
	reachable_states.insert(initial_dfa_state);
	bool is_final;

	queue<string> queue_states;
	queue_states.push(initial_dfa_state);
	while (!queue_states.empty()){
		string cur_state = queue_states.front();
		queue_states.pop();
		for (auto & symbol : our_language){
			if (d.has_trans(cur_state, symbol)){
				string out_state = d.get_trans(cur_state, symbol);
				if (reachable_states.find(out_state) == reachable_states.end()){
					reachable_states.insert(out_state);
					queue_states.push(out_state);
				}
			}
		}
	}

	for (auto & state : reachable_states){
		for (auto & symbol : our_language){
			if (d.has_trans(state, symbol)){
				string out_state = d.get_trans(state, symbol);
				is_final = d.is_final(state);
				clean_d.create_state(state, is_final = is_final);
				if (d.is_initial(state)){
					clean_d.set_initial(state);
				}
				is_final = d.is_final(out_state);
				clean_d.create_state(out_state, is_final = is_final);
				clean_d.set_trans(state, symbol, out_state);
			}
		}
	}
}

void implement_dead_state(DFA &clean_d, Alphabet &our_language){
	
	set<string> dfa_states = clean_d.get_states();
	string dead_state = "dead_state";
	clean_d.create_state(dead_state);

	for (auto & state : dfa_states){
		for (auto & symbol : our_language){
			if (!clean_d.has_trans(state, symbol)){
				clean_d.set_trans(state, symbol, dead_state);
			}
		}
	}

	for (auto & symbol : our_language){
		clean_d.set_trans(dead_state, symbol, dead_state);
	}
}

DFA update_dfa(DFA &d){
	Alphabet our_language = d.get_alphabet();
	DFA clean_d(our_language);
	delete_unreachable_states(clean_d, d, our_language);
	implement_dead_state(clean_d, our_language);
	return clean_d;
}

DFA dfa_minim(DFA &d) {

	string dead_state = "dead_state";
	DFA clean_d = update_dfa(d);

	set<string> clean_dfa_states = clean_d.get_states();
	if (clean_dfa_states.size() == 1 && clean_dfa_states.find(dead_state) != clean_dfa_states.end()){
		DFA new_d(d.get_alphabet());
		string only_state = "0";
		new_d.create_state(only_state, true);
		new_d.set_initial(only_state);
		return new_d;
	}	

	vector<set<string>> classes_eq = get_class_eq(clean_d);

	vector<set<string>> classes_eq_without_ds;
	Alphabet our_language = clean_d.get_alphabet();
	classes_eq_without_ds.clear();
	string initial_state_d = d.get_initial_state();
	set<string> initial_state_new_d;
	for (auto & class_eq : classes_eq){
		if (class_eq.find(dead_state) == class_eq.end()){
			classes_eq_without_ds.push_back(class_eq);
		}
		if (class_eq.find(initial_state_d) != class_eq.end()){
			initial_state_new_d = class_eq;
		}
	}

	map<set<string>, string> new_state2num;
	new_state2num.clear();
	int index = 0;
	for (auto new_state : classes_eq_without_ds){
		new_state2num[new_state] = to_string(index);
		index++;
	}

	map<pair<set<string>, char>, set<string>> move_how;
	map<set<string>, bool> state2is_final;
	state2is_final.clear();
	move_how.clear();
	set<char> used_symbols;
	for (auto & class_eq : classes_eq_without_ds){
		used_symbols.clear();
		state2is_final[class_eq] = false;
		for (auto & in_state : class_eq){
			for (auto & symbol : our_language){
				if (used_symbols.find(symbol) != used_symbols.end()){
					continue;
				}
				string out_state = clean_d.get_trans(in_state, symbol);
				if (out_state == dead_state){
					continue;
				}
				used_symbols.insert(symbol);
				for (auto & inner_class_eq : classes_eq_without_ds){
					if (inner_class_eq.find(out_state) != inner_class_eq.end()){
						move_how[make_pair(class_eq, symbol)] = inner_class_eq;
					}
				}
			}
			if (clean_d.is_final(in_state)){
				state2is_final[class_eq] = true;
			}
		}
	}

	DFA new_d(our_language);

	for (auto move : move_how){
		set<string> in_state = move.first.first;
		char symbol = move.first.second;
		set<string> out_state = move.second;
		bool is_final_in = state2is_final[in_state];
		bool is_final_out = state2is_final[out_state];
		if (!new_d.has_state(new_state2num[in_state])){
			new_d.create_state(new_state2num[in_state], is_final_in);
		}
		if (!new_d.has_state(new_state2num[out_state])){
			new_d.create_state(new_state2num[out_state], is_final_out);
		}
		new_d.set_trans(new_state2num[in_state], symbol, new_state2num[out_state]);
	}

	new_d.set_initial(new_state2num[initial_state_new_d]);

	return new_d;
}