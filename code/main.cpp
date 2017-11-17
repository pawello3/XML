#include <iostream>
#include <fstream>
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

	std::ifstream xml_file;
	xml_file.open(argv[1]);

	XML_Tree* tree = new XML_Tree();
	XML_Parser* parser = new XML_Parser(xml_file, tree);

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

		std::ifstream dtd_file;
		dtd_file.open(argv[2]);
		tree->declare_automata(dtd_file);

		if(tree->is_valid())
			std::cout << "valid\n";
		else
			std::cout << "not valid\n";

		dtd_file.close();
	}
	else
		std::cout << "not well-formed\nnot valid\n";

	delete tree;
	delete parser;
	xml_file.close();

	return 0;
}
