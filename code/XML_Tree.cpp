// Author: Paweł Guzewicz

#include "XML_Tree.hpp"
#include <fstream>
#include <cassert>
#include <iostream>
#include <sstream>
#include <utility>


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
