#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct CasesBlock {
    char *type;
    char *condition;
    char *code;
    struct CasesBlock *next;
} CasesBlock;

typedef struct IfBlock {
    char *type;       // Tipo de condición (if, else if, elif, else)
    char *condition;  // Condición (NULL para else)
    char *code;       // Bloque de código asociado
    struct IfBlock *next; // Siguiente condición en la lista
} IfBlock;

typedef struct {
    char *type;                // Tipo de estructura (if, else if, elif, else, for, switch, while, foreach, do while)
    char *condition;           // Condición (si la hay)
    char *code;                // Bloque de código asociado
    CasesBlock *cases;         // Lista de casos para switch
    IfBlock *options;          // Lista de posibles condiciones
} ConditionBlock;

typedef struct Node {
    ConditionBlock data;
    struct Node *next;
} Node;

typedef struct {
    Node *top;
} Stack;

void init_stack(Stack *s) {
    s->top = NULL;
}

void push(Stack *s, ConditionBlock block) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->data = block;
    new_node->next = s->top;
    s->top = new_node;
}

char *trim(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    *(end + 1) = 0;
    
    return str;
}

void extract_expression(const char *statement, char *expression, long *pos) {
    for(long x = (*pos); x < strlen(statement); x++){
        char j = statement[x];
        if(j != ' ' && j != '\t'){
            (*pos) = x;
            break;
        }
    }

    printf("Text: %s\n", (statement + (*pos)));

    if(statement[*pos] == '('){
       int c = 1;
       int ps = 0;
       int posx = 0;
       (*pos)++;
       for(long x = (*pos); x < strlen(statement); x++, (*pos)++){
           char u = statement[x];           
           if(u == '('){
              expression[ps] = u;
              ps++;
              c++;
           }else if(u == ')'){
              c--;
              if(c == 0){
                 posx = x;
                 (*pos)++;
                 break;
              }else{
                 expression[ps] = u;
                 ps++;
              }
           }else{
              expression[ps] = u;
              ps++;
           }
       }

       if(c > 0){
          printf("error de sintaxis: fin inesperado\n");
          exit(0);
       }else{
          expression[posx] = '\0';
       }
    }else{
        printf("error de sintaxis(%c).\n", statement[(*pos) - 1]);
        exit(0);
    }
}

void extract_code(const char *statement, char *code, long *pos) {
    for(long x = (*pos); x < strlen(statement); x++){
        char j = statement[x];
        if(j != ' '){
            (*pos) = x;
            break;
        }
    }

    if(statement[*pos] == '{'){
       int c = 1;
       int ps = 0;
       int posx = 0;
       (*pos)++;
       for(long x = (*pos); x < strlen(statement); x++, (*pos)++){
           char u = statement[x];           
           if(u == '{'){
              code[ps] = u;
              ps++;
              c++;
           }else if(u == '}'){
              c--;
              if(c == 0){
                 (*pos)++;
                 posx = x;
                 break;
              }else{
                 code[ps] = u;
                 ps++;
              }
           }else{
              code[ps] = u;
              ps++;
           }
       }

       if(c > 0){
          printf("error de sintaxis in code: fin inesperado\n");
          exit(0);
       }else{
          code[posx] = '\0';
       }
    }else{
        printf("error de sintaxis in code(%c).\n", statement[0]);
        exit(0);
    }
}



void add_case(CasesBlock **cases, char *type, char *condition, char *code) {
    CasesBlock *new_case = (CasesBlock *)malloc(sizeof(CasesBlock));
    new_case->type = strdup(type);
    new_case->condition = condition ? strdup(condition) : NULL;
    new_case->code = strdup(code);
    new_case->next = NULL;

    if (*cases == NULL) {
        *cases = new_case;
    } else {
        CasesBlock *current = *cases;
        while (current->next) {
            current = current->next;
        }
        current->next = new_case;
    }
}

void add_ifblock(IfBlock **options, char *type, char *condition, char *code) {
    IfBlock *new_option = (IfBlock *)malloc(sizeof(IfBlock));
    new_option->type = strdup(type);
    new_option->condition = condition ? strdup(condition) : NULL;
    new_option->code = strdup(code);
    new_option->next = NULL;

    if (*options == NULL) {
        *options = new_option;
    } else {
        IfBlock *current = *options;
        while (current->next) {
            current = current->next;
        }
        current->next = new_option;
    }
}

void identify_structure(const char *statement, Stack *stack, int *if_state, long *pos) {
    ConditionBlock block;
    char expression[1024];
    char code[1024];
    memset(expression, '\0', 1024);
    memset(code, '\0', 1024);

    block.options = NULL;
    block.cases = NULL;

    if (strncmp(statement, "if", 2) == 0) {
        *pos += 2;
        extract_expression(statement, expression, pos);
        extract_code(statement, code, pos);
        add_ifblock(&block.options, "if", expression, code);

        while (1) {
            for (long x = (*pos); x < strlen(statement); x++) {
                char j = statement[x];
                if (j != ' ') {
                    (*pos) = x;
                    break;
                }
            }

            if (strncmp(&statement[*pos], "else if", 7) == 0) {
                (*pos) += 7;
                memset(expression, '\0', 1024);
                memset(code, '\0', 1024);
                extract_expression(statement, expression, pos);
                extract_code(statement, code, pos);
                add_ifblock(&block.options, "else if", expression, code);
            } else if (strncmp(&statement[*pos], "elif", 4) == 0) {
                (*pos) += 4;
                memset(expression, '\0', 1024);
                memset(code, '\0', 1024);
                extract_expression(statement, expression, pos);
                extract_code(statement, code, pos);
                add_ifblock(&block.options, "elif", expression, code);
            } else if (strncmp(&statement[*pos], "else", 4) == 0) {
                (*pos) += 4;
                memset(code, '\0', 1024);
                extract_code(statement, code, pos);
                add_ifblock(&block.options, "else", NULL, code);
            } else {
                break;
            }
        }

        block.type = "if";
        block.condition = NULL;
        block.code = NULL;
        push(stack, block);
    } else if (strncmp(statement, "foreach", 7) == 0) {
        *pos += 7;
        extract_expression(statement, expression, pos);
        extract_code(statement, code, pos);

        block.type = "foreach";
        block.condition = strdup(expression);
        block.code = strdup(code);
        push(stack, block);
    } else if (strncmp(statement, "foreach", 3) == 0) {
        *pos += 3;
        extract_expression(statement, expression, pos);
        extract_code(statement, code, pos);

        block.type = "for";
        block.condition = strdup(expression);
        block.code = strdup(code);
        push(stack, block);
    } else if (strncmp(statement, "switch", 6) == 0) {
        *pos += 6;
        extract_expression(statement, expression, pos);
        block.type = "switch";
        block.condition = strdup(expression);
        block.code = NULL;
        block.cases = NULL;

        const char *switch_body = strchr(statement, '{') + 1;
        const char *case_start = switch_body;

        while (case_start && (case_start = strstr(case_start, "case")) != NULL) {
            const char *case_cond_start = strchr(case_start, ' ');
            const char *case_cond_end = strchr(case_start, ':');
            const char *case_code_start = strchr(case_start, '{');
            const char *case_code_end = strchr(case_start, '}');

            if (case_cond_start && case_cond_end && case_code_start && case_code_end) {
                char case_condition[256];
                char case_code[1024];
                
                strncpy(case_condition, case_cond_start + 1, case_cond_end - case_cond_start - 1);
                case_condition[case_cond_end - case_cond_start - 1] = '\0';

                strncpy(case_code, case_code_start + 1, case_code_end - case_code_start - 1);
                case_code[case_code_end - case_code_start - 1] = '\0';

                add_case(&block.cases, "case", case_condition, case_code);
            }
            case_start = case_code_end;
        }

        const char *default_start = strstr(switch_body, "default");
        if (default_start) {
            const char *default_code_start = strchr(default_start, '{');
            const char *default_code_end = strchr(default_start, '}');

            if (default_code_start && default_code_end) {
                char default_code[1024];
                strncpy(default_code, default_code_start + 1, default_code_end - default_code_start - 1);
                default_code[default_code_end - default_code_start - 1] = '\0';

                add_case(&block.cases, "default", NULL, default_code);
            }
        }

        push(stack, block);
        printf("Estructura: switch, Expresión: %s\n", expression);
    } else if (strncmp(statement, "while", 5) == 0) {
        *pos += 5;
        extract_expression(statement, expression, pos);
        extract_code(statement, code, pos);

        block.type = "while";
        block.condition = strdup(expression);
        block.code = strdup(code);
        push(stack, block);
    } else if (strncmp(statement, "do", 2) == 0) {
        (*pos) += 2;
        const char *while_part = strstr(statement, "while");
        if (while_part) {
            extract_code(statement, code, pos);
            (*pos) = 5;
            extract_expression(while_part, expression, pos);
            block.type = "do while";
            block.condition = strdup(expression);
            block.code = strdup(code);
            block.cases = NULL;
            push(stack, block);
            printf("Estructura: do while, Expresión: %s, Código: %s\n", expression, code);
        }
    } else {
        printf("Estructura no reconocida: %s\n", statement);
    }
}

void print_cases(CasesBlock *cases) {
    while (cases) {
        printf("  Type: %s\n", cases->type);
        if (cases->condition) {
            printf("  Condition: %s\n", cases->condition);
        }
        printf("  Code: %s\n\n", cases->code);
        cases = cases->next;
    }
}

void print_ifblocks(IfBlock *options) {
    while (options) {
        printf("  Type: %s\n", options->type);
        if (options->condition) {
            printf("  Condition: %s\n", options->condition);
        }
        printf("  Code: %s\n\n", options->code);
        options = options->next;
    }
}

void print_stack(Stack *s) {
    Node *current = s->top;
    while (current) {
        printf("Type: %s\n", current->data.type);
        if (current->data.condition) {
            printf("Condition: %s\n", current->data.condition);
        }
        if (current->data.code) {
            printf("Code: %s\n", current->data.code);
        }
        if (current->data.cases) {
            printf("Cases:\n");
            print_cases(current->data.cases);
        }
        if (current->data.options) {
            printf("IfBlocks:\n");
            print_ifblocks(current->data.options);
        }
        printf("\n");
        current = current->next;
    }
}

int main() {
    Stack stack;
    init_stack(&stack);

    int if_state = 0; // Estado para verificar la existencia de un 'if'

    char statement[1024];
    long pos = 0;

    // Ejemplo de uso
    strcpy(statement, "if (12 < 23) { printf(\"Hello World\"); } else if (394 > 12) { printf(\"Hello Again\"); } elif (20 > 12) { printf(\"Hello Once More\"); } else { printf(\"Hello Else\"); }");
    identify_structure(statement, &stack, &if_state, &pos);

    pos = 0;
    strcpy(statement, "if(60 < 90){ printf(\"Test if 0\"); }");
    identify_structure(statement, &stack, &if_state, &pos);

    pos = 0;
    strcpy(statement, "if(60 < 90){ printf(\"Test if 3\"); }else{printf(\"ALALALA\")}");
    identify_structure(statement, &stack, &if_state, &pos);

    pos = 0;
    strcpy(statement, "for (int i = 0; i < 10; i++) { printf(\"Loop\"); }");
    identify_structure(statement, &stack, &if_state, &pos);

    pos = 0;
    strcpy(statement, "switch (x) { case 1: { printf(\"Case 1\"); break; } case 2: { printf(\"Case 2\"); break; } default: { printf(\"Default\"); break; } }");
    identify_structure(statement, &stack, &if_state, &pos);

    pos = 0;
    strcpy(statement, "while (true) { printf(\"While Loop\"); }");
    identify_structure(statement, &stack, &if_state, &pos);

    pos = 0;
    strcpy(statement, "do { printf(\"Do Loop\"); } while (false);");
    identify_structure(statement, &stack, &if_state, &pos);

    pos = 0;
    strcpy(statement, "foreach (var item in collection) { printf(\"Item\"); }");
    identify_structure(statement, &stack, &if_state, &pos);
    
    print_stack(&stack);

    return 0;
}