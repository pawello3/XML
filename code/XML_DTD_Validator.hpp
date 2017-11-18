#ifndef XML_DTD_VALIDATOR_HPP_INCLUDED
#define XML_DTD_VALIDATOR_HPP_INCLUDED

#include <list>
#include <unordered_set>

struct Automaton_Node
{
	Automaton_Node* next1;
	Automaton_Node* next2;
	char type1;
	char type2;
};

struct Automaton_Fragment
{
	Automaton_Node* startNode;
	Automaton_Node* endNode;
};

class Automaton
{
	public:
		Automaton(std::string regular_expression);
		~Automaton(void);

		bool matches_regex(std::list<char>* children);

	private:
		std::string regular_expression;
		bool automaton_already_constructed;
		Automaton_Node* root;
		Automaton_Node* final_state;

		std::string convert_to_reverse_Polish_notation(std::string input);
		void build_automaton(void);
		std::unordered_set<Automaton_Node*>
		compute_epsilon_closure(Automaton_Node* node);
		std::unordered_set<Automaton_Node*>
		compute_epsilon_closure(std::unordered_set<Automaton_Node*> states);
		std::unordered_set<Automaton_Node*>
		compute_one_step_closure(Automaton_Node* node, char label);
		std::unordered_set<Automaton_Node*>
		compute_one_step_closure(std::unordered_set<Automaton_Node*> states,
		char label);
};

#endif // XML_DTD_VALIDATOR_HPP_INCLUDED
