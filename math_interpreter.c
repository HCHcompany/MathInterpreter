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

// Funciones de análisis
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
double parse_tenary(Lexer* lexer);
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

    if (lexer->text[lexer->pos] == '\'' || lexer->text[lexer->pos] == '"') {
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
    } else if (token.type == TOKEN_STRING) {
        char* str_value = parse_primary_string(lexer);
        token.string_value = str_value;
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
    } else {
        fprintf(stderr, "Error: token inesperado en la expresión\n");
        exit(EXIT_FAILURE);
    }
}

Token parse_unary(Lexer* lexer) {
    Token token = lexer->current_token;
    if (token.type == TOKEN_MINUS) {
        get_next_token(lexer);
        Token tok = parse_primary(lexer);
        if(tok.type == TOKEN_STRING){
           return tok;
        }else{
           double result = (-tok.number_value);
           tok.number_value = result;
           return tok;    
        }
    } else {
        return parse_primary(lexer);
    }
}

Token parse_factor(Lexer* lexer) {
    Token value = parse_unary(lexer);
    Token token = lexer->current_token;

    while (token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE || token.type == TOKEN_MOD) {
        get_next_token(lexer);
        if (token.type == TOKEN_MULTIPLY) {
            if(value.type == TOKEN_STRING){
               printf("Operation fail * not apli for strings\n");
               exit(0);
            }else{
                Token lext = parse_unary(lexer);
                if(lext.type == TOKEN_NUMBER){
                   value.number_value *= lext.number_value;
                }else{
                    printf("Operation fail * not apli for strings\n");
                    exit(0);
                } 
            }
        } else if (token.type == TOKEN_DIVIDE) {
            if(value.type == TOKEN_STRING){
               printf("Operation fail / not apli for strings\n");
               exit(0);
            }else{
                Token lext = parse_unary(lexer);
                if(lext.type == TOKEN_NUMBER){
                   value.number_value /= lext.number_value;
                }else{
                    printf("Operation fail / not apli for strings\n");
                    exit(0);
                } 
            }
        } else if (token.type == TOKEN_MOD) {
            if(value.type == TOKEN_NUMBER){
               Token n = parse_unary(lexer);
               if(n.type == TOKEN_NUMBER){
                  value.number_value = my_fmod(value.number_value, n.number_value);
               }else{
                  printf("Operation fail mod not apli for strings\n");
                  exit(0);
               }
            }else{
                printf("Operation fail mod not apli for strings\n");
                exit(0);
            }
        }
        token = lexer->current_token;
    }
    return value;
}

Token parse_term(Lexer* lexer) {
    Token value = parse_factor(lexer);
    Token token = lexer->current_token;

    while (token.type == TOKEN_PLUS || token.type == TOKEN_MINUS) {
        get_next_token(lexer);
        if (token.type == TOKEN_PLUS) {
            if(value.type == TOKEN_STRING){
               Token n = parse_factor(lexer);
               if(n.type == TOKEN_NUMBER){
                  char *strn = (char *)malloc(sizeof(char) * (strlen(value.string_value) + 4096));
                  memset(strn, '\0', sizeof(char) * (strlen(value.string_value) + 4096));
                  strcpy(strn, value.string_value);
                  char *res = (char *)malloc(sizeof(char) * 4096);
                  memset(res, '\0', sizeof(char) * 4096);
                  res = dtoa(res, n.number_value);
                  strcat(strn, res);
                  free(res);
                  free(value.string_value);
                  value.string_value = NULL;
                  value.string_value = strn;
                  value.type = TOKEN_STRING;
               }else{
                  char *strn = (char *)malloc(sizeof(char) * (strlen(value.string_value) + strlen(n.string_value) + 1));
                  memset(strn, '\0', sizeof(char) * (strlen(value.string_value) + strlen(n.string_value) + 1));
                  strcpy(strn, value.string_value);
                  strcat(strn, n.string_value);
                  free(n.string_value);
                  free(value.string_value);
                  value.string_value = NULL;
                  value.string_value = strn;
                  value.type = TOKEN_STRING;
               }  
            }else{
                Token n = parse_factor(lexer);
                if(n.type == TOKEN_NUMBER){
                   value.number_value += n.number_value;
                }else{
                   char *strn = (char *)malloc(sizeof(char) * (4096 + strlen(n.string_value)));
                   memset(strn, '\0', sizeof(char) * (4096 + strlen(n.string_value)));
                   strn = dtoa(strn, value.number_value);
                   strcat(strn, n.string_value);
                   free(n.string_value);
                   value.number_value = 0;
                   value.string_value = strn;
                   value.type = TOKEN_STRING;
                }
            }
        } else if (token.type == TOKEN_MINUS) {
            if(value.type == TOKEN_STRING){
                printf("Error Operation - not apply in strings\n");
                exit(0);
            }else{
                Token n = parse_factor(lexer);
                if(n.type == TOKEN_NUMBER){
                   value.number_value -= n.number_value;
                }else{
                   printf("Error Operation - not apply in strings\n");
                   exit(0);
                }
            }
        }
        token = lexer->current_token;
    }
    return value;
}

char* parse_expression_string(Lexer* lexer) {
    char* left = parse_primary_string(lexer);
    Token token = lexer->current_token;

    while (token.type == TOKEN_PLUS) {
        get_next_token(lexer);
        char* right = parse_primary_string(lexer);
        size_t new_length = strlen(left) + strlen(right) + 1;
        left = (char*)realloc(left, new_length);
        strcat(left, right);
        free(right);
        token = lexer->current_token;
    }
    return left;
}

Token parse_expression(Lexer* lexer) {
    Token token = lexer->current_token;
    Token result = parse_term(lexer);
    if(result.type == TOKEN_STRING){
       token.string_value = result.string_value;
       lexer->string_status = true;
    }else{
       token.number_value = result.number_value;
    }
    return token;
}

Token evaluate(const char* expression) {
    Lexer lexer = {expression, 0, false, { TOKEN_END }};
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

char * dtoa(char *s, double n) {
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
