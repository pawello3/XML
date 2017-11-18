#include <iostream>
#include "XML_Parser.hpp"
#include "XML_Tree.hpp"

using namespace std;

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cout << "Usage: XML_Validator <xml_file> <dtd_file>";
		return 0;
	}

	XML_Tree* tree = new XML_Tree();
	XML_Parser* parser = new XML_Parser(argv[1], tree);

	parser->parse();
	bool is_well_formed = true;
	try
	{
		is_well_formed = parser->xml_file_is_well_formed();
	}
	catch(std::string ex)
	{
		std::cerr << ex + "\n";
	}

	if(is_well_formed)
	{
		std::cout << "well-formed\n";

		tree->declare_automata(argv[2]);

		if(tree->is_valid())
			std::cout << "valid";
		else
			std::cout << "not valid";
	}
	else
		std::cout << "not well-formed\nnot valid";

	delete tree;
	delete parser;

	return 0;
}
