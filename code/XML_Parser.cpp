// Author: Paweł Guzewicz

#include "XML_Parser.hpp"
#include <iostream>
#include <fstream>

XML_Parser::XML_Parser(std::string xml_file, XML_Tree* tree)
	:xml_file(xml_file)
	,validity_stack(new std::stack<std::string>())
	,tree(tree)
	,precomputed(false)
	,broken(false)
{
}

XML_Parser::~XML_Parser(void)
{
	delete validity_stack;
}

void XML_Parser::parse_line(std::string line) throw(std::string)
{
	if(line.size() < 3)
		throw std::string("Wrong format of XML file");
	if(line.substr(0, 1) == "0")
	{
		std::string name = line.substr(2);
		this->validity_stack->push(name);
		this->tree->add_child(name[0]);
	}
	else if(line.substr(0, 1) == "1")
		if(this->validity_stack->top() == line.substr(2))
		{
			this->validity_stack->pop();
			try
			{
				this->tree->go_to_parent();
			}
			catch(...)
			{
				throw std::string("Wrong format of XML file");
			}
		}
		else
		{
			throw std::string("XML file is not well-formed");
		}
	else
		throw std::string("Wrong format of XML file");
}

void XML_Parser::parse(void)
{
	this->precomputed = true;

	std::string line;
	bool one_root = true;

	std::ifstream xml_file_stream;
	xml_file_stream.open(this->xml_file, std::ifstream::in);

	while(getline(xml_file_stream, line))
	{
		if(!one_root)
		{
			this->broken = true; /* stack was empty and there is more input
			so we have more then one root = invalid */
			return;
		}
		try
		{
			parse_line(line);
		}
		catch(std::string ex)
		{
			this->broken = true;
			//std::cerr << ex + "\n";
			return;
		}
		if(this->validity_stack->size() == 0)
			one_root = false;
	}

	xml_file_stream.close();
}
