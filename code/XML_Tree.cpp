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
	return new_states;
}

std::unordered_set<Automaton_Node*> Automaton::compute_epsilon_closure(std::unordered_set<Automaton_Node*> states)
{
	std::unordered_set<Automaton_Node*> new_states = std::unordered_set<Automaton_Node*>();
	std::unordered_set<Automaton_Node*> new_states_partial = std::unordered_set<Automaton_Node*>();
	for(auto it = states.begin(); it != states.end(); it++)
	{
		new_states_partial = compute_epsilon_closure(*it);
		new_states.insert(new_states_partial.begin(), new_states_partial.end());
	}
	return new_states;
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

	// simulate NFA
    std::unordered_set<Automaton_Node*> states = compute_epsilon_closure(root);

    for(auto it = children->begin(); it != children->end(); it++)
	{
		states = compute_one_step_closure(states, *it);
		states = compute_epsilon_closure(states);
	}

	return states.count(final_state) == 1;
}

std::string Automaton::convert_to_reverse_Polish_notation(std::string input)
{
	unsigned int input_size = input.size();
	if(input_size == 1 && input[0] == '_')
		return input;

	input += "\0"; // easy look-ahead for introducing explicit concatenation
	std::stack<char> aux_stack = std::stack<char>();
	std::string output = "";

	for(unsigned int i = 0; i < input_size; i++)
	{
		switch(input[i])
		{
			case '?':
			case '+':
			case '*':
			case '(':
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
			default:
				output += input[i];
				switch(input[i + 1])
				{
					case '?':
					case '+':
					case '*':
					case '(':
					case ')':
					case '\0':
						break;
					default:
						bool stop = true;
						while(stop && !aux_stack.empty())
						{
							switch(aux_stack.top())
							{
								case '?':
								case '+':
								case '*':
									output += aux_stack.top();
									aux_stack.pop();
								default:
									stop = false;
							}
						}
						aux_stack.push('.'); // concatenation
						break;
				}
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
				delete fragment_top2;
				fragment_top1->endNode = fragment_top2->endNode;
				fragments.push(fragment_top1);
				break;
			case '?':
				fragment_top = fragments.top();
				fragments.pop();
				left = new Automaton_Node();
				left->next1 = fragment_top->startNode;
				left->next2 = fragment_top->endNode;
				left->type1 = 127;
				left->type2 = 127;
				fragment_top->startNode = left;
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
