#ifndef XML_PARSER_HPP_INCLUDED
#define XML_PARSER_HPP_INCLUDED

#include <ifstream>
#include <string>
#include <stack>
#include "XML_Tree.hpp"

class XML_Parser
{
	public:
		XML_Parser(std::ifstream xml_file, XML_Tree* tree);
		~XML_Parser(void);

		void parse(void);
		inline bool xml_file_is_well_formed() throw(std::string)
		{
			if(!precomputed)
				throw(std::string("File not parsed"));
			return validity_stack->size() == 0 && !broken;
		}

	private:
		std::ifstream xml_file;
		std::stack<std::string>* validity_stack;
		XML_Tree* tree;
		bool precomputed;
		bool broken;

		void parse_line(std::string line) throw(std::string);
};

#endif // XML_PARSER_HPP_INCLUDED
