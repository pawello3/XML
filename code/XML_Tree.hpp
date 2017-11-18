// Author: Paweł Guzewicz

#ifndef XML_TREE_HPP_INCLUDED
#define XML_TREE_HPP_INCLUDED

#include <list>
#include <unordered_map>
#include "XML_DTD_Validator.hpp"

class Tree_Node
{
	public:
		Tree_Node(char name, Tree_Node* parent);
		~Tree_Node(void);

		inline char get_name(void)
		{
			return this->name;
		}
		inline Tree_Node* get_parent(void)
		{
			return this->parent;
		}
		inline std::list<Tree_Node*>* get_children(void)
		{
			return this->children;
		}
		Tree_Node* add_child(char name);
		bool is_valid(std::unordered_map<char, Automaton*>* automata);

	private:
		char name;
		Tree_Node* parent;
		std::list<Tree_Node*>* children;
};

class XML_Tree
{
	public:
		XML_Tree(void);
		~XML_Tree(void);

		inline void add_child(char name) throw(std::string)
		{
			if(this->context == nullptr)
				throw std::string("Wrong context");
			this->context = this->context->add_child(name);
		}
		inline void go_to_parent(void) throw(std::string)
		{
			if(this->context == nullptr)
				throw std::string("Wrong context");
			this->context = this->context->get_parent();
		}
		void declare_automata(std::string dtd_file);
		bool is_valid(void) throw(std::string);

	private:
		std::unordered_map<char, Automaton*>* automata;
		Tree_Node* root;
		Tree_Node* context;
};

#endif // XML_TREE_HPP_INCLUDED
