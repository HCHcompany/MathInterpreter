#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

static double PRECISION = 0.00000000000001;
static int MAX_NUMBER_STRING_SIZE = 4096; //32

// Definición de tokens
typedef enum {
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MOD,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_XOR,
    TOKEN_SHL,
    TOKEN_SHR,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,
    TOKEN_QUESTION,
    TOKEN_COLON,
    TOKEN_END,
    TOKEN_INVALID
} TokenType;

// Estructura de token
typedef struct {
    TokenType type;
    union {
        double number_value;
        char *string_value;
    };
} Token;

// Estructura del lexer
typedef struct {
    const char* text;
    size_t pos;
    bool string_status;
    Token current_token;
} Lexer;

// Funciones de ayuda para el lexer
Token get_next_token(Lexer* lexer);
Token get_number_token(Lexer* lexer);
Token get_string_token(Lexer* lexer);

void advance(Lexer* lexer);
Token peek(Lexer* lexer);
Token parse_expression(Lexer* lexer);
Token parse_term(Lexer* lexer);
Token parse_factor(Lexer* lexer);
Token parse_primary(Lexer* lexer);
Token parse_unary(Lexer* lexer);
Token parse_tenary(Lexer* lexer);
char* parse_expression_string(Lexer* lexer);

// Función de evaluación
Token evaluate(const char* expression);

// Función para redondear el resultado
double round_result(double value, int precision);

// Implementaciones propias de funciones matemáticas
double my_fmod(double x, double y);
double my_pow(double base, int exp);
double my_abs(double value);
double my_atof(const char* str);
double my_round(double value);
char *dtoa(char *s, double n);

// Función principal
int main() {
    char expression[256];

    printf("Ingrese una expresión matemática (o 'salir' para terminar):\n");
    while (1) {
        printf("> ");
        if (!fgets(expression, sizeof(expression), stdin)) {
            break;
        }
        if (strncmp(expression, "salir", 5) == 0) {
            break;
        }

        Token result = evaluate(expression);
        if(result.type == TOKEN_STRING){
            printf("Resultado: %s\n", result.string_value);
        }else{
           if(fabs(result.number_value - (int)result.number_value) < 1e-10) {
               printf("Resultado: %.0f\n", result.number_value);
           }else{
               printf("Resultado: %.14f\n", result.number_value);
           }
        }
    }

    return 0;
}

// Implementación del lexer
Token get_next_token(Lexer* lexer) {
    while (lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])) {
        lexer->pos++;
    }

    if (lexer->text[lexer->pos] == '\0') {
        Token token = { TOKEN_END };
        lexer->current_token = token;
        return token;
    }

    if (isdigit(lexer->text[lexer->pos]) || lexer->text[lexer->pos] == '.') {
        return get_number_token(lexer);
    }

    if(lexer->text[lexer->pos] == '\''){
        Token n = get_string_token(lexer);
        if(strlen(n.string_value) < 3 && strlen(n.string_value) > 0){
           if(strlen(n.string_value) == 2){
              if(n.string_value[0] == '\\'){
                 Token token = { TOKEN_NUMBER };
                 token.number_value = (double)(int)n.string_value[1];
                 lexer->current_token = token;
                 lexer->string_status = false;
                 free(n.string_value);
                 return token;
              }else{
                printf("Error de declaracion de caracter: %s\n", n.string_value);
                exit(0);     
              }
           }else{
              Token token = { TOKEN_NUMBER };
              token.number_value = (double)(int)n.string_value[0];
              lexer->current_token = token;
              lexer->string_status = false;
              free(n.string_value);
              return token;
           }
        }else{
           printf("Error de declaracion de caracter.\n");
           exit(0);
        }
    }else if(lexer->text[lexer->pos] == '"') {
        lexer->string_status = true;
        return get_string_token(lexer);
    }

    char current = lexer->text[lexer->pos++];
    Token token = { TOKEN_INVALID };

    switch (current) {
        case '+': token.type = TOKEN_PLUS; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '*': token.type = TOKEN_MULTIPLY; break;
        case '/': token.type = TOKEN_DIVIDE; break;
        case '%': token.type = TOKEN_MOD; break;
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '<':
            if (lexer->text[lexer->pos] == '=') { lexer->pos++; token.type = TOKEN_LE; }
            else if (lexer->text[lexer->pos] == '<') { lexer->pos++; token.type = TOKEN_SHL; } 
            else { token.type = TOKEN_LT; }
            break;
        case '>':
            if (lexer->text[lexer->pos] == '=') { lexer->pos++; token.type = TOKEN_GE; }
            else if (lexer->text[lexer->pos] == '>') { lexer->pos++; token.type = TOKEN_SHR; }  
            else { token.type = TOKEN_GT; }
            break;
        case '=':
            if (lexer->text[lexer->pos] == '=') { lexer->pos++; token.type = TOKEN_EQ; }
            else { token.type = TOKEN_INVALID; }
            break;
        case '!':
            if (lexer->text[lexer->pos] == '=') { lexer->pos++; token.type = TOKEN_NE; }
            else { token.type = TOKEN_NOT; }
            break;
        case '&':
            if (lexer->text[lexer->pos] == '&') {
                lexer->pos++;
                token.type = TOKEN_LOGICAL_AND;
            } else {
                token.type = TOKEN_AND; 
            }
            break;
        case '|':
            if (lexer->text[lexer->pos] == '|') {
                lexer->pos++;
                token.type = TOKEN_LOGICAL_OR;
            } else {
                token.type = TOKEN_OR;
            }
            break;
        case '^': token.type = TOKEN_XOR; break;
        case '?': token.type = TOKEN_QUESTION; break;
        case ':': token.type = TOKEN_COLON; break;
        default: token.type = TOKEN_INVALID; break;
    }

    lexer->current_token = token;
    return token;
}

Token get_number_token(Lexer* lexer) {
    char buffer[64];
    size_t length = 0;

    while (isdigit(lexer->text[lexer->pos]) || lexer->text[lexer->pos] == '.') {
        buffer[length++] = lexer->text[lexer->pos++];
    }
    buffer[length] = '\0';

    Token token = { TOKEN_NUMBER };
    token.number_value = my_atof(buffer);

    lexer->current_token = token;
    return token;
}

Token get_string_token(Lexer* lexer) {
    char quote = lexer->text[lexer->pos++];
    char buffer[256];
    size_t length = 0;
    
    while (lexer->text[lexer->pos] != '\0' && lexer->text[lexer->pos] != quote) {
        buffer[length++] = lexer->text[lexer->pos++];
    }
    
    if (lexer->text[lexer->pos] != quote) {
        fprintf(stderr, "Error: falta comilla de cierre\n");
        exit(EXIT_FAILURE);
    }

    buffer[length] = '\0';
    lexer->pos++; // Consume la comilla de cierre
    Token token = { TOKEN_STRING };
    token.string_value = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    memset(token.string_value, '\0', sizeof(char) * (strlen(buffer) + 1));
    strcpy(token.string_value, buffer);
    lexer->current_token = token;
    return token;
}

// Implementación del parser
char* parse_primary_string(Lexer* lexer) {
    Token token = lexer->current_token;
    if (token.type == TOKEN_STRING) {
        get_next_token(lexer);
        char* result = (char*)malloc(strlen(token.string_value) + 1);
        strcpy(result, token.string_value);
        return result;
    } else if (token.type == TOKEN_LPAREN) {
        get_next_token(lexer); // Consume '('
        char* result = parse_expression_string(lexer);
        if (lexer->current_token.type != TOKEN_RPAREN) {
            fprintf(stderr, "Error: falta ')' en la expresión\n");
            exit(EXIT_FAILURE);
        }
        get_next_token(lexer); // Consume ')'
        return result;
    } else {
        fprintf(stderr, "Error: token inesperado en la expresión\n");
        exit(EXIT_FAILURE);
    }
}

Token parse_primary(Lexer* lexer) {
    Token token = lexer->current_token;
    if (token.type == TOKEN_NUMBER) {
        get_next_token(lexer);
        return token;
    } else if (token.type == TOKEN_STRING){
        char* str_value = parse_primary_string(lexer);
        token.string_value = str_value;
        return token;
    } else if (token.type == TOKEN_XOR || token.type == TOKEN_SHR || token.type == TOKEN_SHL || 
               token.type == TOKEN_AND || token.type == TOKEN_OR){
        get_next_token(lexer);
        return token;
    } else if (token.type == TOKEN_LPAREN) {
        get_next_token(lexer); // Consume '('
        Token result = parse_expression(lexer);
        if (lexer->current_token.type != TOKEN_RPAREN) {
            fprintf(stderr, "Error: falta ')' en la expresión\n");
            exit(EXIT_FAILURE);
        }
        get_next_token(lexer); // Consume ')'
        return result;
    } else{
        fprintf(stderr, "(1) Error: token inesperado en la expresión: %d\n", token.type);
        exit(EXIT_FAILURE);
    }
}

Token parse_unary(Lexer* lexer) {
    Token token = lexer->current_token;
    if (token.type == TOKEN_MINUS || token.type == TOKEN_PLUS || token.type == TOKEN_NOT) {
        get_next_token(lexer);
        Token operand = parse_unary(lexer);
        if (token.type == TOKEN_MINUS) {
            operand.number_value = -operand.number_value;
        } else if (token.type == TOKEN_PLUS) {
            // No cambia el valor
        } else if (token.type == TOKEN_NOT) {
            operand.number_value = !operand.number_value;
        }
        return operand;
    }
    return parse_primary(lexer);
}

Token parse_factor(Lexer* lexer) {
    Token left = parse_unary(lexer);
    while (lexer->current_token.type == TOKEN_MULTIPLY || lexer->current_token.type == TOKEN_DIVIDE || lexer->current_token.type == TOKEN_MOD) {
        TokenType operator = lexer->current_token.type;
        get_next_token(lexer);
        Token right = parse_unary(lexer);
        if (operator == TOKEN_MULTIPLY) {
            left.number_value *= right.number_value;
        } else if (operator == TOKEN_DIVIDE) {
            left.number_value /= right.number_value;
        } else if (operator == TOKEN_MOD) {
            left.number_value = my_fmod(left.number_value, right.number_value);
        }
    }
    return left;
}

Token parse_term(Lexer* lexer) {
    Token left = parse_factor(lexer);
    while (lexer->current_token.type == TOKEN_PLUS || lexer->current_token.type == TOKEN_MINUS) {
        TokenType operator = lexer->current_token.type;
        get_next_token(lexer);
        Token right = parse_factor(lexer);
        if (operator == TOKEN_PLUS){
            if(left.type == TOKEN_STRING){
               if(right.type == TOKEN_NUMBER){
                  char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) + 4097));
                  memset(strn, '\0', sizeof(char) * (strlen(left.string_value) + 4097));
                  strcpy(strn, left.string_value);
                  char *n = (char *)malloc(sizeof(char) * 4096);
                  memset(n, '\0', sizeof(char) * 4096);
                  n = dtoa(n, right.number_value);
                  strcat(strn, n);
                  free(n);
                  lexer->string_status = true;
                  left.type = TOKEN_STRING;
                  free(left.string_value);
                  left.string_value = NULL;
                  left.string_value = strn;
               }else{
                  char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) + strlen(right.string_value) + 1));
                  memset(strn, '\0', sizeof(char) * (strlen(left.string_value) + strlen(right.string_value) + 1));
                  strcpy(strn, left.string_value);
                  strcat(strn, right.string_value);
                  free(left.string_value);
                  free(right.string_value);
                  left.string_value = NULL;
                  right.string_value = NULL;
                  left.type = TOKEN_STRING;
                  left.string_value = strn;
                  lexer->string_status = true;
               }
            }else{
                if(right.type == TOKEN_NUMBER){
                   left.number_value += right.number_value;
                   left.type = TOKEN_NUMBER;
                   lexer->string_status = false;
                }else{
                   char *strn = (char *)malloc(sizeof(char) * (strlen(right.string_value) + 4097));
                   memset(strn, '\0', sizeof(char) * (strlen(right.string_value) + 4097));
                   char *n = (char *)malloc(sizeof(char) * 4096);
                   memset(n, '\0', sizeof(char) * 4096);
                   n = dtoa(n, left.number_value);
                   strcpy(strn, n);
                   free(n);
                   strcat(strn, right.string_value);
                   free(right.string_value);
                   left.type = TOKEN_STRING;
                   left.string_value = strn;
                   lexer->string_status = true;
                }
            }
        }else if(operator == TOKEN_MINUS){
            if(left.type == TOKEN_STRING){
               if(right.type == TOKEN_NUMBER){
                  char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) - ((int)right.number_value) + 1));
                  memset(strn, '\0', sizeof(char) * (strlen(left.string_value) - ((int)right.number_value) + 1));
                  for(int x = 0; x < (strlen(left.string_value) - (right.number_value)); x++){
                      strn[x] = left.string_value[x];
                  }
                  free(left.string_value);
                  left.string_value = NULL;
                  left.string_value = strn;
                  lexer->string_status = true;
                  left.type = TOKEN_STRING;
               }else{
                  if(strlen(left.string_value) >= strlen(right.string_value)){
                     bool status_rest = true;
                     for(int x = (strlen(right.string_value) - 1), y = (strlen(left.string_value) - 1); x >= 0; x--, y--){
                         char c0 = left.string_value[y];
                         char c1 = right.string_value[x];
                         if(c0 != c1){
                            status_rest = false;
                            break;
                         }
                     }

                     if(status_rest){
                        char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) - strlen(right.string_value) + 1));
                        memset(strn, '\0', sizeof(char) * (strlen(left.string_value) - strlen(right.string_value) + 1));
                        for(int x = 0; x < (strlen(left.string_value) - strlen(right.string_value)); x++){
                            strn[x] = left.string_value[x];
                        }
                        free(right.string_value);
                        right.string_value = NULL;
                        free(left.string_value);
                        left.string_value = NULL;
                        lexer->string_status = true;
                        left.type = TOKEN_STRING;
                        left.string_value = strn;
                     }else{
                        free(right.string_value);
                        right.string_value = NULL;
                        left.type = TOKEN_STRING;
                        lexer->string_status = true;   
                     }
                  }else{
                     free(right.string_value);
                     right.string_value = NULL;
                     left.type = TOKEN_STRING;
                     lexer->string_status = true;
                  }
               }
            }else{
               if(right.type == TOKEN_NUMBER){
                  left.number_value -= right.number_value;
                  left.type = TOKEN_NUMBER;
                  lexer->string_status = false;
               }else{
                  left.number_value -= strlen(right.string_value);
                  left.type = TOKEN_NUMBER;
                  lexer->string_status = false;
               }
            }
        }
    }
    return left;
}

Token parse_comparison(Lexer* lexer) {
    Token left = parse_term(lexer);
    while (lexer->current_token.type == TOKEN_LT || lexer->current_token.type == TOKEN_GT || lexer->current_token.type == TOKEN_LE || lexer->current_token.type == TOKEN_GE || lexer->current_token.type == TOKEN_EQ || lexer->current_token.type == TOKEN_NE) {
        TokenType operator = lexer->current_token.type;
        get_next_token(lexer);
        Token right = parse_term(lexer);
        switch (operator) {
            case TOKEN_LT:{
                left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.string_value ? strlen(left.string_value) : 0)) < ((right.type == TOKEN_NUMBER) ? right.number_value : (right.string_value ? strlen(right.string_value) : 0));
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                break;
            } 

            case TOKEN_GT: {
                left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.string_value ? strlen(left.string_value) : 0)) > ((right.type == TOKEN_NUMBER) ? right.number_value : (right.string_value ? strlen(right.string_value) : 0));
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                break;
            }

            case TOKEN_LE: {
                left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.string_value ? strlen(left.string_value) : 0)) <= ((right.type == TOKEN_NUMBER) ? right.number_value : (right.string_value ? strlen(right.string_value) : 0));
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                break;
            }

            case TOKEN_GE: {
                left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.string_value ? strlen(left.string_value) : 0)) >= ((right.type == TOKEN_NUMBER) ? right.number_value : (right.string_value ? strlen(right.string_value) : 0));
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                break;
            }

            case TOKEN_EQ: {
                if((left.type == TOKEN_NUMBER && right.type == TOKEN_NUMBER) || (left.type == TOKEN_NUMBER && right.type == TOKEN_STRING) || (left.type == TOKEN_STRING && right.type == TOKEN_NUMBER)){
                    left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.string_value ? strlen(left.string_value) : 0)) == ((right.type == TOKEN_NUMBER) ? right.number_value : (right.string_value ? strlen(right.string_value) : 0));
                }else if(left.type == TOKEN_STRING && right.type == TOKEN_STRING){
                    left.number_value = (left.string_value && right.string_value) ? (strcmp(left.string_value, right.string_value) == 0) : 0;
                }
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                break;
            }

            case TOKEN_NE: {
                if((left.type == TOKEN_NUMBER && right.type == TOKEN_NUMBER) || (left.type == TOKEN_NUMBER && right.type == TOKEN_STRING) || (left.type == TOKEN_STRING && right.type == TOKEN_NUMBER)){
                    left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.string_value ? strlen(left.string_value) : 0)) != ((right.type == TOKEN_NUMBER) ? right.number_value : (right.string_value ? strlen(right.string_value) : 0));
                }else if(left.type == TOKEN_STRING && right.type == TOKEN_STRING){
                    left.number_value = (left.string_value && right.string_value) ? (strcmp(left.string_value, right.string_value) != 0) : 0;
                }
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                break;
            }

            default:{
                break;
            }
        }
    }
    return left;
}

Token parse_logical_and(Lexer* lexer) {
    Token left = parse_comparison(lexer);
    while (lexer->current_token.type == TOKEN_LOGICAL_AND) {
        get_next_token(lexer);
        Token right = parse_comparison(lexer);
        left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.string_value ? 1 : 0)) && ((right.type == TOKEN_NUMBER) ? right.number_value : (right.string_value ? 1 : 0));
        left.type = TOKEN_NUMBER;
        lexer->string_status = false;
    }
    return left;
}

Token parse_logical_or(Lexer* lexer) {
    Token left = parse_logical_and(lexer);
    while (lexer->current_token.type == TOKEN_LOGICAL_OR) {
        get_next_token(lexer);
        Token right = parse_logical_and(lexer);
        left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.string_value ? 1 : 0)) || ((right.type == TOKEN_NUMBER) ? right.number_value : (right.string_value ? 1 : 0));
        left.type = TOKEN_NUMBER;
        lexer->string_status = false;
    }
    return left;
}

Token parse_xor(Lexer *lexer){
    Token sym = parse_factor(lexer);
    if(sym.type == TOKEN_XOR){
       Token right = parse_factor(lexer);
       return right;
    }
    return sym;
}

Token parse_shl(Lexer *lexer){
    Token sym = parse_factor(lexer);
    if(sym.type == TOKEN_SHL){
       Token right = parse_factor(lexer);
       return right;
    }
    return sym;
}

Token parse_shr(Lexer *lexer){
    Token sym = parse_factor(lexer);
    if(sym.type == TOKEN_SHR){
       Token right = parse_factor(lexer);
       return right;
    }
    return sym;
}

Token parse_and(Lexer *lexer){
    Token sym = parse_factor(lexer);
    if(sym.type == TOKEN_AND){
       Token right = parse_factor(lexer);
       return right;
    }
    return sym;
}

Token parse_or(Lexer *lexer){
    Token sym = parse_factor(lexer);
    if(sym.type == TOKEN_OR){
       Token right = parse_factor(lexer);
       return right;
    }
    return sym;
}

Token parse_tenary(Lexer* lexer) {
    Token condition = parse_logical_or(lexer);
    if (lexer->current_token.type == TOKEN_QUESTION) {
        get_next_token(lexer); // Consume '?'
        Token true_expr = parse_expression(lexer);
        if (lexer->current_token.type != TOKEN_COLON) {
            fprintf(stderr, "(0) Error: falta ':' en la expresión ternaria\n");
            exit(EXIT_FAILURE);
        }
        get_next_token(lexer); // Consume ':'
        Token false_expr = parse_expression(lexer);
        if(condition.type == TOKEN_STRING){
           condition = condition.string_value ? true_expr : false_expr;
           if(condition.type == TOKEN_STRING){
              lexer->string_status = true;
           }
        }else if(condition.type == TOKEN_NUMBER){
           condition = condition.number_value ? true_expr : false_expr;
           if(condition.type == TOKEN_STRING){
              lexer->string_status = true;
           }
        }else{
            printf("(0) Error de token.\n");
            exit(0);
        }
    }else if(lexer->current_token.type == TOKEN_XOR){
        Token val = parse_xor(lexer);
        if(condition.type == TOKEN_STRING){
           fprintf(stderr, "Error el simbolo ^ no soporta strings para operar.\n");
           exit(0);
        }else{
           if(val.type == TOKEN_NUMBER){
              long long xor = (long long)condition.number_value ^ (long long)val.number_value;
              condition.number_value = (double)xor;
              condition.type = TOKEN_NUMBER;
              lexer->string_status = false;
           }else{
              fprintf(stderr, "Error el simbolo ^ no soporta strings para operar.\n");
              exit(0);
           }
        }
        
    }else if(lexer->current_token.type == TOKEN_SHL){
        // TOKEN_SHL <<
        Token val = parse_shl(lexer);
        printf("SHL: %lf << %lf", condition.number_value, val.number_value);
        if(condition.type == TOKEN_STRING){

        }else{

        }
        getchar();
    }else if(lexer->current_token.type == TOKEN_SHR){
        // TOKEN_SHR >>
        Token val = parse_shr(lexer);
        printf("SHR: %lf >> %lf", condition.number_value, val.number_value);
        getchar();
    }else if(lexer->current_token.type == TOKEN_AND){
        // TOKEN_AND &
        Token val = parse_and(lexer);
        printf("AND: %lf & %lf", condition.number_value, val.number_value);
        getchar();
    }else if(lexer->current_token.type == TOKEN_OR){
        // OKEN_OR |
        Token val = parse_or(lexer);
        printf("OR: %lf | %lf", condition.number_value, val.number_value);
        getchar();
    }
    return condition;
}

Token parse_expression(Lexer* lexer) {
    return parse_tenary(lexer);
}

char* parse_expression_string(Lexer* lexer) {
    Token token = parse_expression(lexer);
    if (token.type == TOKEN_STRING) {
        return token.string_value;
    } else {
        fprintf(stderr, "Error: se esperaba una expresión de cadena\n");
        exit(EXIT_FAILURE);
    }
}

Token evaluate(const char* expression) {
    Lexer lexer = { expression, 0, false, TOKEN_END};
    get_next_token(&lexer);
    Token tk = parse_expression(&lexer);
    if(lexer.string_status){
       tk.type = TOKEN_STRING;
    }else{
       tk.type = TOKEN_NUMBER;
    }
    return tk;
}

double round_result(double value, int precision) {
    double factor = pow(10.0, precision);
    return round(value * factor) / factor;
}

// Implementaciones de funciones matemáticas
double my_fmod(double x, double y) {
    return fmod(x, y);
}

double my_pow(double base, int exp) {
    return pow(base, exp);
}

double my_abs(double value) {
    return fabs(value);
}

double my_atof(const char* str) {
    return atof(str);
}

double my_round(double value) {
    return round(value);
}

char *dtoa(char *s, double n) {
    // handle special cases
    if (isnan(n)) {
        strcpy(s, "nan");
    } else if (isinf(n)) {
        strcpy(s, "inf");
    } else if (n == 0.0) {
        strcpy(s, "0");
    } else {
        int digit, m, m1;
        char *c = s;
        int neg = (n < 0);
        if (neg)
            n = -n;
        // calculate magnitude
        m = log10(n);
        int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
        if (neg)
            *(c++) = '-';
        // set up for scientific notation
        if (useExp) {
            if (m < 0)
               m -= 1.0;
            n = n / pow(10.0, m);
            m1 = m;
            m = 0;
        }
        if (m < 1.0) {
            m = 0;
        }
        // convert the number
        while (n > PRECISION || m >= 0) {
            double weight = pow(10.0, m);
            if (weight > 0 && !isinf(weight)) {
                digit = floor(n / weight);
                n -= (digit * weight);
                *(c++) = '0' + digit;
            }
            if (m == 0 && n > 0)
                *(c++) = '.';
            m--;
        }
        if (useExp) {
            // convert the exponent
            int i, j;
            *(c++) = 'e';
            if (m1 > 0) {
                *(c++) = '+';
            } else {
                *(c++) = '-';
                m1 = -m1;
            }
            m = 0;
            while (m1 > 0) {
                *(c++) = '0' + m1 % 10;
                m1 /= 10;
                m++;
            }
            c -= m;
            for (i = 0, j = m-1; i<j; i++, j--) {
                // swap without temporary
                c[i] ^= c[j];
                c[j] ^= c[i];
                c[i] ^= c[j];
            }
            c += m;
        }
        *(c) = '\0';
    }
    return s;
}
