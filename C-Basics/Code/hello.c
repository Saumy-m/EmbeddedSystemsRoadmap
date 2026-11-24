// calling header files for imports

/*
    #include -> a preprocessor directive to let the compiler know the library referenced 
    in the code below pre-compilation.
*/
#include<stdio.h> // calls in-built input output functions

 
/*
    int main() -> the main function is the entry point of the program, where the execution starts.
    The return type of main is int, which means it returns an integer value to the operating system upon completion.
*/
int main(){ // int is the return, main is the function name, anything in brackets are function parameters

    // in-built function to display the output on the terminal screen (default value is of type int or string)
    printf("Hello World!\n"); // displays the string "Hello World!" in the terminal and \n moves the cursor to the next line after printing
    return 0;
}