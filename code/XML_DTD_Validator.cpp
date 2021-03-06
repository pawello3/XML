// Author: Paweł Guzewicz

#include "XML_DTD_Validator.hpp"
#include <stack>

Automaton::Automaton(std::string regular_expression)
	:regular_expression(regular_expression)
	,automaton_already_constructed(false)
	,root(nullptr)
	,final_state(nullptr)
{
}

Automaton::~Automaton(void)
{
	delete root;
}

std::string Automaton::convert_to_reverse_Polish_notation(std::string input)
{
	unsigned int input_size = input.size();
	if(input_size == 1 && input[0] == '_')
		return input;

	std::stack<char> aux_stack = std::stack<char>();
	std::string output = "";

	for(unsigned int i = 0; i < input_size; i++)
	{
		switch(input[i])
		{
			case '?':
			case '+':
			case '*':
				aux_stack.push(input[i]);
				break;
			case ')':
				while(aux_stack.top() != '(')
				{
					output += aux_stack.top();
					aux_stack.pop();
				}
				aux_stack.pop();
				break;
			case '(':
			default:
				if(i > 0)
				{
					if(input[i - 1] != '(')
					{
						while(!aux_stack.empty() && aux_stack.top() != '(')
						{
							output += aux_stack.top();
							aux_stack.pop();
						}
						aux_stack.push('.'); // concatenation
					}
				}
				if(input[i] == '(')
					aux_stack.push(input[i]);
				else
					output += input[i];
				break;
		}
	}
	while(!aux_stack.empty())
	{
		output += aux_stack.top();
		aux_stack.pop();
	}

	return output;
}

void Automaton::build_automaton(void)
{
    std::string postfix_regex =
		convert_to_reverse_Polish_notation(this->regular_expression);
    if(postfix_regex == "_")
		return;

    std::stack<Automaton_Fragment*> fragments =
		std::stack<Automaton_Fragment*>();
	Automaton_Node* left = nullptr;
	Automaton_Node* right = nullptr;
	Automaton_Fragment* fragment_top = nullptr;
	Automaton_Fragment* fragment_top1 = nullptr;
	Automaton_Fragment* fragment_top2 = nullptr;

	// Types of the transitions
	// label: character describing the transition
	// '\0': no transition (also pointer set to nullptr)
    // 127: epsilon transition

    // each node has at most two transitions by construction
    // endNode is always the final state of the subautomaton (points to nowhere)

	for(unsigned int i = 0; i < postfix_regex.size(); i++)
	{
		switch(postfix_regex[i])
		{
			case '.':
				fragment_top2 = fragments.top();
				fragments.pop();
				fragment_top1 = fragments.top();
				fragments.pop();
				fragment_top1->endNode->next1 = fragment_top2->startNode->next1;
				fragment_top1->endNode->type1 = fragment_top2->startNode->type1;
				fragment_top1->endNode->next2 = fragment_top2->startNode->next2;
				fragment_top1->endNode->type2 = fragment_top2->startNode->type2;
				delete fragment_top2->startNode;
				fragment_top1->endNode = fragment_top2->endNode;
				delete fragment_top2;
				fragments.push(fragment_top1);
				break;
			case '?':
				fragment_top = fragments.top();
				fragments.pop();
				left = new Automaton_Node();
				right = new Automaton_Node();
				left->next1 = fragment_top->startNode;
				left->next2 = right;
				left->type1 = 127;
				left->type2 = 127;
				fragment_top->endNode->next1 = right;
				fragment_top->endNode->next2 = nullptr;
				fragment_top->endNode->type1 = 127;
				fragment_top->endNode->type2 = '\0';
				right->next1 = nullptr;
				right->next2 = nullptr;
				right->type1 = '\0';
				right->type2 = '\0';
				fragment_top->startNode = left;
				fragment_top->endNode = right;
				fragments.push(fragment_top);
				break;
			case '+':
				fragment_top = fragments.top();
				fragments.pop();
				left = new Automaton_Node();
				right = new Automaton_Node();
				left->next1 = fragment_top->startNode;
				left->next2 = nullptr;
				left->type1 = 127;
				left->type2 = '\0';
				fragment_top->endNode->next1 = right;
				fragment_top->endNode->next2 = fragment_top->startNode;
				fragment_top->endNode->type1 = 127;
				fragment_top->endNode->type2 = 127;
				right->next1 = nullptr;
				right->next2 = nullptr;
				right->type1 = '\0';
				right->type2 = '\0';
				fragment_top->startNode = left;
				fragment_top->endNode = right;
				fragments.push(fragment_top);
				break;
			case '*':
				fragment_top = fragments.top();
				fragments.pop();
				left = new Automaton_Node();
				right = new Automaton_Node();
				left->next1 = fragment_top->startNode;
				left->next2 = right;
				left->type1 = 127;
				left->type2 = 127;
				fragment_top->endNode->next1 = right;
				fragment_top->endNode->next2 = fragment_top->startNode;
				fragment_top->endNode->type1 = 127;
				fragment_top->endNode->type2 = 127;
				right->next1 = nullptr;
				right->next2 = nullptr;
				right->type1 = '\0';
				right->type2 = '\0';
				fragment_top->startNode = left;
				fragment_top->endNode = right;
				fragments.push(fragment_top);
				break;
			default:
				left = new Automaton_Node();
				right = new Automaton_Node();
				left->next1 = right;
				left->next2 = nullptr;
				left->type1 = postfix_regex[i];
				left->type2 = '\0';
				right->next1 = nullptr;
				right->next2 = nullptr;
				right->type1 = '\0';
				right->type2 = '\0';
				Automaton_Fragment* fragment = new Automaton_Fragment();
				fragment->startNode = left;
				fragment->endNode = right;
				fragments.push(fragment);
				break;
		}
	}
	this->root = fragments.top()->startNode;
	this->final_state = fragments.top()->endNode;
	fragments.pop();
}

std::unordered_set<Automaton_Node*>
Automaton::compute_epsilon_closure(Automaton_Node* node)
{
	std::unordered_set<Automaton_Node*> new_states =
		std::unordered_set<Automaton_Node*>();

	// add the state itself
	new_states.insert(node);

	// add the states accessible in one epsilon step
	if(node->type1 == 127)
		new_states.insert(node->next1);
	if(node->type2 == 127)
		new_states.insert(node->next2);

	return new_states;
}

std::unordered_set<Automaton_Node*>
Automaton::compute_epsilon_closure(std::unordered_set<Automaton_Node*> states)
{
	std::unordered_set<Automaton_Node*> states_iter_before =
		std::unordered_set<Automaton_Node*>();

	std::unordered_set<Automaton_Node*> states_iter_after =
		std::unordered_set<Automaton_Node*>();

	states_iter_after.insert(states.begin(), states.end());

	std::unordered_set<Automaton_Node*> new_states_partial =
		std::unordered_set<Automaton_Node*>();

	do // saturate the set of states with all possible epsilon steps (0 or more)
	{
		states_iter_before.clear();

		states_iter_before.insert(states_iter_after.begin(),
			states_iter_after.end());

		states_iter_after.clear();

		new_states_partial.clear();

		for(auto it = states_iter_before.begin();
			it != states_iter_before.end(); it++)
		{
			new_states_partial = compute_epsilon_closure(*it);
			states_iter_after.insert(new_states_partial.begin(),
				new_states_partial.end());
		}

		states_iter_after.insert(states_iter_before.begin(),
			states_iter_before.end());
	}
	while(states_iter_before != states_iter_after);

	return states_iter_after;
}

std::unordered_set<Automaton_Node*>
Automaton::compute_one_step_closure(Automaton_Node* node, char label)
{
	std::unordered_set<Automaton_Node*> new_states =
		std::unordered_set<Automaton_Node*>();

	// add the states accessible in one step following label
	if(node->type1 == label)
		new_states.insert(node->next1);
	if(node->type2 == label)
		new_states.insert(node->next2);

	return new_states;
}

std::unordered_set<Automaton_Node*>
Automaton::compute_one_step_closure(std::unordered_set<Automaton_Node*> states,
	char label)
{
	std::unordered_set<Automaton_Node*> new_states =
		std::unordered_set<Automaton_Node*>();

	std::unordered_set<Automaton_Node*> new_states_partial =
		std::unordered_set<Automaton_Node*>();

	/* add all the states accessible in one step following label from the
	current state's set*/
	for(auto it = states.begin(); it != states.end(); it++)
	{
		new_states_partial = compute_one_step_closure(*it, label);
		new_states.insert(new_states_partial.begin(), new_states_partial.end());
	}
	return new_states;
}

bool Automaton::matches_regex(std::list<char>* children)
{
	if(!this->automaton_already_constructed)
	{
		this->build_automaton();
		automaton_already_constructed = true;
	}

	// Check children with automaton
	if(root == nullptr)
		return children->size() == 0;

	// simulate NFA
    std::unordered_set<Automaton_Node*> states =
		compute_epsilon_closure(this->root);
    states = compute_epsilon_closure(states);
    for(auto it = children->begin(); it != children->end(); it++)
	{
		states = compute_one_step_closure(states, *it);
		states = compute_epsilon_closure(states);
	}

	return states.count(final_state) == 1;
}
