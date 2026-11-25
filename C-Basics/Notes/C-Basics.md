1. Every C program consists of a body and a main function.
   
2. Before the body arrives the the header files usually called through preprocessor directive called #include . 
   This directive is basically there to make your compiler aware of header files which may have function blueprints being accessed from that particular header file and being used withint the body of the code.

3. In the file "hello.c" (can be found in the Code directory of this section i.e. ../Code), the library or header file being imported is the "stdio.h" (stands for -> "Standard input & output"). We use the printf function from the header file to display "Hello World!" followed by a new line character before exiting the program.
   
4. The main function in hello.c is the main entry point into the programs execution. A function has three seperate parts in it:
   1. **Return type**: This suggests the user when the function execution ends what is the type of the value/data that is returned. For main function, it always is an int type.

   2. **Function Name**: 

   3. **Statements**: These are lines of code written to be executed once the program execution begins. They can include variable initializationn, functions, variable updation and deletion, logic blocks (i.e. if, elseif, else, etc.) and printf/printing functions.

   3. 
