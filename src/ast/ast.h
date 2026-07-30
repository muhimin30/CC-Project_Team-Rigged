#ifndef AST_H
#define AST_H

    typedef enum {
        ND_PROGRAM,     
        ND_BLOCK,       
        ND_DECL,        
        ND_ASSIGN,      
        ND_IF,          
        ND_WHILE,      
        ND_PRINT,       
        ND_BINOP,       
        ND_UNOP,        
        ND_ID,          
        ND_INT_LIT,    
        ND_FLOAT_LIT,   
        ND_BOOL_LIT     
} NodeType;

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_VOID,     
    TYPE_UNKNOWN    
} DataType;

typedef struct Node {
    NodeType type;
    int line;
    DataType data_type;  
    char *decl_type_name; 
    char *name;           
    char *op;              

    char *resolved_name;

    int    int_val;    
    double float_val;  
    int    bool_val;   

    struct Node *left;   
    struct Node *right;  
    struct Node *third;          

    
    struct Node **stmts;
    int stmt_count;
    int stmt_cap;
} Node;

Node *new_node(NodeType type, int line);
void  add_stmt(Node *block, Node *stmt);

void  print_ast(Node *n, int depth);
const char *datatype_str(DataType t);
const char *nodetype_str(NodeType t);

#endif