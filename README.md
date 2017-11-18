## Author: Paweł Guzewicz

In order to evaluate the assignment, you may follow this instruction. In the report file I describe the data structure used for in my implementation, along with all assumptions made for the specification of the program. I check the performance using test files (I don't attach them due to the big size of the files) and later discuss the obtained results.

(Linux environment is assumed)

### 1. Comipilation
	$ cd <directory_with_unpacked_archive>
	$ make

### 2. Running the program
	$ ./bin/XML_Validator <xml_file> <dtd_file>

### 1 & 2* (Optionally)
	You can use Code::Blocks IDE to open the project: file XML.cbp and then press F9 to build and run.

### 3. Testing
	$ jupyter notebook
	Open the file testing.ipynb, read the notebook with cells already evaluated. Bash commands are invoked in the Python code in order to run the compiled program (step 1 is essensial).

### 4. Report
	Open the file report.pdf.

#### At any time you may want to clean up the working directory. You can use predefined rules for make.
	$ make clean
	$ make distclean