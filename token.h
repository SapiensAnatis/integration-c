#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED // Include guards: block the same header from being included twice in a file


// --- Type declarations ---

enum Token_Type {
    Operator, 
    Number,
    Variable,
    Bracket_Left, // having a separate enum for bracket and (left, right) seems a bit silly to me
    Bracket_Right,
    Function
};

enum Operator_Type {
    Op_Add,
    Op_Subtract,
    Op_Multiply,
    Op_Divide,
    Op_Power
};

enum Function_Type {
    Func_Sin,
    Func_Cos,
    Func_Tan,
    Func_Ln,
    Func_Exp,
    Func_Log // log10
};


enum Associativity {
    Assoc_Left,
    Assoc_Right
};


// The reason why there are both Function and Operator fields, as well as
// Function_Type/Operator_Type is so that when it comes time to evaluate the tokens and do stuff, 
// with them, it is possible to both examine "is the token a function" and 
// "what function is the token". If we only stored function types, to check the former would  
// require `if token.Function_Type != null` which is less readable

struct Token {
    enum Token_Type type;
    // Number-exclusive property, but as Stack is monotype all tokens must have these properties
    // They will be null/uninitialized for token types that don't use them, though
    double value; 
    // Operator-exclusive properties
    enum Operator_Type operator_type;
    int precedence;
    enum Associativity associativity;
    // Function-exclusive properties
    enum Function_Type function_type;
};

// Functions

double get_token_value(struct Token *token, double x);
void print_token(struct Token *token);
void print_tokenized(struct Token *token, int num_tokens);

#endif