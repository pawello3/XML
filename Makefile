COMPILER=g++
ERRORSFLAGS=-Wall -Wextra -Werror -O2
STANDARDFLAGS=-std=c++11
LIBRARIES=-lm
CODE_DIRECTORY=code
_DEPENDENCIES=XML_Parser.hpp XML_Tree.hpp XML_DTD_Validator.hpp
DEPENDENCIES=$(patsubst %,$(CODE_DIRECTORY)/%,$(_DEPENDENCIES))
_SOURCES=main.cpp XML_Parser.cpp XML_Tree.cpp XML_DTD_Validator.cpp
SOURCES=$(patsubst %,$(CODE_DIRECTORY)/%,$(_SOURCES))
OBJECTS_DIRECTORY=objects
_OBJECTS=$(_SOURCES:.cpp=.o)
OBJECTS=$(patsubst %,$(OBJECTS_DIRECTORY)/%,$(_OBJECTS))
BINARY_FILE_DIRECTORY=bin
EXECUTABLE=$(BINARY_FILE_DIRECTORY)/XML_Validator
ZIP_FILE=Pawel_Guzewicz_Project.zip

all: cpp

cpp: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	if [ ! -d $(BINARY_FILE_DIRECTORY) ]; then mkdir $(BINARY_FILE_DIRECTORY); fi;\
	$(COMPILER) $(STANDARDFLAGS) $(ERRORSFLAGS) $^ -o $@ $(LIBRARIES)

$(OBJECTS_DIRECTORY)/%.o: $(CODE_DIRECTORY)/%.cpp $(DEPENDENCIES)
	if [ ! -d $(OBJECTS_DIRECTORY) ]; then mkdir $(OBJECTS_DIRECTORY); fi;\
	$(COMPILER) $(STANDARDFLAGS) $(ERRORSFLAGS) -c $< -o $@ $(LIBRARIES)

.PHONY: clean distclean

zip: clean
	zip -r $(ZIP_FILE) $(CODE_DIRECTORY) Makefile README.txt

clean:
	find . -name "*.o" -type f -exec rm '{}' \;
	find . -name "*~" -type f -exec rm '{}' \;
	find . -name "*.save" -type f -exec rm '{}' \;
	find . -name "*.log" -type f -exec rm '{}' \;
	find . -name "*.dat" -type f -exec rm '{}' \;
	- rmdir $(OBJECTS_DIRECTORY)
	- rm $(EXECUTABLE)
	- rmdir $(BINARY_FILE_DIRECTORY)

distclean: clean
	- rm $(ZIP_FILE)
