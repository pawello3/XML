#include "XML_Tree.hpp"
#include <fstream>
#include <cassert>
#include <iostream>
#include <sstream>
#include <utility>
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

std::unordered_set<Automaton_Node*> Automaton::compute_epsilon_closure(Automaton_Node* node)
{
	std::unordered_set<Automaton_Node*> new_states = std::unordered_set<Automaton_Node*>();
	if(node->type1 == 127)
		new_states.insert(node->next1);
	if(node->type2 == 127)
		new_states.insert(node->next2);
	new_states.insert(node);
	return new_states;
}

std::unordered_set<Automaton_Node*> Automaton::compute_epsilon_closure(std::unordered_set<Automaton_Node*> states)
{
	std::unordered_set<Automaton_Node*> states_iter_before = std::unordered_set<Automaton_Node*>();
	std::unordered_set<Automaton_Node*> states_iter_after = std::unordered_set<Automaton_Node*>();
	states_iter_after.insert(states.begin(), states.end());
	std::unordered_set<Automaton_Node*> new_states_partial = std::unordered_set<Automaton_Node*>();

	do // saturate the set of states by doing all possible epsilon steps (0 or more)
	{
		states_iter_before.clear();
		states_iter_before.insert(states_iter_after.begin(), states_iter_after.end());
		states_iter_after.clear();
		new_states_partial.clear();
		for(auto it = states_iter_before.begin(); it != states_iter_before.end(); it++)
		{
			new_states_partial = compute_epsilon_closure(*it);
			states_iter_after.insert(new_states_partial.begin(), new_states_partial.end());
		}
		states_iter_after.insert(states_iter_before.begin(), states_iter_before.end());
	}
	while(states_iter_before != states_iter_after);

	return states_iter_after;
}

std::unordered_set<Automaton_Node*> Automaton::compute_one_step_closure(Automaton_Node* node, char step)
{
	std::unordered_set<Automaton_Node*> new_states = std::unordered_set<Automaton_Node*>();
	if(node->type1 == step)
		new_states.insert(node->next1);
	if(node->type2 == step)
		new_states.insert(node->next2);
	return new_states;
}

std::unordered_set<Automaton_Node*> Automaton::compute_one_step_closure(std::unordered_set<Automaton_Node*> states, char step)
{
	std::unordered_set<Automaton_Node*> new_states = std::unordered_set<Automaton_Node*>();
	std::unordered_set<Automaton_Node*> new_states_partial = std::unordered_set<Automaton_Node*>();
	for(auto it = states.begin(); it != states.end(); it++)
	{
		new_states_partial = compute_one_step_closure(*it, step);
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

	/*std::cout << "Root" << std::endl;
	std::cout << root->type1 << " " << root->next1 << std::endl;
	std::cout << root->type2 << " " << root->next2 << std::endl;*/

	// simulate NFA
    std::unordered_set<Automaton_Node*> states = compute_epsilon_closure(this->root);
    states = compute_epsilon_closure(states);
    /*for(auto it = states.begin(); it != states.end(); it++)
	{
		std::cout << "\tL0\n";
		std::cout << "\t" << (*it)->type1 << " " << (*it)->next1 << std::endl;
		std::cout << "\t" << (*it)->type2 << " " << (*it)->next2 << std::endl;
	}*/

    for(auto it = children->begin(); it != children->end(); it++)
	{
		//std::cout << "\t\tL1 " << (*it) << std::endl;
		states = compute_one_step_closure(states, *it);
		/*for(auto it = states.begin(); it != states.end(); it++)
		{
			std::cout << "\t\t\tL2\n";
			std::cout << "\t\t\t" << (*it)->type1 << " " << (*it)->next1 << std::endl;
			std::cout << "\t\t\t" << (*it)->type2 << " " << (*it)->next2 << std::endl;
		}*/
		states = compute_epsilon_closure(states);
		/*for(auto it = states.begin(); it != states.end(); it++)
		{
			std::cout << "\t\t\tL3\n";
			std::cout << "\t\t\t" << (*it)->type1 << " " << (*it)->next1 << std::endl;
			std::cout << "\t\t\t" << (*it)->type2 << " " << (*it)->next2 << std::endl;
		}*/
	}

	return states.count(final_state) == 1;
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
    std::string postfix_regex = convert_to_reverse_Polish_notation(this->regular_expression);
    if(postfix_regex == "_")
		return;

    std::stack<Automaton_Fragment*> fragments = std::stack<Automaton_Fragment*>();

    // char = 0 undefined
    // char = 127 epsilon transition

	Automaton_Node* left = nullptr;
	Automaton_Node* right = nullptr;
	Automaton_Fragment* fragment_top = nullptr;
	Automaton_Fragment* fragment_top1 = nullptr;
	Automaton_Fragment* fragment_top2 = nullptr;

	//std::cout << postfix_regex << std::endl;

	for(unsigned int i = 0; i < postfix_regex.size(); i++)
	{
		switch(postfix_regex[i])
		{
			case '.':
				//std::cout << ".\n";
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
				//std::cout << "?\n";
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
				//std::cout << "+\n";
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
				//std::cout << "*\n";
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
				//std::cout << "letter\n";
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

////////////////////////////////////////////////////////////////////////////////

Tree_Node::Tree_Node(char name, Tree_Node* parent)
	:name(name)
	,parent(parent)
	,children(new std::list<Tree_Node*>())
{
}

Tree_Node::~Tree_Node(void)
{
	for(auto it = this->children->begin(); it != this->children->end(); it++)
		delete *it;
	delete this->children;
}

Tree_Node* Tree_Node::add_child(char name)
{
	Tree_Node* new_child = new Tree_Node(name, this);
	this->children->push_back(new_child);
	return new_child;
}

bool Tree_Node::is_valid(std::unordered_map<char, Automaton*>* automata)
{
	// check if the node is valid
	std::list<char>* children_names = new std::list<char>();
	for(auto it = this->children->begin(); it != this->children->end(); it++)
		children_names->push_back((*it)->get_name());
	bool is_valid = (*automata)[this->name]->matches_regex(children_names);
	delete children_names;
	if(!is_valid)
		return false;

	// check if the child nodes are valid
	for(auto it = this->children->begin(); it != this->children->end(); it++)
		if(!(*it)->is_valid(automata))
			return false;

	// subtree rooted in this is valid
	return true;
}

////////////////////////////////////////////////////////////////////////////////

XML_Tree::XML_Tree(void)
	:automata(new std::unordered_map<char, Automaton*>())
	,root(new Tree_Node('\0', nullptr))
{
	this->context = this->root;
}

XML_Tree::~XML_Tree(void)
{
	for(auto it = this->automata->begin(); it != this->automata->end(); it++)
		delete it->second;
	delete this->automata;
	delete root;
}

void XML_Tree::declare_automata(std::string dtd_file)
{
	std::string line;
	std::ifstream dtd_file_stream;
	dtd_file_stream.open(dtd_file, std::ifstream::in);

	while(getline(dtd_file_stream, line))
	{
		std::istringstream iss(line);
		char name;
		std::string name_str, regular_expression;
		std::getline(iss, name_str, ' '); // match until separator
		name = name_str[0];
		std::getline(iss, regular_expression); // match the rest of the line
		Automaton* automaton = new Automaton(regular_expression);
		this->automata->insert(std::pair<char, Automaton*>(name, automaton));
	}

	dtd_file_stream.close();
}

bool XML_Tree::is_valid(void) throw(std::string)
{
	assert(this->root == this->context); /* we had to come back to root while
	building the tree from the XML file */
	return this->root->get_children()->front()->is_valid(this->automata);
}
