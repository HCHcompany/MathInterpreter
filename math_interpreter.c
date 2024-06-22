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
    TOKEN_CHAR,
    TOKEN_VAR,
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
    TOKEN_CCH,
    TOKEN_INVALID,
    TOKEN_NONE,
} TokenType;

// Estructura de token
typedef struct {
    TokenType type;
    union{
        double number_value;
        char *string_value;
        int char_value;
    };
} Token;

// Estructura del lexer
typedef struct {
    const char* text;
    size_t pos;
    bool string_status;
    bool char_status;
    Token current_token;
} Lexer;

//Estructura de operaciones internas.
typedef enum{
   NUMBER_STATUS,
   STRING_STATUS,
   VAR_STATUS,
   NONE_STATUS,
}OPERATOR_STATUS;

typedef struct Operator Operator;
typedef struct Operator{
   char *symbols;
   double number_value;
   char *str_value;
   char *var_value;
   OPERATOR_STATUS status;
   Operator *prev;
   Operator *next;
}Operator;

// Funciones de ayuda para el lexer
Token get_next_token(Lexer* lexer);
Token get_number_token(Lexer* lexer);
Token get_string_token(Lexer* lexer);

Token parse_expression(Lexer* lexer);
Token parse_term(Lexer* lexer);
Token parse_factor(Lexer* lexer);
Token parse_primary(Lexer* lexer);
Token parse_unary(Lexer* lexer);
Token parse_tenary(Lexer* lexer);
char* parse_expression_string(Lexer* lexer);
Token get_token_digit_char_string(Lexer *lexer);

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
        if(!fgets(expression, sizeof(expression), stdin)) {
            break;
        }
        if (strncmp(expression, "salir", 5) == 0) {
            break;
        }

        //�
        Token result = evaluate(expression);
        if(result.type == TOKEN_STRING){
            printf("Resultado: %s\n", result.string_value);
        }else if(result.type == TOKEN_CHAR){
            printf("Resultado: %c\n", (char)result.char_value);
        }else{
           if(fabs(result.number_value - (int)result.number_value) < 1e-10){
               printf("Resultado: %.0f\n", result.number_value);
           }else{
               printf("Resultado: %.14f\n", result.number_value);
           }
        }
    }

    return 0;
}

Operator *createOperator(){
   Operator *operator = (Operator *)malloc(sizeof(Operator));
   operator->next = NULL;
   operator->prev = NULL;
   operator->symbols = NULL;
   operator->number_value = 0.0f;
   operator->str_value = NULL;
   operator->var_value = NULL;
   operator->status = NONE_STATUS;
   return operator;
}

void addOperatorNumber(Operator *operator, const char *symbols, double value){
   Operator **pos = &operator;
   Operator **prev = NULL;
   while(true){
         if(*pos){
            if(!((*pos)->symbols)){
                (*pos)->symbols = (char *)malloc(sizeof(char) * (strlen(symbols) + 1));
                memset((*pos)->symbols, '\0', sizeof(char) * (strlen(symbols) + 1));
                strcpy((*pos)->symbols, symbols);
                (*pos)->number_value = value;
                (*pos)->status = NUMBER_STATUS;
                break;
            }else{
               prev = pos;
               pos = &((*pos)->next);
            }
         }else{
            (*pos) = (Operator *)malloc(sizeof(Operator));
            (*pos)->next = NULL;
            if(prev){
               (*pos)->prev = *prev;
            }else{
               (*pos)->prev = NULL;
            }
            (*pos)->symbols = (char *)malloc(sizeof(char) * (strlen(symbols) + 1));
             memset((*pos)->symbols, '\0', sizeof(char) * (strlen(symbols) + 1));
             strcpy((*pos)->symbols, symbols);
             (*pos)->number_value = value;
             (*pos)->status = NUMBER_STATUS;
             (*pos)->str_value = NULL;
             (*pos)->var_value = NULL;
             break;
         }
   }
}

void addOperatorString(Operator *operator, const char *symbols, const char *value){
   Operator **pos = &operator;
   Operator **prev = NULL;
   while(true){
         if(*pos){
            if(!((*pos)->symbols)){
                (*pos)->symbols = (char *)malloc(sizeof(char) * (strlen(symbols) + 1));
                memset((*pos)->symbols, '\0', sizeof(char) * (strlen(symbols) + 1));
                strcpy((*pos)->symbols, symbols);
                (*pos)->str_value = (char *)malloc(sizeof(char) * (strlen(value) + 1));
                memset((*pos)->str_value, '\0', sizeof(char) * (strlen(value) + 1));
                strcpy((*pos)->str_value, value);
                (*pos)->status = STRING_STATUS;
                break;
            }else{
               prev = pos;
               pos = &((*pos)->next);
            }
         }else{
            (*pos) = (Operator *)malloc(sizeof(Operator));
            (*pos)->next = NULL;
            if(prev){
               (*pos)->prev = *prev;
            }else{
               (*pos)->prev = NULL;
            }
            (*pos)->symbols = (char *)malloc(sizeof(char) * (strlen(symbols) + 1));
             memset((*pos)->symbols, '\0', sizeof(char) * (strlen(symbols) + 1));
             strcpy((*pos)->symbols, symbols);
             (*pos)->number_value = 0;
             (*pos)->str_value = (char *)malloc(sizeof(char) * (strlen(value) + 1));
             memset((*pos)->str_value, '\0', sizeof(char) * (strlen(value) + 1));
             strcpy((*pos)->str_value, value);
             (*pos)->status = STRING_STATUS;
             (*pos)->var_value = NULL;
             break;
         }
   }
}

void addOperatorVar(Operator *operator, const char *symbols, const char *value){
   Operator **pos = &operator;
   Operator **prev = NULL;
   while(true){
         if(*pos){
            if(!((*pos)->symbols)){
                (*pos)->symbols = (char *)malloc(sizeof(char) * (strlen(symbols) + 1));
                memset((*pos)->symbols, '\0', sizeof(char) * (strlen(symbols) + 1));
                strcpy((*pos)->symbols, symbols);
                (*pos)->var_value = (char *)malloc(sizeof(char) * (strlen(value) + 1));
                memset((*pos)->var_value, '\0', sizeof(char) * (strlen(value) + 1));
                strcpy((*pos)->var_value, value);
                (*pos)->status = VAR_STATUS;
                break;
            }else{
               prev = pos;
               pos = &((*pos)->next);
            }
         }else{
            (*pos) = (Operator *)malloc(sizeof(Operator));
            (*pos)->next = NULL;
            if(prev){
               (*pos)->prev = *prev;
            }else{
               (*pos)->prev = NULL;
            }
            (*pos)->symbols = (char *)malloc(sizeof(char) * (strlen(symbols) + 1));
             memset((*pos)->symbols, '\0', sizeof(char) * (strlen(symbols) + 1));
             strcpy((*pos)->symbols, symbols);
             (*pos)->number_value = 0;
             (*pos)->var_value = (char *)malloc(sizeof(char) * (strlen(value) + 1));
             memset((*pos)->var_value, '\0', sizeof(char) * (strlen(value) + 1));
             strcpy((*pos)->var_value, value);
             (*pos)->status = VAR_STATUS;
             (*pos)->str_value = NULL;
             break;
         }
   }
}

void freeOperator(Operator *operator){
   if(operator){
      Operator **pos = &operator;
      Operator **end = NULL;
      while(*pos){
            end = pos;
            pos = &((*pos)->next);
      }

      Operator **prev = NULL;
      while(*end){
            prev = &((*end)->prev);
            if((*end)->status == NUMBER_STATUS){
               (*end)->number_value = 0;
            }else if((*end)->status == STRING_STATUS){
               free((*end)->str_value);
               (*end)->str_value = NULL;
            }else if((*end)->status == VAR_STATUS){
               free((*end)->var_value);
               (*end)->var_value = NULL;
            }
            free((*end)->symbols);
            free((*end));
            end = prev;
      }
   }
}

bool is_variable_value(Lexer *lexer){
    if(lexer->text[lexer->pos] != '\'' && lexer->text[lexer->pos] != '"' && lexer->text[lexer->pos] != '*' && lexer->text[lexer->pos] != '/' &&
       lexer->text[lexer->pos] != '(' && lexer->text[lexer->pos] != ')' && lexer->text[lexer->pos] != '~' && lexer->text[lexer->pos] != '!' && 
       lexer->text[lexer->pos] != '%' && lexer->text[lexer->pos] != '&' && lexer->text[lexer->pos] != '|' && lexer->text[lexer->pos] != '<' && 
       lexer->text[lexer->pos] != '>' && lexer->text[lexer->pos] != '=' && lexer->text[lexer->pos] != '^' && lexer->text[lexer->pos] != '?' &&
       lexer->text[lexer->pos] != ':' && lexer->text[lexer->pos] != '0' && lexer->text[lexer->pos] != '1' && lexer->text[lexer->pos] != '2' &&
       lexer->text[lexer->pos] != '3' && lexer->text[lexer->pos] != '4' && lexer->text[lexer->pos] != '5' && lexer->text[lexer->pos] != '6' &&
       lexer->text[lexer->pos] != '7' && lexer->text[lexer->pos] != '8' && lexer->text[lexer->pos] != '9'){
       if(lexer->text[lexer->pos] == '+' ){
          if(lexer->text[lexer->pos + 1] == '+'){
             return true;
          }else{
             return false;
          }
       }else if(lexer->text[lexer->pos] == '-'){
          if(lexer->text[lexer->pos + 1] == '-'){
             return true;
          }else{
             return false;
          }
       }else{
         return true;
       }
    }else{
         return false;
    }
}

Token get_variable_value(Lexer *lexer, bool sub){
    //Obtener nombre de la variable.
    Operator *operator = createOperator();
    char *var_name = (char *)malloc(sizeof(char) * (strlen(lexer->text) + 1));
    memset(var_name, '\0', sizeof(char) * (strlen(lexer->text) + 1));
    int u0 = 0; // Esta es la posicion del puntero donde se guarda el nombre de la variable.
    bool isSymbols = false; // Este especifica si ya se a declarado un simbolo "++" o "--" dentro de la operacion.
    while(lexer->text[lexer->pos] != '\'' && lexer->text[lexer->pos] != '"' && lexer->text[lexer->pos] != '<' &&  lexer->text[lexer->pos] != '?' && 
          lexer->text[lexer->pos] != '(' && lexer->text[lexer->pos] != ')' && lexer->text[lexer->pos] != '~' && lexer->text[lexer->pos] != '!' && 
          lexer->text[lexer->pos] != '>' && lexer->text[lexer->pos] != ':' && lexer->text[lexer->pos] != '\0'){
          char c = lexer->text[lexer->pos];
          if(c == ' ' || c == '\t'){
             lexer->pos++;
          }else if(c == '+'){
             char b = lexer->text[lexer->pos + 1];
             if(b == '+'){ //Se incrementa 1.
                if(!isSymbols){ // Se verifica si ya se han declarado simbolos previos.
                    lexer->pos+=2; // Se incrementa 2 para llegar al proximo caracter.
                    addOperatorNumber(operator, "++", 1); // Se añade el operador de suma incrementando 1.
                    isSymbols = true; // Se establece la declaracion de simbolos como verdadero.
                }else{
                  //Error simbolos ya declarados.
                  fprintf(stderr, "(0) Symbols error in operation.\n");
                  exit(0);
                }
             }else if(b == '=' && sub){ // Se incrementa el valor que le sigue.
                if(!isSymbols){
                    if(u0 > 0){
                       lexer->pos += 2; // Se incrementa la posicion a 2 para seguir leyendo el codigo que sigue.
                       while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){ // Se recorre el codigo hasta encontrar un caracter diferente a space y tab o \0
                             lexer->pos++;
                       }

                       if(lexer->text[lexer->pos] == '\0'){ // Se verifica si el caracter en la posicion actual no indica el termino del codigo.
                          fprintf(stderr, "(3) Error termino inesperado del codigo.\n"); // En el caso de que haya terminado se lanza un error indicando que hubo un termino inesperado.
                          exit(0);
                       }

                       //En el caso de que el caracter sea valido completamente se procede a validar si es NUMBER, CHAR o STRING.
                       Token val = get_token_digit_char_string(lexer); // Esto retornara un tipo NUMBER, STRING, CHAR o en el caso de que no coincida NONE.
                       if(val.type == TOKEN_CHAR || val.type == TOKEN_NUMBER){ // En el caso de que sea NUMBER o CHAR se procede a añadir el operador incrementando el valor leido.
                          addOperatorNumber(operator, "+=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                          lexer->char_status = false;
                       }else if(val.type == TOKEN_STRING){ // En el caso de que el tipo sea STRING se procede a añadir el operador y el valor.
                          addOperatorString(operator, "+=", val.string_value);
                          free(val.string_value);
                          lexer->string_status = false;
                       }else{ //Se asume que es una variable.
                          //Se procede a buscar nombre y valor.
                          val = get_variable_value(lexer, false); // En este caso el segundo parametro es false para omitir la simbologia +=, -=, etc....
                          if(val.type == TOKEN_NUMBER || val.type == TOKEN_CHAR){
                             addOperatorNumber(operator, "+=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                             lexer->char_status = false;
                          }else if(val.type == TOKEN_STRING){
                             addOperatorString(operator, "+=", val.string_value);
                             free(val.string_value);
                             lexer->string_status = false;
                          }else{
                             fprintf(stderr, "(err1) Error token desconocido\n");
                          }
                       }

                    }else{
                      fprintf(stderr, "(1) Error no se especifico el la variable.\n");
                      exit(0);
                    } 
                }else{
                   fprintf(stderr, "(2) Symbols error in operation.\n");
                   exit(0);
                }
             }else{ // El token corresponde a una suma.
                break;
             }
          }else if(c == '-'){
             char b = lexer->text[lexer->pos + 1];
             if(b == '-'){ //Se resta 1.
                if(!isSymbols){ // Se verifica si ya se han declarado simbolos previos.
                    lexer->pos+=2; // Se incrementa 2 para llegar al proximo caracter.
                    addOperatorNumber(operator, "--", 1); // Se añade el operador de resta disminuyendo 1.
                    isSymbols = true; // Se establece la declaracion de simbolos como verdadero.
                }else{
                  //Error simbolos ya declarados.
                  fprintf(stderr, "(0) Symbols error in operation.\n");
                  exit(0);
                }
             }else if(b == '=' && sub){ // Se resta el valor que le sigue.
                if(!isSymbols){
                    if(u0 > 0){
                       lexer->pos += 2; // Se incrementa la posicion a 2 para seguir leyendo el codigo que sigue.
                       while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){ // Se recorre el codigo hasta encontrar un caracter diferente a space y tab o \0
                             lexer->pos++;
                       }

                       if(lexer->text[lexer->pos] == '\0'){ // Se verifica si el caracter en la posicion actual no indica el termino del codigo.
                          fprintf(stderr, "(3) Error termino inesperado del codigo.\n"); // En el caso de que haya terminado se lanza un error indicando que hubo un termino inesperado.
                          exit(0);
                       }

                       //En el caso de que el caracter sea valido completamente se procede a validar si es NUMBER, CHAR o STRING.
                       Token val = get_token_digit_char_string(lexer); // Esto retornara un tipo NUMBER, STRING, CHAR o en el caso de que no coincida NONE.
                       if(val.type == TOKEN_CHAR || val.type == TOKEN_NUMBER){ // En el caso de que sea NUMBER o CHAR se procede a añadir el operador incrementando el valor leido.
                          addOperatorNumber(operator, "-=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                          lexer->char_status = false;
                       }else if(val.type == TOKEN_STRING){ // En el caso de que el tipo sea STRING se procede a añadir el operador y el valor.
                          addOperatorString(operator, "-=", val.string_value);
                          free(val.string_value);
                          lexer->string_status = false;
                       }else{ //Se asume que es una variable.
                          //Se procede a buscar nombre y valor.
                          val = get_variable_value(lexer, false); // En este caso el segundo parametro es false para omitir la simbologia +=, -=, etc....
                          if(val.type == TOKEN_NUMBER || val.type == TOKEN_CHAR){
                             addOperatorNumber(operator, "-=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                             lexer->char_status = false;
                          }else if(val.type == TOKEN_STRING){
                             addOperatorString(operator, "-=", val.string_value);
                             free(val.string_value);
                             lexer->string_status = false;
                          }else{
                             fprintf(stderr, "(err1) Error token desconocido\n");
                          }
                       }

                    }else{
                      fprintf(stderr, "(1) Error no se especifico el la variable.\n");
                      exit(0);
                    } 
                }else{
                   fprintf(stderr, "(2) Symbols error in operation.\n");
                   exit(0);
                }
             }else{ // El token corresponde a una suma.
                break;
             }
          }else if(c == '*'){
             char b = lexer->text[lexer->pos + 1];
             if(b == '*'){ //En este caso si el simbolo que sigue es * es porque hay un error.
                fprintf(stderr, "Error en la operacion ** no valida\n");
                exit(0);
             }else if(b == '=' && sub){ // Se multiplica por el valor que le sigue.
                if(!isSymbols){
                    if(u0 > 0){
                       lexer->pos += 2; // Se incrementa la posicion a 2 para seguir leyendo el codigo que sigue.
                       while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){ // Se recorre el codigo hasta encontrar un caracter diferente a space y tab o \0
                             lexer->pos++;
                       }

                       if(lexer->text[lexer->pos] == '\0'){ // Se verifica si el caracter en la posicion actual no indica el termino del codigo.
                          fprintf(stderr, "(3) Error termino inesperado del codigo.\n"); // En el caso de que haya terminado se lanza un error indicando que hubo un termino inesperado.
                          exit(0);
                       }

                       //En el caso de que el caracter sea valido completamente se procede a validar si es NUMBER, CHAR o STRING.
                       Token val = get_token_digit_char_string(lexer); // Esto retornara un tipo NUMBER, STRING, CHAR o en el caso de que no coincida NONE.
                       if(val.type == TOKEN_CHAR || val.type == TOKEN_NUMBER){ // En el caso de que sea NUMBER o CHAR se procede a añadir el operador incrementando el valor leido.
                          addOperatorNumber(operator, "*=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                          lexer->char_status = false;
                       }else if(val.type == TOKEN_STRING){ // En el caso de que el tipo sea STRING se procede a añadir el operador y el valor.
                          addOperatorString(operator, "*=", val.string_value);
                          free(val.string_value);
                          lexer->string_status = false;
                       }else{ //Se asume que es una variable.
                          //Se procede a buscar nombre y valor.
                          val = get_variable_value(lexer, false); // En este caso el segundo parametro es false para omitir la simbologia +=, -=, etc....
                          if(val.type == TOKEN_NUMBER || val.type == TOKEN_CHAR){
                             addOperatorNumber(operator, "*=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                             lexer->char_status = false;
                          }else if(val.type == TOKEN_STRING){
                             addOperatorString(operator, "*=", val.string_value);
                             free(val.string_value);
                             lexer->string_status = false;
                          }else{
                             fprintf(stderr, "(err1) Error token desconocido\n");
                          }
                       }

                    }else{
                      fprintf(stderr, "(1) Error no se especifico el la variable.\n");
                      exit(0);
                    } 
                }else{
                   fprintf(stderr, "(2) Symbols error in operation.\n");
                   exit(0);
                }
             }else{ // El token corresponde a una suma.
                break;
             }
          }else if(c == '/'){
             char b = lexer->text[lexer->pos + 1];
             if(b == '/'){ //En este caso si el simbolo que sigue es / es porque hay un error.
                fprintf(stderr, "Error en la operacion // no valida\n");
                exit(0);
             }else if(b == '=' && sub){ // Se divide por el valor que le sigue.
                if(!isSymbols){
                    if(u0 > 0){
                       lexer->pos += 2; // Se incrementa la posicion a 2 para seguir leyendo el codigo que sigue.
                       while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){ // Se recorre el codigo hasta encontrar un caracter diferente a space y tab o \0
                             lexer->pos++;
                       }

                       if(lexer->text[lexer->pos] == '\0'){ // Se verifica si el caracter en la posicion actual no indica el termino del codigo.
                          fprintf(stderr, "(3) Error termino inesperado del codigo.\n"); // En el caso de que haya terminado se lanza un error indicando que hubo un termino inesperado.
                          exit(0);
                       }

                       //En el caso de que el caracter sea valido completamente se procede a validar si es NUMBER, CHAR o STRING.
                       Token val = get_token_digit_char_string(lexer); // Esto retornara un tipo NUMBER, STRING, CHAR o en el caso de que no coincida NONE.
                       if(val.type == TOKEN_CHAR || val.type == TOKEN_NUMBER){ // En el caso de que sea NUMBER o CHAR se procede a añadir el operador incrementando el valor leido.
                          addOperatorNumber(operator, "/=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                          lexer->char_status = false;
                       }else if(val.type == TOKEN_STRING){ // En el caso de que el tipo sea STRING se procede a añadir el operador y el valor.
                          addOperatorString(operator, "/=", val.string_value);
                          free(val.string_value);
                          lexer->string_status = false;
                       }else{ //Se asume que es una variable.
                          //Se procede a buscar nombre y valor.
                          val = get_variable_value(lexer, false); // En este caso el segundo parametro es false para omitir la simbologia +=, -=, etc....
                          if(val.type == TOKEN_NUMBER || val.type == TOKEN_CHAR){
                             addOperatorNumber(operator, "/=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                             lexer->char_status = false;
                          }else if(val.type == TOKEN_STRING){
                             addOperatorString(operator, "/=", val.string_value);
                             free(val.string_value);
                             lexer->string_status = false;
                          }else{
                             fprintf(stderr, "(err1) Error token desconocido\n");
                          }
                       }

                    }else{
                      fprintf(stderr, "(1) Error no se especifico el la variable.\n");
                      exit(0);
                    } 
                }else{
                   fprintf(stderr, "(2) Symbols error in operation.\n");
                   exit(0);
                }
             }else{ // El token corresponde a una suma.
                break;
             }
          }else if(c == '%'){ 
             char b = lexer->text[lexer->pos + 1];
             if(b == '%'){ //En este caso si el simbolo que sigue es % es porque hay un error.
                fprintf(stderr, "Error en la operacion %% no valida\n");
                exit(0);
             }else if(b == '=' && sub){ // Resto del el valor que le sigue.
                if(!isSymbols){
                    if(u0 > 0){
                       lexer->pos += 2; // Se incrementa la posicion a 2 para seguir leyendo el codigo que sigue.
                       while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){ // Se recorre el codigo hasta encontrar un caracter diferente a space y tab o \0
                             lexer->pos++;
                       }

                       if(lexer->text[lexer->pos] == '\0'){ // Se verifica si el caracter en la posicion actual no indica el termino del codigo.
                          fprintf(stderr, "(3) Error termino inesperado del codigo.\n"); // En el caso de que haya terminado se lanza un error indicando que hubo un termino inesperado.
                          exit(0);
                       }

                       //En el caso de que el caracter sea valido completamente se procede a validar si es NUMBER, CHAR o STRING.
                       Token val = get_token_digit_char_string(lexer); // Esto retornara un tipo NUMBER, STRING, CHAR o en el caso de que no coincida NONE.
                       if(val.type == TOKEN_CHAR || val.type == TOKEN_NUMBER){ // En el caso de que sea NUMBER o CHAR se procede a añadir el operador incrementando el valor leido.
                          addOperatorNumber(operator, "%=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                          lexer->char_status = false;
                       }else if(val.type == TOKEN_STRING){ // En el caso de que el tipo sea STRING se procede a añadir el operador y el valor.
                          addOperatorString(operator, "%=", val.string_value);
                          free(val.string_value);
                          lexer->string_status = false;
                       }else{ //Se asume que es una variable.
                          //Se procede a buscar nombre y valor.
                          val = get_variable_value(lexer, false); // En este caso el segundo parametro es false para omitir la simbologia +=, -=, etc....
                          if(val.type == TOKEN_NUMBER || val.type == TOKEN_CHAR){
                             addOperatorNumber(operator, "%=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                             lexer->char_status = false;
                          }else if(val.type == TOKEN_STRING){
                             addOperatorString(operator, "%=", val.string_value);
                             free(val.string_value);
                             lexer->string_status = false;
                          }else{
                             fprintf(stderr, "(err1) Error token desconocido\n");
                          }
                       }

                    }else{
                      fprintf(stderr, "(1) Error no se especifico el la variable.\n");
                      exit(0);
                    } 
                }else{
                   fprintf(stderr, "(2) Symbols error in operation.\n");
                   exit(0);
                }
             }else{ // El token corresponde a una suma.
                break;
             }
          }else if(c == '&'){
            char b = lexer->text[lexer->pos + 1];
             if(b == '&'){ //En este caso se detecta un operador logico.
                lexer->pos--;
                break;
             }else if(b == '=' && sub){ // Se incrementa el valor que le sigue.
                if(!isSymbols){
                    if(u0 > 0){
                       lexer->pos += 2; // Se incrementa la posicion a 2 para seguir leyendo el codigo que sigue.
                       while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){ // Se recorre el codigo hasta encontrar un caracter diferente a space y tab o \0
                             lexer->pos++;
                       }

                       if(lexer->text[lexer->pos] == '\0'){ // Se verifica si el caracter en la posicion actual no indica el termino del codigo.
                          fprintf(stderr, "(3) Error termino inesperado del codigo.\n"); // En el caso de que haya terminado se lanza un error indicando que hubo un termino inesperado.
                          exit(0);
                       }

                       //En el caso de que el caracter sea valido completamente se procede a validar si es NUMBER, CHAR o STRING.
                       Token val = get_token_digit_char_string(lexer); // Esto retornara un tipo NUMBER, STRING, CHAR o en el caso de que no coincida NONE.
                       if(val.type == TOKEN_CHAR || val.type == TOKEN_NUMBER){ // En el caso de que sea NUMBER o CHAR se procede a añadir el operador incrementando el valor leido.
                          addOperatorNumber(operator, "&=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                          lexer->char_status = false;
                       }else if(val.type == TOKEN_STRING){ // En el caso de que el tipo sea STRING se procede a añadir el operador y el valor.
                          addOperatorString(operator, "&=", val.string_value);
                          free(val.string_value);
                          lexer->string_status = false;
                       }else{ //Se asume que es una variable.
                          //Se procede a buscar nombre y valor.
                          val = get_variable_value(lexer, false); // En este caso el segundo parametro es false para omitir la simbologia +=, -=, etc....
                          if(val.type == TOKEN_NUMBER || val.type == TOKEN_CHAR){
                             addOperatorNumber(operator, "&=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                             lexer->char_status = false;
                          }else if(val.type == TOKEN_STRING){
                             addOperatorString(operator, "&=", val.string_value);
                             free(val.string_value);
                             lexer->string_status = false;
                          }else{
                             fprintf(stderr, "(err1) Error token desconocido\n");
                          }
                       }

                    }else{
                      fprintf(stderr, "(1) Error no se especifico el la variable.\n");
                      exit(0);
                    } 
                }else{
                   fprintf(stderr, "(2) Symbols error in operation.\n");
                   exit(0);
                }
             }else{ // El token corresponde a una suma.
                break;
             }
          }else if(c == '|'){
             char b = lexer->text[lexer->pos + 1];
             if(b == '|'){ //En este caso se detecta un operador logico.
                lexer->pos--;
                break;
             }else if(b == '=' && sub){ // Se | el valor que le sigue.
                if(!isSymbols){
                    if(u0 > 0){
                       lexer->pos += 2; // Se incrementa la posicion a 2 para seguir leyendo el codigo que sigue.
                       while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){ // Se recorre el codigo hasta encontrar un caracter diferente a space y tab o \0
                             lexer->pos++;
                       }

                       if(lexer->text[lexer->pos] == '\0'){ // Se verifica si el caracter en la posicion actual no indica el termino del codigo.
                          fprintf(stderr, "(3) Error termino inesperado del codigo.\n"); // En el caso de que haya terminado se lanza un error indicando que hubo un termino inesperado.
                          exit(0);
                       }

                       //En el caso de que el caracter sea valido completamente se procede a validar si es NUMBER, CHAR o STRING.
                       Token val = get_token_digit_char_string(lexer); // Esto retornara un tipo NUMBER, STRING, CHAR o en el caso de que no coincida NONE.
                       if(val.type == TOKEN_CHAR || val.type == TOKEN_NUMBER){ // En el caso de que sea NUMBER o CHAR se procede a añadir el operador incrementando el valor leido.
                          addOperatorNumber(operator, "|=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                          lexer->char_status = false;
                       }else if(val.type == TOKEN_STRING){ // En el caso de que el tipo sea STRING se procede a añadir el operador y el valor.
                          addOperatorString(operator, "|=", val.string_value);
                          free(val.string_value);
                          lexer->string_status = false;
                       }else{ //Se asume que es una variable.
                          //Se procede a buscar nombre y valor.
                          val = get_variable_value(lexer, false); // En este caso el segundo parametro es false para omitir la simbologia +=, -=, etc....
                          if(val.type == TOKEN_NUMBER || val.type == TOKEN_CHAR){
                             addOperatorNumber(operator, "|=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                             lexer->char_status = false;
                          }else if(val.type == TOKEN_STRING){
                             addOperatorString(operator, "|=", val.string_value);
                             free(val.string_value);
                             lexer->string_status = false;
                          }else{
                             fprintf(stderr, "(err1) Error token desconocido\n");
                          }
                       }

                    }else{
                      fprintf(stderr, "(1) Error no se especifico el la variable.\n");
                      exit(0);
                    } 
                }else{
                   fprintf(stderr, "(2) Symbols error in operation.\n");
                   exit(0);
                }
             }else{ // El token corresponde a una suma.
                break;
             }
          }else if(c == '='){
            printf("Llegue aqui\n");
            char b = lexer->text[lexer->pos + 1];
             if(b == '='){ //En este caso se toma como un operador logico por lo tanto se frena el proceso.
                lexer->pos--; // Se resta uno para que el operador logico quede completo. 
                break;
             }else if(b == '=' && sub){ // Se asigna el valor que le sigue.
                if(!isSymbols){
                    if(u0 > 0){
                       lexer->pos += 2; // Se incrementa la posicion a 2 para seguir leyendo el codigo que sigue.
                       while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){ // Se recorre el codigo hasta encontrar un caracter diferente a space y tab o \0
                             lexer->pos++;
                       }

                       if(lexer->text[lexer->pos] == '\0'){ // Se verifica si el caracter en la posicion actual no indica el termino del codigo.
                          fprintf(stderr, "(3) Error termino inesperado del codigo.\n"); // En el caso de que haya terminado se lanza un error indicando que hubo un termino inesperado.
                          exit(0);
                       }

                       //En el caso de que el caracter sea valido completamente se procede a validar si es NUMBER, CHAR o STRING.
                       Token val = get_token_digit_char_string(lexer); // Esto retornara un tipo NUMBER, STRING, CHAR o en el caso de que no coincida NONE.
                       if(val.type == TOKEN_CHAR || val.type == TOKEN_NUMBER){ // En el caso de que sea NUMBER o CHAR se procede a añadir el operador incrementando el valor leido.
                          addOperatorNumber(operator, "=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                          lexer->char_status = false;
                       }else if(val.type == TOKEN_STRING){ // En el caso de que el tipo sea STRING se procede a añadir el operador y el valor.
                          addOperatorString(operator, "=", val.string_value);
                          free(val.string_value);
                          lexer->string_status = false;
                       }else{ //Se asume que es una variable.
                          //Se procede a buscar nombre y valor.
                          val = get_variable_value(lexer, false); // En este caso el segundo parametro es false para omitir la simbologia +=, -=, etc....
                          if(val.type == TOKEN_NUMBER || val.type == TOKEN_CHAR){
                             addOperatorNumber(operator, "=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                             lexer->char_status = false;
                          }else if(val.type == TOKEN_STRING){
                             addOperatorString(operator, "=", val.string_value);
                             free(val.string_value);
                             lexer->string_status = false;
                          }else{
                             fprintf(stderr, "(err1) Error token desconocido\n");
                          }
                       }

                    }else{
                      fprintf(stderr, "(1) Error no se especifico el la variable.\n");
                      exit(0);
                    } 
                }else{
                   fprintf(stderr, "(2) Symbols error in operation.\n");
                   exit(0);
                }
             }else{ // El token corresponde a una suma.
                break;
             }
          }else if(c == '^'){
             char b = lexer->text[lexer->pos + 1];
             if(b == '^'){ //En este caso si el simbolo que sigue es ^ es porque hay un error.
                fprintf(stderr, "Error en la operacion ^^ no valida\n");
                exit(0);
             }else if(b == '=' && sub){ // Se ^ el valor que le sigue.
                if(!isSymbols){
                    if(u0 > 0){
                       lexer->pos += 2; // Se incrementa la posicion a 2 para seguir leyendo el codigo que sigue.
                       while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){ // Se recorre el codigo hasta encontrar un caracter diferente a space y tab o \0
                             lexer->pos++;
                       }

                       if(lexer->text[lexer->pos] == '\0'){ // Se verifica si el caracter en la posicion actual no indica el termino del codigo.
                          fprintf(stderr, "(3) Error termino inesperado del codigo.\n"); // En el caso de que haya terminado se lanza un error indicando que hubo un termino inesperado.
                          exit(0);
                       }

                       //En el caso de que el caracter sea valido completamente se procede a validar si es NUMBER, CHAR o STRING.
                       Token val = get_token_digit_char_string(lexer); // Esto retornara un tipo NUMBER, STRING, CHAR o en el caso de que no coincida NONE.
                       if(val.type == TOKEN_CHAR || val.type == TOKEN_NUMBER){ // En el caso de que sea NUMBER o CHAR se procede a añadir el operador incrementando el valor leido.
                          addOperatorNumber(operator, "^=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                          lexer->char_status = false;
                       }else if(val.type == TOKEN_STRING){ // En el caso de que el tipo sea STRING se procede a añadir el operador y el valor.
                          addOperatorString(operator, "^=", val.string_value);
                          free(val.string_value);
                          lexer->string_status = false;
                       }else{ //Se asume que es una variable.
                          //Se procede a buscar nombre y valor.
                          val = get_variable_value(lexer, false); // En este caso el segundo parametro es false para omitir la simbologia +=, -=, etc....
                          if(val.type == TOKEN_NUMBER || val.type == TOKEN_CHAR){
                             addOperatorNumber(operator, "^=", val.type == TOKEN_NUMBER ? val.number_value : (double)(int)val.char_value);
                             lexer->char_status = false;
                          }else if(val.type == TOKEN_STRING){
                             addOperatorString(operator, "^=", val.string_value);
                             free(val.string_value);
                             lexer->string_status = false;
                          }else{
                             fprintf(stderr, "(err1) Error token desconocido\n");
                          }
                       }

                    }else{
                      fprintf(stderr, "(1) Error no se especifico el la variable.\n");
                      exit(0);
                    } 
                }else{
                   fprintf(stderr, "(2) Symbols error in operation.\n");
                   exit(0);
                }
             }else{ // El token corresponde a una suma.
                break;
             }
          }else{
             var_name[u0] = c;
             lexer->pos++;
             u0++;
          }
    }

    //Verificar si existe y obtener valor de la variable.
    //En el caso de que la variable sea numerica y los simbolos sean ++ o --, de incrementa 1 o se discrimina 1

    free(var_name);
    Token value = {TOKEN_NUMBER};
    value.number_value = 12;

    //----------------TEST OPERATOR-------------------
    //Esto solo es una prueba, me debo basar en esto para hacer la implementacion real.
    Operator **pos = &operator;
    while(*pos && (*pos)->symbols){ // %, &, |, =, ^
          if(strcmp((*pos)->symbols, "++") == 0 || strcmp((*pos)->symbols, "+=") == 0){
             if((*pos)->status == NUMBER_STATUS){ 
                value.number_value += (*pos)->number_value;
             }else if((*pos)->status == STRING_STATUS){

             }else if((*pos)->status == VAR_STATUS){

             }
          }else if(strcmp((*pos)->symbols, "--") == 0 || strcmp((*pos)->symbols, "-=") == 0){
             if((*pos)->status == NUMBER_STATUS){ 
                value.number_value -= (*pos)->number_value;
             }else if((*pos)->status == STRING_STATUS){

             }else if((*pos)->status == VAR_STATUS){

             }
          }else if(strcmp((*pos)->symbols, "*=") == 0){
             if((*pos)->status == NUMBER_STATUS){ 
                value.number_value *= (*pos)->number_value;
             }else if((*pos)->status == STRING_STATUS){

             }else if((*pos)->status == VAR_STATUS){

             }
          }else if(strcmp((*pos)->symbols, "/=") == 0){
             if((*pos)->status == NUMBER_STATUS){ 
                value.number_value /= (*pos)->number_value;
             }else if((*pos)->status == STRING_STATUS){

             }else if((*pos)->status == VAR_STATUS){

             }
          }else if(strcmp((*pos)->symbols, "%=") == 0){
             if((*pos)->status == NUMBER_STATUS){ 
                value.number_value = fmod(value.number_value, (*pos)->number_value);
             }else if((*pos)->status == STRING_STATUS){

             }else if((*pos)->status == VAR_STATUS){

             }
          }else if(strcmp((*pos)->symbols, "&=") == 0){
             if((*pos)->status == NUMBER_STATUS){ 
                long l0 = (long)value.number_value;
                long l1 = (long)(*pos)->number_value;
                value.number_value = (double)(l0 & l1);
             }else if((*pos)->status == STRING_STATUS){

             }else if((*pos)->status == VAR_STATUS){

             }
          }else if(strcmp((*pos)->symbols, "|=") == 0){
             if((*pos)->status == NUMBER_STATUS){ 
                long l0 = (long)value.number_value;
                long l1 = (long)(*pos)->number_value;
                value.number_value = (double)(l0 | l1);
             }else if((*pos)->status == STRING_STATUS){

             }else if((*pos)->status == VAR_STATUS){

             }
          }else if(strcmp((*pos)->symbols, "=") == 0){
             if((*pos)->status == NUMBER_STATUS){ 
                value.number_value = (*pos)->number_value;
             }else if((*pos)->status == STRING_STATUS){

             }else if((*pos)->status == VAR_STATUS){

             }
          }else if(strcmp((*pos)->symbols, "^=") == 0){
             if((*pos)->status == NUMBER_STATUS){ 
                long l0 = (long)value.number_value;
                long l1 = (long)(*pos)->number_value;
                value.number_value = (double)(l0 ^ l1);
             }else if((*pos)->status == STRING_STATUS){

             }else if((*pos)->status == VAR_STATUS){

             }
          }else{
             fprintf(stderr, "Error simbolo no existe\n");
             exit(0);
          }
          pos = &((*pos)->next);
    }
    freeOperator(operator);
    //-------------------END TEST---------------------

    lexer->char_status = false;
    lexer->string_status = false;
    lexer->current_token = value;
    return value;
}

Token get_token_digit_char_string(Lexer *lexer){
   if (isdigit(lexer->text[lexer->pos]) || lexer->text[lexer->pos] == '.') {
        return get_number_token(lexer);
    }

    if(lexer->text[lexer->pos] == '\''){
        Token n = get_string_token(lexer);
        if(strlen(n.string_value) < 3 && strlen(n.string_value) > 0){
           if(strlen(n.string_value) == 2){
              if(n.string_value[0] == '\\'){
                 Token token = { TOKEN_CHAR };
                 token.char_value = n.string_value[1];
                 lexer->current_token = token;
                 lexer->string_status = false;
                 lexer->char_status = true;
                 free(n.string_value);
                 return token;
              }else{
                printf("Error de declaracion de caracter: %s\n", n.string_value);
                exit(0);     
              }
           }else{
              Token token = { TOKEN_CHAR };
              token.char_value = n.string_value[0];
              lexer->current_token = token;
              lexer->string_status = false;
              lexer->char_status = true;
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
    Token end = {TOKEN_NONE};
    return end;
}

// Implementación del lexer
Token get_next_token(Lexer* lexer) {
    while(lexer->text[lexer->pos] != '\0' && isspace(lexer->text[lexer->pos])){
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
                 Token token = { TOKEN_CHAR };
                 token.char_value = n.string_value[1];
                 lexer->current_token = token;
                 lexer->string_status = false;
                 lexer->char_status = true;
                 free(n.string_value);
                 return token;
              }else{
                printf("Error de declaracion de caracter: %s\n", n.string_value);
                exit(0);     
              }
           }else{
              Token token = { TOKEN_CHAR };
              token.char_value = n.string_value[0];
              lexer->current_token = token;
              lexer->string_status = false;
              lexer->char_status = true;
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

    if(is_variable_value(lexer)){
       return get_variable_value(lexer, true);
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
        case '~': token.type = TOKEN_CCH; break;
        default: token.type = TOKEN_INVALID; break;
    }

    lexer->current_token = token;
    return token;
}

bool isChar(char c){
    if(c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' || c == 'g' || c == 'h' || c == 'i' || c == 'j' || c == 'k' || 
       c == 'l' || c == 'm' || c == 'n' || c == 'o' || c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' || c == 'u' || c == 'v' || 
       c == 'w' || c == 'x' || c == 'y' || c == 'z' || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'G' || 
       c == 'H' || c == 'I' || c == 'J' || c == 'K' || c == 'L' || c == 'M' || c == 'N' || c == 'O' || c == 'P' || c == 'Q' || c == 'R' || 
       c == 'S' || c == 'T' || c == 'U' || c == 'V' || c == 'W' || c == 'X' || c == 'Y' || c == 'Z'){
        return true;
    }else{
        return false;
    }
}

Token get_number_token(Lexer* lexer) {
    char buffer[64];
    size_t length = 0;
    bool iscom = false;
    while(isdigit(lexer->text[lexer->pos]) || lexer->text[lexer->pos] == '.' || lexer->text[lexer->pos] == '_' || isChar(lexer->text[lexer->pos])){
          if(iscom){
             if(isdigit(lexer->text[lexer->pos])){
                buffer[length++] = lexer->text[lexer->pos++];
             }else if(lexer->text[lexer->pos] == '.'){
                fprintf(stderr, "Error de sintaxis se repite el punto flotante en el valor numerico\n");
                exit(0);
             }else{
                fprintf(stderr, "Error en la declaracion de el valor numerico, contiene caracteres.\n");
                exit(0);
             }
          }else{
            if(lexer->text[lexer->pos] == '.'){
               buffer[length++] = lexer->text[lexer->pos++];
               iscom = true;
            }else if(isdigit(lexer->text[lexer->pos])){ 
               buffer[length++] = lexer->text[lexer->pos++];
            }else{
               fprintf(stderr, "Error en la declaracion de el valor numerico, contiene caracteres.\n");
               exit(0);
            }
          }
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
    if(token.type == TOKEN_CHAR){
        get_next_token(lexer);
        return token;
    }else if(token.type == TOKEN_NUMBER) {
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
        fprintf(stderr, "(1) Error: token inesperado en la expresión: %d --> %s\n", token.type, lexer->text);
        exit(EXIT_FAILURE);
    }
}

Token parse_unary(Lexer* lexer) {
    Token token = lexer->current_token;
    if(token.type == TOKEN_MINUS || token.type == TOKEN_PLUS || token.type == TOKEN_NOT || token.type == TOKEN_CCH) {
        get_next_token(lexer);
        Token operand = parse_unary(lexer);
        if (token.type == TOKEN_MINUS) {
            if(operand.type == TOKEN_NUMBER){
               operand.number_value = -operand.number_value;
            }else if(operand.type == TOKEN_CHAR){
               operand.number_value = -(double)((int)operand.char_value);
               operand.type = TOKEN_NUMBER;
               lexer->char_status = false;
            }else{
               fprintf(stderr, "Incompatible symbol - for string values\n");
               exit(0);
            }
        } else if (token.type == TOKEN_PLUS){
            if(operand.type == TOKEN_NUMBER){
               operand.number_value = +operand.number_value;
            }else if(operand.type == TOKEN_CHAR){
               operand.number_value = +(double)((int)operand.char_value);
               operand.type = TOKEN_NUMBER;
               lexer->char_status = false;
            }else{
               fprintf(stderr, "Incompatible symbol + for string values\n");
               exit(0);
            }
        }else if (token.type == TOKEN_NOT){
            if(operand.type == TOKEN_NUMBER){
               operand.number_value = !operand.number_value;
            }else if(operand.type == TOKEN_CHAR){
               operand.number_value = !(double)((int)operand.char_value);
               operand.type = TOKEN_NUMBER;
               lexer->char_status = false;
            }else{
               fprintf(stderr, "Incompatible symbol ! for string values\n");
               exit(0);
            }
        }else if(token.type == TOKEN_CCH){
            if(operand.type == TOKEN_NUMBER){
               operand.number_value = (double)~((long)operand.number_value);
            }else if(operand.type == TOKEN_CHAR){
               operand.number_value = (double)~((long)operand.char_value);
               operand.type = TOKEN_NUMBER;
               lexer->char_status = false;
            }else{
               fprintf(stderr, "Incompatible symbol ~ for string values\n");
               exit(0);
            }
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
            if(left.type == TOKEN_STRING){
               if(right.type == TOKEN_NUMBER){
                   //"cadena" * 1
                   if(right.number_value > 0){
                      char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) * (int)right.number_value + 1));
                      memset(strn, '\0', sizeof(char) * (strlen(left.string_value) * (int)right.number_value + 1));
                      long u0 = 0;
                      for(int x = 0; x < (int)right.number_value; x++){
                          for(int y = 0; y < strlen(left.string_value); y++){
                              strn[u0] = left.string_value[y];
                              u0++;
                          }
                      }
                      free(left.string_value);
                      left.string_value = strn;
                      left.type = TOKEN_STRING;
                      lexer->char_status = false;
                      lexer->string_status = true;
                   }
               }else if(right.type == TOKEN_CHAR){
                   //"cadena" * 'a'
                   char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) * 2 + 1));
                   memset(strn, '\0', sizeof(char) * (strlen(left.string_value) * 2 + 1));
                   long u0 = 0;
                   for(int x = 0; x < strlen(left.string_value); x++){
                       strn[u0] = left.string_value[x];
                       u0++;
                       if(x < (strlen(left.string_value) - 1)){
                          strn[u0] = right.char_value;
                          u0++;
                       }
                   }
                   free(left.string_value);
                   left.string_value = strn;
                   left.type = TOKEN_STRING;
                   lexer->char_status = false;
                   lexer->string_status = true;
               }else{
                  //"cadena" * "cadena"
                  char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) + (strlen(right.string_value) * (strlen(left.string_value) - 1)) + 1));
                  memset(strn, '\0', sizeof(char) * (strlen(left.string_value) + (strlen(right.string_value) * (strlen(left.string_value) - 1)) + 1));
                  long u0 = 0;
                  for(int x = 0; x < strlen(left.string_value); x++){
                      strn[u0] = left.string_value[x];
                      u0++;
                      if(x != (strlen(left.string_value) - 1)){
                         for(int y = 0; y < strlen(right.string_value); y++){
                             strn[u0] = right.string_value[y];
                             u0++;
                         }
                      }
                  }
                  free(right.string_value);
                  right.string_value = NULL;
                  free(left.string_value);
                  left.string_value = strn;
                  left.type = TOKEN_STRING;
                  lexer->char_status = false;
                  lexer->string_status = true;
               }
            }else if(left.type == TOKEN_CHAR){
               if(right.type == TOKEN_NUMBER){
                  //'a' * 1
                  char nc = (char)((int)((double)(int)left.char_value * right.number_value));
                  left.char_value = nc;
                  left.type = TOKEN_CHAR;
                  lexer->string_status = false;
                  lexer->char_status = true;
               }else if(right.type == TOKEN_CHAR){
                  // 'a' * 'b'
                  char nc = (char)((int)((double)(int)left.char_value * (double)(int)right.char_value));
                  left.char_value = nc;
                  left.type = TOKEN_CHAR;
                  lexer->string_status = false;
                  lexer->char_status = true;
               }else{
                   //'a' * "cadena"
                   char *strn = (char *)malloc(sizeof(char) * (strlen(right.string_value) * 2 + 1));
                   memset(strn, '\0', sizeof(char) * (strlen(right.string_value) * 2 + 1));
                   long u0 = 0;
                   for(int x = 0; x < strlen(right.string_value); x++){
                       strn[u0] = right.string_value[x];
                       u0++;
                       if(x < (strlen(right.string_value) - 1)){
                          strn[u0] = left.char_value;
                          u0++;
                       }
                   }
                   free(right.string_value);
                   right.string_value = NULL;
                   left.string_value = strn;
                   left.type = TOKEN_STRING;
                   lexer->char_status = false;
                   lexer->string_status = true;
               }
            }else{
               if(right.type == TOKEN_NUMBER){
                  // 1 * 2
                  left.number_value *= right.number_value;
                  left.type = TOKEN_NUMBER;
                  lexer->char_status = false;
                  lexer->string_status = false;
               }else if(right.type == TOKEN_CHAR){
                   // 1 * 'a'
                  left.number_value *= (double)(int)right.char_value;
                  left.type = TOKEN_NUMBER;
                  lexer->char_status = false;
                  lexer->string_status = false;
               }else{
                   // 1 * "cadena"
                   if(right.number_value > 0){
                      char *strn = (char *)malloc(sizeof(char) * (strlen(right.string_value) * (int)left.number_value + 1));
                      memset(strn, '\0', sizeof(char) * (strlen(right.string_value) * (int)left.number_value + 1));
                      long u0 = 0;
                      for(int x = 0; x < (int)left.number_value; x++){
                          for(int y = 0; y < strlen(right.string_value); y++){
                              strn[u0] = right.string_value[y];
                              u0++;
                          }
                      }
                      free(right.string_value);
                      right.string_value = NULL;
                      left.string_value = strn;
                      left.type = TOKEN_STRING;
                      lexer->char_status = false;
                      lexer->string_status = true;
                   }
               }
            }
        } else if (operator == TOKEN_DIVIDE) {
            if(left.type == TOKEN_STRING){
               if(right.type == TOKEN_NUMBER){
                   //"cadena" / 1
                   int len = (strlen(left.string_value) / (int)right.number_value);
                   char *strn = (char *)malloc(sizeof(char) * (len + 1));
                   memset(strn, '\0', sizeof(char) * (len + 1));
                   for(int x = 0; x < len; x++){
                       strn[x] = left.string_value[x];
                   }
                   free(left.string_value);
                   left.string_value = strn;
                   lexer->char_status = false;
                   lexer->string_status = true;
                   left.type = TOKEN_STRING;
               }else if(right.type == TOKEN_CHAR){
                   //"cadena" / 'a'
                   char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) + 1));
                   memset(strn, '\0', sizeof(char) * (strlen(left.string_value) + 1));
                   bool err = false;
                   int u0 = 0;
                   for(int x = 0; x < strlen(left.string_value); x++){
                       char c0 = left.string_value[x];
                       if(c0 == right.char_value){
                          err = true;
                          x++;
                          for(int y = x; y < strlen(left.string_value); y++){
                              char c1 = left.string_value[y];
                              if(c1 != right.char_value){
                                 strn[u0] = c1;
                                 u0++;
                                 x++;
                              }else{
                                 err = false;
                                 break;
                              }
                          }

                          if(err){
                             fprintf(stderr, "Error no se cerro fragment con el simbolo %c\n", right.char_value);
                             exit(0);  
                          }
                       }
                   }

                   free(left.string_value);
                   left.string_value = strn;
                   left.type = TOKEN_STRING;
                   lexer->char_status = false;
                   lexer->string_status = true;
               }else{
                  //"cadena" / "cadena"
                  char *separator = (char *)malloc(sizeof(char) * (strlen(right.string_value) + 1));
                  memset(separator, '\0', sizeof(char) * (strlen(right.string_value) + 1));
                  char *sidx = (char *)malloc(sizeof(char) * (strlen(right.string_value) + 1));
                  memset(sidx, '\0', sizeof(char) * (strlen(right.string_value) + 1));
                  bool isn = false;
                  int u0 = 0, u1 = 0;
                  for(int x = 0; x < strlen(right.string_value); x++){
                      char c0 = right.string_value[x];
                      if(isn){
                         sidx[u1] = c0;
                         u1++;
                      }else{
                         if(c0 != '$'){
                            separator[u0] = c0;
                            u0++;
                         }else{
                            isn = true;
                         }
                      }
                  }

                  if(isn){
                     int idx = atoi(sidx);
                     int pos = 0;
                     char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) + 1));
                     memset(strn, '\0', sizeof(char) * (strlen(left.string_value) + 1));
                     int u0 = 0;
                     for(int v = 0; v < strlen(left.string_value); v++){
                         char c0 = left.string_value[v];
                         if(c0 == separator[0]){
                            bool isAv = true;
                            for(int o = 0, q = v; o < strlen(separator); o++, q++){
                                char p0 = separator[o];
                                char p1 = left.string_value[q];
                                if(p0 != p1){
                                   isAv = false;
                                }
                            }

                            if(isAv){
                               if(idx == pos){
                                  break;
                               }else{
                                  memset(strn, '\0', sizeof(char) * (strlen(left.string_value) + 1));
                                  u0 = 0;
                                  v += (strlen(separator) - 1);
                                  pos++;
                               }
                            }else{
                               strn[u0] = c0;
                               u0++;    
                            }
                         }else{
                            strn[u0] = c0;
                            u0++;
                         }
                     }
                     free(right.string_value);
                     right.string_value = NULL;
                     free(left.string_value);
                     left.string_value = strn;
                     left.type = TOKEN_STRING;
                     lexer->char_status = false;
                     lexer->string_status = true;
                  }else{
                    fprintf(stderr, "Syntax error not '$' symbol in string.\n");
                    exit(0);
                  }
               }
            }else if(left.type == TOKEN_CHAR){
               if(right.type == TOKEN_NUMBER){
                  //'a' / 1
                  char nc = (char)((int)((double)(int)left.char_value / right.number_value));
                  left.char_value = nc;
                  left.type = TOKEN_CHAR;
                  lexer->string_status = false;
                  lexer->char_status = true;
               }else if(right.type == TOKEN_CHAR){
                  // 'a' / 'b'
                  char nc = (char)((int)((double)(int)left.char_value / (double)(int)right.number_value));
                  left.char_value = nc;
                  left.type = TOKEN_CHAR;
                  lexer->string_status = false;
                  lexer->char_status = true;
               }else{
                //'a' / "cadena"
                char *strn = (char *)malloc(sizeof(char) * (strlen(right.string_value) + 1));
                memset(strn, '\0', sizeof(char) * (strlen(right.string_value) + 1));
                bool err = false;
                int u0 = 0;
                for(int x = 0; x < strlen(right.string_value); x++){
                    char c0 = right.string_value[x];
                    if(c0 == left.char_value){
                       err = true;
                       x++;
                       for(int y = x; y < strlen(right.string_value); y++){
                           char c1 = right.string_value[y];
                           if(c1 != left.char_value){
                              strn[u0] = c1;
                              u0++;
                              x++;
                           }else{
                              err = false;
                              break;
                           }
                       }
                       if(err){
                          fprintf(stderr, "Error no se cerro fragment con el simbolo %c\n", right.char_value);
                          exit(0);  
                       }
                    }
                }
                free(right.string_value);
                right.string_value = NULL;
                left.string_value = strn;
                left.type = TOKEN_STRING;
                lexer->char_status = false;
                lexer->string_status = true;
               }
            }else{
               if(right.type == TOKEN_NUMBER){
                  // 1 / 2
                  left.number_value /= right.number_value;
                  left.type = TOKEN_NUMBER;
                  lexer->string_status = false;
                  lexer->char_status = false;
               }else if(right.type == TOKEN_CHAR){
                   // 1 / 'a'
                   left.number_value /= (double)right.char_value;
                   left.type = TOKEN_NUMBER;
                   lexer->string_status = false;
                   lexer->char_status = false;
               }else{
                   // 1 / "cadena"
                   int len = (strlen(right.string_value) / (int)left.number_value);
                   char *strn = (char *)malloc(sizeof(char) * (len + 1));
                   memset(strn, '\0', sizeof(char) * (len + 1));
                   for(int x = 0; x < len; x++){
                       strn[x] = right.string_value[x];
                   }
                   free(right.string_value);
                   right.string_value = NULL;
                   left.string_value = strn;
                   lexer->char_status = false;
                   lexer->string_status = true;
                   left.type = TOKEN_STRING;
               }
            }
        } else if (operator == TOKEN_MOD) {//%
            if(left.type == TOKEN_STRING){
               if(right.type == TOKEN_NUMBER){
                   //"cadena" % 1
                   int len = (int)((double)(strlen(left.string_value) / 100.0) * (int)right.number_value);
                   if(len <= strlen(left.string_value)){
                      char *strn = (char *)malloc(sizeof(char) * (len + 1));
                      memset(strn, '\0', sizeof(char) * (len + 1));
                      for(int x = 0; x < len; x++){
                          strn[x] = left.string_value[x];
                      }
                      free(left.string_value);
                      left.string_value = strn;
                      left.type = TOKEN_STRING;
                      lexer->char_status = false;
                      lexer->string_status = true;
                   }else{
                      fprintf(stderr, "Error porcentaje fuera de rango 0-100.\n");
                      exit(0);
                   }
               }else if(right.type == TOKEN_CHAR){
                   //"cadena" % 'a'
                   int nc = 0;
                   for(int x = 0; x < strlen(left.string_value); x++){
                       char c0 = left.string_value[x];
                       if(c0 == right.char_value){
                          nc++;
                       }
                   }

                   double porc = ((double)strlen(left.string_value) / 100.0) * (double)nc;
                   free(left.string_value);
                   left.string_value = NULL;
                   left.number_value = porc;
                   left.type = TOKEN_NUMBER;
                   lexer->char_status = false;
                   lexer->string_status = false;
               }else{
                  //"cadena" % "cadena"
                  long words = 0;
                  for(int x = 0; x < strlen(left.string_value); x++){
                      char c0 = left.string_value[x];
                      if(c0 == ' ' || c0 == '\t' || x == (strlen(left.string_value) - 1)){
                         words++;
                      }
                  }

                  long coins = 0;
                  char *buf = (char *)malloc(sizeof(char) * strlen(left.string_value) + 1);
                  memset(buf, '\0', sizeof(char) * strlen(left.string_value) + 1);
                  long u0 = 0;
                  for(int a = 0; a < strlen(right.string_value); a++){
                      char c0 = right.string_value[a];
                      if(c0 == ' ' || c0 == '\t' || a == (strlen(right.string_value) - 1)){
                         if(a == (strlen(right.string_value) - 1)){
                            buf[u0] = c0;
                            u0++;   
                         }
                         a++;
                         char *val = (char *)malloc(sizeof(char) * (strlen(right.string_value) + 1));
                         memset(val, '\0', sizeof(char) * (strlen(right.string_value) + 1));
                         long u1 = 0;
                         for(int b = 0; b < strlen(left.string_value); b++){
                             char p0 = left.string_value[b];
                             if(p0 == ' ' || p0 == '\t' || b == (strlen(left.string_value) - 1)){
                                if(b == (strlen(left.string_value) - 1)){    
                                   val[u1] = p0;
                                   u1++;
                                }
                                
                                if(strcmp(buf, val) == 0){
                                   coins++;
                                   memset(val, '\0', sizeof(char) * (strlen(right.string_value) + 1));
                                   u1 = 0;
                                }else{
                                   memset(val, '\0', sizeof(char) * (strlen(right.string_value) + 1));
                                   u1 = 0;
                                }
                             }else{
                                val[u1] = p0;
                                u1++;
                             }
                         }
                         free(val);
                         val = NULL;
                         memset(buf, '\0', sizeof(char) * strlen(left.string_value) + 1);
                         u0 = 0;
                      }else{
                         buf[u0] = c0;
                         u0++;
                      }
                  }
                  free(buf);

                  char *c = (char *)malloc(sizeof(char) * 4096);
                  memset(c, '\0', sizeof(char) * 4096);
                  char *w = (char *)malloc(sizeof(char) * 4096);
                  memset(w, '\0', sizeof(char) * 4096);
                  char *p = (char *)malloc(sizeof(char) * 4096);
                  memset(p, '\0', sizeof(char) * 4096);
                  
                  c = dtoa(c, (double)coins);
                  w = dtoa(w, (double)words);
                  p = dtoa(p, (double)((100.0 / (double)words) * (double)coins));

                  char *strn = (char *)malloc(sizeof(char) * (6 + strlen(c) + strlen(w) + strlen(p) + 10));
                  memset(strn, '\0', sizeof(char) * (6 + strlen(c) + strlen(w) + strlen(p) + 10));
                  strcpy(strn, "C:");
                  strcat(strn, c);
                  free(c);
                  strcat(strn, "|W:");
                  strcat(strn, w);
                  free(w);
                  strcat(strn, "|P:");
                  strcat(strn, p);
                  free(p);
                  free(right.string_value);
                  right.string_value = NULL;
                  free(left.string_value);
                  left.string_value = strn;
                  left.type = TOKEN_STRING;
                  lexer->char_status = false;
                  lexer->string_status = true;
               }
            }else if(left.type == TOKEN_CHAR){
               if(right.type == TOKEN_NUMBER){
                  //'a' % 1
                  left.char_value = (char)(int)my_fmod((double)left.char_value, right.number_value); // fmod es como param1 % param2 pero para double's
                  left.type = TOKEN_CHAR;
                  lexer->char_status = true;
                  lexer->string_status = false;
               }else if(right.type == TOKEN_CHAR){
                  // 'a' % 'b'
                  left.char_value = (char)(int)my_fmod((double)left.char_value, (double)right.char_value); // fmod es como param1 % param2 pero para double's
                  left.type = TOKEN_CHAR;
                  lexer->char_status = true;
                  lexer->string_status = false;
               }else{
                 //'a' % "cadena"
                 int nc = 0;
                   for(int x = 0; x < strlen(right.string_value); x++){
                       char c0 = right.string_value[x];
                       if(c0 == left.char_value){
                          nc++;
                       }
                   }

                   double porc = ((double)strlen(right.string_value) / 100.0) * (double)nc;
                   free(right.string_value);
                   right.string_value = NULL;
                   left.number_value = porc;
                   left.type = TOKEN_NUMBER;
                   lexer->char_status = false;
                   lexer->string_status = false;
               }
            }else{
               if(right.type == TOKEN_NUMBER){
                  // 1 % 2
                  left.number_value = my_fmod(left.number_value, right.number_value); // fmod es como param1 % param2 pero para double's
                  left.type = TOKEN_NUMBER;
                  lexer->char_status = false;
                  lexer->string_status = false;
               }else if(right.type == TOKEN_CHAR){
                   // 1 % 'a'
                   left.number_value = my_fmod(left.number_value, (double)right.char_value); // fmod es como param1 % param2 pero para double's
                   left.type = TOKEN_NUMBER;
                   lexer->char_status = false;
                   lexer->string_status = false;
               }else{
                   // 1 % "cadena"
                   int len = (int)((double)(strlen(right.string_value) / 100.0) * (int)left.number_value);
                   if(len <= strlen(right.string_value)){
                      char *strn = (char *)malloc(sizeof(char) * (len + 1));
                      memset(strn, '\0', sizeof(char) * (len + 1));
                      for(int x = 0; x < len; x++){
                          strn[x] = right.string_value[x];
                      }
                      free(right.string_value);
                      right.string_value = NULL;
                      left.string_value = strn;
                      left.type = TOKEN_STRING;
                      lexer->char_status = false;
                      lexer->string_status = true;
                   }else{
                      fprintf(stderr, "Error porcentaje fuera de rango 0-100.\n");
                      exit(0);
                   }
               }
            }
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
                  lexer->char_status = false;
                  left.type = TOKEN_STRING;
                  free(left.string_value);
                  left.string_value = NULL;
                  left.string_value = strn;
               }else if(right.type == TOKEN_CHAR){
                  char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) + 2));
                  memset(strn, '\0', sizeof(char) * (strlen(left.string_value) + 2));
                  strcpy(strn, left.string_value);
                  strn[strlen(left.string_value)] = right.char_value;
                  free(left.string_value);
                  left.string_value = NULL;
                  lexer->string_status = true;
                  lexer->char_status = false;
                  left.type = TOKEN_STRING;
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
                  lexer->char_status = false;
               }
            }else if(left.type == TOKEN_CHAR){
                if(right.type == TOKEN_NUMBER){
                   left.char_value = (int)((int)left.char_value + (int)right.number_value);
                   left.type = TOKEN_CHAR;
                   lexer->string_status = false;
                   lexer->char_status = true;
                }else if(right.type == TOKEN_CHAR){
                   left.char_value = (int)((int)left.char_value + (int)right.char_value);
                   left.type = TOKEN_CHAR;
                   lexer->string_status = false;
                   lexer->char_status = true;
                }else{
                   char *strn = (char *)malloc(sizeof(char) * (strlen(right.string_value) + 2));
                   memset(strn, '\0', sizeof(char) * (strlen(right.string_value) + 2));
                   strn[0] = (char)left.char_value;
                   strcat(strn, right.string_value);
                   free(right.string_value);
                   left.string_value = strn;
                   left.type = TOKEN_STRING;
                   lexer->string_status = true;
                   lexer->char_status = false;
                }
            }else{
                if(right.type == TOKEN_NUMBER){
                   left.number_value += right.number_value;
                   left.type = TOKEN_NUMBER;
                   lexer->string_status = false;
                   lexer->char_status = false;
                }else if(right.type == TOKEN_CHAR){
                   left.number_value += (int)right.char_value;
                   left.type = TOKEN_NUMBER;
                   lexer->string_status = false;
                   lexer->char_status = false;
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
                   lexer->char_status = false;
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
                  lexer->char_status = false;
                  left.type = TOKEN_STRING;
               }else if(right.type == TOKEN_CHAR){
                  char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) + 1));
                  memset(strn, '\0', sizeof(char) * (strlen(left.string_value) + 1));
                  int u = 0;
                  for(int x = 0; x < strlen(left.string_value); x++){
                      char c0 = left.string_value[x];
                      if(c0 != (char)right.char_value){
                         strn[u] = c0;
                         u++;
                      }
                  }
                  free(left.string_value);
                  left.string_value = NULL;
                  left.string_value = strn;
                  lexer->string_status = true;
                  lexer->char_status = false;
                  left.type = TOKEN_STRING;
               }else{
                   char *strn = (char *)malloc(sizeof(char) * (strlen(left.string_value) + 1));
                   memset(strn, '\0', sizeof(char) * (strlen(left.string_value) + 1));
                   int u = 0;
                   for(int x = 0; x < strlen(left.string_value); x++){
                       char c = left.string_value[x];
                       if(c == right.string_value[0]){
                          bool inc = true;
                          for(int l = 0, k = x; l < strlen(right.string_value); l++, k++){
                              char p0 = right.string_value[l];
                              char p1 = left.string_value[k];
                              if(p0 != p1){
                                 inc = false;
                                 break;
                              }
                          }

                          if(inc){
                             x += (strlen(right.string_value) - 1); 
                          }else{
                            strn[u] = c;
                            u++;
                          }
                       }else{
                          strn[u] = c;
                          u++;
                       }
                   }
                   free(right.string_value);
                   right.string_value = NULL;
                   free(left.string_value);
                   left.string_value = NULL;
                   lexer->string_status = true;
                   lexer->char_status = false;
                   left.type = TOKEN_STRING;
                   left.string_value = strn;
               }
            }else if(left.type == TOKEN_CHAR){
                if(right.type == TOKEN_NUMBER){
                   left.char_value = (int)((int)left.char_value - (int)right.number_value);
                   left.type = TOKEN_CHAR;
                   lexer->string_status = false;
                   lexer->char_status = true;
                }else if(right.type == TOKEN_CHAR){
                   left.char_value = (int)((int)left.char_value - (int)right.char_value);
                   left.type = TOKEN_CHAR;
                   lexer->string_status = false;
                   lexer->char_status = true;
                }else{
                   char *strn = (char *)malloc(sizeof(char) * (strlen(right.string_value) + 1));
                   memset(strn, '\0', sizeof(char) * (strlen(right.string_value) + 1));
                   int u = 0;
                   for(int x = 0; x < strlen(right.string_value); x++){
                       char c0 = right.string_value[x];
                       if(c0 == (char)left.char_value){
                          strn[u] = c0;
                          u++;
                       }
                   }
                   free(right.string_value);
                   right.string_value = NULL;
                   lexer->string_status = true;
                   lexer->char_status = false;
                   left.type = TOKEN_STRING;
                   left.string_value = strn;
                }
            }else{
               if(right.type == TOKEN_NUMBER){
                  left.number_value -= right.number_value;
                  left.type = TOKEN_NUMBER;
                  lexer->string_status = false;
                  lexer->char_status = false;
               }else if(right.type == TOKEN_CHAR){
                  left.number_value -= (int)right.char_value;
                  left.type = TOKEN_NUMBER;
                  lexer->string_status = false;
                  lexer->char_status = false;
                  lexer->char_status = false;
               }else{
                  char *strn = (char *)malloc(sizeof(char) * (strlen(right.string_value) + 1));
                  memset(strn, '\0', sizeof(char) * (strlen(right.string_value) + 1));
                  int u = 0;
                  for(int x = left.number_value; x < strlen(right.string_value); x++){
                      strn[u] = right.string_value[x];
                      u++;
                  }
                  free(right.string_value);
                  right.string_value = NULL;
                  left.type = TOKEN_STRING;
                  lexer->string_status = true;
                  lexer->char_status = false;
                  left.string_value = strn;
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
                left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.type == TOKEN_CHAR) ? ((int)left.char_value) : (left.string_value ? strlen(left.string_value) : 0)) < ((right.type == TOKEN_NUMBER) ? right.number_value : (right.type == TOKEN_CHAR) ? ((int)right.char_value) : (right.string_value ? strlen(right.string_value) : 0));
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                lexer->char_status = false;
                break;
            } 

            case TOKEN_GT: {
                left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.type == TOKEN_CHAR) ? ((int)left.char_value) : (left.string_value ? strlen(left.string_value) : 0)) > ((right.type == TOKEN_NUMBER) ? right.number_value : (right.type == TOKEN_CHAR) ? ((int)right.char_value) : (right.string_value ? strlen(right.string_value) : 0));
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                lexer->char_status = false;
                break;
            }

            case TOKEN_LE: {
                left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.type == TOKEN_CHAR) ? ((int)left.char_value) : (left.string_value ? strlen(left.string_value) : 0)) <= ((right.type == TOKEN_NUMBER) ? right.number_value : (right.type == TOKEN_CHAR) ? ((int)right.char_value) : (right.string_value ? strlen(right.string_value) : 0));
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                lexer->char_status = false;
                break;
            }

            case TOKEN_GE: {
                left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.type == TOKEN_CHAR) ? ((int)left.char_value) : (left.string_value ? strlen(left.string_value) : 0)) >= ((right.type == TOKEN_NUMBER) ? right.number_value : (right.type == TOKEN_CHAR) ? ((int)right.char_value) : (right.string_value ? strlen(right.string_value) : 0));
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                lexer->char_status = false;
                break;
            }

            case TOKEN_EQ: {
                if((left.type == TOKEN_NUMBER && right.type == TOKEN_NUMBER) || (left.type == TOKEN_NUMBER && right.type == TOKEN_STRING) || (left.type == TOKEN_STRING && right.type == TOKEN_NUMBER) || 
                   (left.type == TOKEN_CHAR && right.type == TOKEN_CHAR) || (left.type == TOKEN_NUMBER && right.type == TOKEN_CHAR) || (left.type == TOKEN_CHAR && right.type == TOKEN_NUMBER) || 
                   (left.type == TOKEN_STRING && right.type == TOKEN_NUMBER) || (left.type == TOKEN_NUMBER && right.type == TOKEN_STRING)){
                    left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.type == TOKEN_CHAR) ? ((int)left.char_value) : (left.string_value ? strlen(left.string_value) : 0)) == ((right.type == TOKEN_NUMBER) ? right.number_value : (right.type == TOKEN_CHAR) ? ((int)right.char_value) : (right.string_value ? strlen(right.string_value) : 0));
                }else if(left.type == TOKEN_STRING && right.type == TOKEN_STRING){
                    left.number_value = (left.string_value && right.string_value) ? (strcmp(left.string_value, right.string_value) == 0) : 0;
                }
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                lexer->char_status = false;
                break;
            }

            case TOKEN_NE: {
                if((left.type == TOKEN_NUMBER && right.type == TOKEN_NUMBER) || (left.type == TOKEN_NUMBER && right.type == TOKEN_STRING) || (left.type == TOKEN_STRING && right.type == TOKEN_NUMBER) || 
                   (left.type == TOKEN_CHAR && right.type == TOKEN_CHAR) || (left.type == TOKEN_NUMBER && right.type == TOKEN_CHAR) || (left.type == TOKEN_CHAR && right.type == TOKEN_NUMBER) || 
                   (left.type == TOKEN_STRING && right.type == TOKEN_NUMBER) || (left.type == TOKEN_NUMBER && right.type == TOKEN_STRING)){
                    left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.type == TOKEN_CHAR) ? ((int)left.char_value) : (left.string_value ? strlen(left.string_value) : 0)) != ((right.type == TOKEN_NUMBER) ? right.number_value : (right.type == TOKEN_CHAR) ? ((int)right.char_value) : (right.string_value ? strlen(right.string_value) : 0));
                }else if(left.type == TOKEN_STRING && right.type == TOKEN_STRING){
                    left.number_value = (left.string_value && right.string_value) ? (strcmp(left.string_value, right.string_value) != 0) : 0;
                }
                left.type = TOKEN_NUMBER;
                lexer->string_status = false;
                lexer->char_status = false;
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
        left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.type == TOKEN_CHAR) ? ((int)left.char_value) : (left.string_value ? 1 : 0)) && ((right.type == TOKEN_NUMBER) ? right.number_value : (right.type == TOKEN_CHAR) ? ((int)right.char_value) : (right.string_value ? 1 : 0));
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
        left.number_value = ((left.type == TOKEN_NUMBER) ? left.number_value : (left.type == TOKEN_CHAR) ? ((int)left.char_value) : (left.string_value ? 1 : 0)) || ((right.type == TOKEN_NUMBER) ? right.number_value : (right.type == TOKEN_CHAR) ? ((int)right.char_value) : (right.string_value ? 1 : 0));
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
    //XOR, SHR, SHL, AND, OR
    while(lexer->current_token.type == TOKEN_QUESTION || lexer->current_token.type == TOKEN_XOR || lexer->current_token.type == TOKEN_SHL || 
          lexer->current_token.type == TOKEN_SHR || lexer->current_token.type == TOKEN_AND || lexer->current_token.type == TOKEN_OR){
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
                    lexer->char_status = false;
                 }else if(condition.type == TOKEN_CHAR){
                    lexer->string_status = false;
                    lexer->char_status = true;
                 }else{
                    lexer->string_status = false;
                    lexer->char_status = false;
                 }
              }else if(condition.type == TOKEN_NUMBER){
                 condition = condition.number_value ? true_expr : false_expr;
                 if(condition.type == TOKEN_STRING){
                    lexer->string_status = true;
                    lexer->char_status = false;
                 }else if(condition.type == TOKEN_CHAR){
                    lexer->string_status = false;
                    lexer->char_status = true;
                 }else{
                    lexer->string_status = false;
                    lexer->char_status = false;
                 }
              }else if(condition.type == TOKEN_CHAR){
                 condition = ((int)condition.char_value) ? true_expr : false_expr;
                 if(condition.type == TOKEN_STRING){
                    lexer->string_status = true;
                    lexer->char_status = false;
                 }else if(condition.type == TOKEN_CHAR){
                    lexer->string_status = false;
                    lexer->char_status = true;
                 }else{
                    lexer->string_status = false;
                    lexer->char_status = false;
                 }
              }else{
                  printf("(0) Error la condicion es invalida.\n");
                  exit(0);
              }
          }else if(lexer->current_token.type == TOKEN_XOR){
              Token val = parse_xor(lexer);
              if(condition.type == TOKEN_STRING){
                 fprintf(stderr, "Error el simbolo ^ no soporta strings para operar.\n");
                 exit(0);
              }else if(condition.type == TOKEN_CHAR){
                 if(val.type == TOKEN_NUMBER){
                    long long xor = (long long)condition.char_value ^ (long long)val.number_value;
                    condition.char_value = (int)xor;
                    condition.type = TOKEN_CHAR;
                    lexer->string_status = false;
                    lexer->char_status = true;
                 }else if(val.type == TOKEN_CHAR){
                    long long xor = (long long)condition.char_value ^ (long long)val.char_value;
                    condition.char_value = (int)xor;
                    condition.type = TOKEN_CHAR;
                    lexer->string_status = false;
                    lexer->char_status = true;
                 }else{
                    fprintf(stderr, "Error el simbolo ^ no soporta strings para operar.\n");
                    exit(0);
                 }
              }else{
                 if(val.type == TOKEN_NUMBER){
                    long long xor = (long long)condition.number_value ^ (long long)val.number_value;
                    condition.number_value = (double)xor;
                    condition.type = TOKEN_NUMBER;
                    lexer->string_status = false;
                    lexer->char_status = false;
                 }else if(val.type == TOKEN_CHAR){
                    long long xor = (long long)condition.number_value ^ (long long)val.char_value;
                    condition.number_value = (double)xor;
                    condition.type = TOKEN_NUMBER;
                    lexer->string_status = false;
                    lexer->char_status = false;
                 }else{
                    fprintf(stderr, "Error el simbolo ^ no soporta strings para operar.\n");
                    exit(0);
                 }
              }
              
          }else if(lexer->current_token.type == TOKEN_SHL){
              // TOKEN_SHL <<
              Token val = parse_shl(lexer);
              if(condition.type == TOKEN_STRING){
                  if(val.type == TOKEN_NUMBER){
                     //"aa" << 1 
                     long long u = (strlen(condition.string_value) + (long)val.number_value);
                     free(condition.string_value);
                     condition.string_value = NULL;
                     condition.number_value = (double)u;
                     condition.type = TOKEN_NUMBER;
                     lexer->string_status = false;
                     lexer->char_status = false;
                  }else if(val.type == TOKEN_CHAR){
                     //"aa" << 'a'
                     long long u = 0;
                     for(int x = 0; x < strlen(condition.string_value); x++){
                         char c = condition.string_value[x];
                         if(c == val.char_value){
                            u++;
                         }
                     }
                     free(condition.string_value);
                     condition.string_value = NULL;
                     lexer->string_status = false;
                     lexer->char_status = false;
                     condition.type = TOKEN_NUMBER;
                     condition.number_value = (double)u;
                  }else{
                     //"aa" << "bb"
                     long long u = 0;
                     for(int x = 0; x < strlen(condition.string_value); x++){
                         char c = condition.string_value[x];
                         if(c == val.string_value[0]){
                            bool inc = true;
                            for(int l = 0, k = x; l < strlen(val.string_value); l++, k++){
                                char p0 = val.string_value[l];
                                char p1 = condition.string_value[k];
                                if(p0 != p1){
                                   inc = false;
                                   break;
                                }
                             }
      
                             if(inc){
                                x += (strlen(val.string_value) - 1); 
                                u++;
                             }
                         }
                     }
                     free(val.string_value);
                     val.string_value = NULL;
                     free(condition.string_value);
                     condition.string_value = NULL;
                     lexer->string_status = false;
                     lexer->char_status = false;
                     condition.type = TOKEN_NUMBER;
                     condition.number_value = (double)u;
                  }
              }else if(condition.type == TOKEN_CHAR){
                  if(val.type == TOKEN_NUMBER){
                     long l0 = (long)condition.char_value;
                     long l1 = (long)val.number_value;
                     long long val = (long long)(l0 << l1);
                     condition.char_value = (int)val;
                     condition.type = TOKEN_CHAR;
                     lexer->char_status = true;
                     lexer->string_status = false;
                  }else if(val.type == TOKEN_CHAR){
                     long l0 = (long)condition.char_value;
                     long l1 = (long)val.char_value;
                     long long val = (long long)(l0 << l1);
                     condition.char_value = (int)val;
                     condition.type = TOKEN_CHAR;
                     lexer->char_status = true;
                     lexer->string_status = false;
                  }else{
                     // 'a' << "aa" 
                     long long u = 0;
                     for(int x = 0; x < strlen(val.string_value); x++){
                         char c = val.string_value[x];
                         if(c == condition.char_value){
                            u++;
                         }
                     }
                     free(val.string_value);
                     val.string_value = NULL;
                     lexer->string_status = false;
                     lexer->char_status = false;
                     condition.type = TOKEN_NUMBER;
                     condition.number_value = u > 0 ? 0 : 1;
                  }
              }else{
                  if(val.type == TOKEN_NUMBER){
                     long l0 = (long)condition.number_value;
                     long l1 = (long)val.number_value;
                     long long val = (long long)(l0 << l1);
                     condition.number_value = (double)val;
                     condition.type = TOKEN_NUMBER;
                     lexer->char_status = false;
                     lexer->string_status = false;
                  }else if(val.type == TOKEN_CHAR){
                     // 1 << 'a'
                     long l0 = (long)condition.number_value;
                     long l1 = (long)val.char_value;
                     long long val = (long long)(l0 << l1);
                     condition.number_value = (double)val;
                     condition.type = TOKEN_NUMBER;
                     lexer->char_status = false;
                     lexer->string_status = false;
                  }else{
                     //1 << "aa"
                     long long u = (strlen(val.string_value) - (long)condition.number_value);
                     free(val.string_value);
                     val.string_value = NULL;
                     condition.number_value = (double)u;
                     condition.type = TOKEN_NUMBER;
                     lexer->string_status = false;
                     lexer->char_status = false;
                  }
              }
          }else if(lexer->current_token.type == TOKEN_SHR){
              // TOKEN_SHR >>
              Token val = parse_shr(lexer);
              if(condition.type == TOKEN_STRING){
                  if(val.type == TOKEN_NUMBER){
                    //"aa" >> 1
                    long long u = (strlen(condition.string_value) - (long)val.number_value);
                    free(condition.string_value);
                    condition.string_value = NULL;
                    condition.number_value = (double)u;
                    condition.type = TOKEN_NUMBER;
                    lexer->string_status = false;
                    lexer->char_status = false;
                  }else if(val.type == TOKEN_CHAR){
                    //"aa" >> 'a'
                    long long u = 0;
                    for(int x = 0; x < strlen(condition.string_value); x++){
                         char c = condition.string_value[x];
                         if(c == val.char_value){
                            u++;
                         }
                    }
                    free(condition.string_value);
                    condition.string_value = NULL;
                    lexer->string_status = false;
                    lexer->char_status = false;
                    condition.type = TOKEN_NUMBER;
                    condition.number_value = u > 0 ? 0 : 1;
                  }else{
                    //"aa" >> "bb"  
                    long long u = 0;
                     for(int x = 0; x < strlen(val.string_value); x++){
                         char c = val.string_value[x];
                         if(c == condition.string_value[0]){
                            bool inc = true;
                            for(int l = 0, k = x; l < strlen(condition.string_value); l++, k++){
                                char p0 = condition.string_value[l];
                                char p1 = val.string_value[k];
                                if(p0 != p1){
                                   inc = false;
                                   break;
                                }
                             }
      
                             if(inc){
                                x += (strlen(condition.string_value) - 1); 
                                u++;
                             }
                         }
                     }
                     free(val.string_value);
                     val.string_value = NULL;
                     free(condition.string_value);
                     condition.string_value = NULL;
                     lexer->string_status = false;
                     lexer->char_status = false;
                     condition.type = TOKEN_NUMBER;
                     condition.number_value = u > 0 ? 0 : 1;
                  }
              }else if(condition.type == TOKEN_CHAR){
                  if(val.type == TOKEN_NUMBER){
                     //'a' >> 1
                     long l0 = (long)val.number_value;
                     long l1 = (long)condition.char_value;
                     long long val = (long long)(l0 >> l1);
                     condition.number_value = (double)val;
                     condition.type = TOKEN_NUMBER;
                     lexer->char_status = false;
                     lexer->string_status = false;
                  }else if(val.type == TOKEN_CHAR){
                     //'a' >> 'b'
                     long l0 = (long)condition.char_value;
                     long l1 = (long)val.char_value;
                     long long val = (long long)(l0 >> l1);
                     condition.char_value = (int)val;
                     condition.type = TOKEN_CHAR;
                     lexer->char_status = true;
                     lexer->string_status = false;
                  }else{
                     //'a' >> "aa"
                     long long u = 0;
                     for(int x = 0; x < strlen(val.string_value); x++){
                         char c = val.string_value[x];
                         if(c == condition.char_value){
                            u++;
                         }
                     }
                     free(val.string_value);
                     val.string_value = NULL;
                     lexer->string_status = false;
                     lexer->char_status = false;
                     condition.type = TOKEN_NUMBER;
                     condition.number_value = (double)u;
                  }
              }else{
                  if(val.type == TOKEN_NUMBER){
                     //1 >> 2
                     long l0 = (long)condition.number_value;
                     long l1 = (long)val.number_value;
                     long long val = (long long)(l0 >> l1);
                     condition.number_value = (double)val;
                     condition.type = TOKEN_NUMBER;
                     lexer->char_status = false;
                     lexer->string_status = false;
                  }else if(val.type == TOKEN_CHAR){
                    //1 >> 'a'
                    long l0 = (long)val.char_value;
                    long l1 = (long)condition.number_value;
                    long long val = (long long)(l0 << l1);
                    condition.char_value = (int)val;
                    condition.type = TOKEN_CHAR;
                    lexer->char_status = true;
                    lexer->string_status = false;
                  }else{
                     //1 >> "aa"
                     long long u = (strlen(val.string_value) + (long)condition.number_value);
                     free(val.string_value);
                     val.string_value = NULL;
                     condition.number_value = (double)u;
                     condition.type = TOKEN_NUMBER;
                     lexer->string_status = false;
                     lexer->char_status = false;
                  }
              }
          }else if(lexer->current_token.type == TOKEN_AND){
              // TOKEN_AND &
              Token val = parse_and(lexer);
              if(condition.type == TOKEN_STRING){
                  if(val.type == TOKEN_NUMBER){
                     if(strlen(condition.string_value) > (int)val.number_value || strlen(condition.string_value) == (int)val.number_value){
                        char *strn = (char *)malloc(sizeof(char) * (strlen(condition.string_value) + 1));
                        memset(strn, '\0', sizeof(char) * (strlen(condition.string_value) + 1));
                        int u0 = 0;
                        for(int x = 0; x < (strlen(condition.string_value) - (int)val.number_value); x++){
                            strn[x] = condition.string_value[x];
                            u0++;
                        }
                        
                        int u1 = u0;
                        for(int y = (strlen(condition.string_value) - 1); y >= u0; y--){
                            strn[u1] = condition.string_value[y];
                            u1++;
                        }
                        free(condition.string_value);
                        condition.string_value = NULL;
                        condition.string_value = strn;
                        condition.type = TOKEN_STRING;
                        lexer->char_status = false;
                        lexer->string_status = true;
                     }
                  }else if(val.type == TOKEN_CHAR){
                     //"aaa" & 'a'
                     long result = 0;
                     for(int x = 0; x < strlen(condition.string_value); x++){
                         if(condition.string_value[x] == val.char_value){
                            result += (int)condition.string_value[x];
                         }
                     }
                     free(condition.string_value);
                     condition.string_value = NULL;
                     condition.number_value = (double)result;
                     condition.type = TOKEN_NUMBER;
                     lexer->string_status = false;
                     lexer->char_status = false;
                  }else{
                     char *alp = (char *)malloc(sizeof(char) * 4096);
                     char *ocl = (char *)malloc(sizeof(char) * strlen(val.string_value) + 2);
                     memset(ocl, '\0', sizeof(char) * strlen(val.string_value) + 2);
                     memset(alp, '\0', sizeof(char) * 4096);
                     bool isp = false;
                     long u0 = 0, u1 = 0;
                     for(int j = 0; j < strlen(val.string_value); j++){
                         char n = val.string_value[j];
                         if(isp){
                            alp[u1] = n;
                            u1++;
                         }else{
                            if(n != '$'){
                               ocl[u0] = n;
                               u0++;
                            }else{
                               isp = true;
                            }
                         }
                     }
                     if(isp){
                        int code = atoi(alp);
                        char *strn = (char *)malloc(sizeof(char) * (strlen(condition.string_value) + strlen(val.string_value) + code + 2));
                        memset(strn, '\0', sizeof(char) * (strlen(condition.string_value) + strlen(val.string_value) + code + 2));
                        strcpy(strn, condition.string_value);
                        int k = strlen(condition.string_value) + code;
   
                        for(int x = 0; x < strlen(ocl); x++){
                            strn[k] = ocl[x];
                            k++;
                        }
                        strn[k] = '\0';
                        free(condition.string_value);
                        condition.string_value = NULL;
                        free(val.string_value);
                        val.string_value = NULL;
                        condition.string_value = strn;
                        condition.type = TOKEN_STRING;
                        lexer->char_status = false;
                        lexer->string_status = true;
                     }else{
                         fprintf(stderr, "Syntax error not '$' symbol in string.\n");
                         exit(0);
                     }
                  }
              }else if(condition.type == TOKEN_CHAR){
                  if(val.type == TOKEN_NUMBER){
                     long long i = ((long)condition.char_value & (long)val.number_value);
                     condition.char_value = (char)i;
                     condition.char_value = true;
                     condition.string_value = false;
                     condition.type = TOKEN_CHAR;
                  }else if(val.type == TOKEN_CHAR){
                     long long i = ((long)condition.char_value & (long)val.char_value);
                     condition.char_value = (char)i;
                     condition.char_value = true;
                     condition.string_value = false;
                     condition.type = TOKEN_CHAR;
                  }else{
                     // 'a' & "aaa"
                     long result = 0;
                     for(int x = 0; x < strlen(val.string_value); x++){
                         if(val.string_value[x] == condition.char_value){
                            result += (int)val.string_value[x];
                         }
                     }
                     free(val.string_value);
                     val.string_value = NULL;
                     condition.number_value = (double)(-result);
                     condition.type = TOKEN_NUMBER;
                     lexer->string_status = false;
                     lexer->char_status = false;
                  }
              }else{
                  if(val.type == TOKEN_NUMBER){
                     long long i = ((long)condition.number_value & (long)val.number_value);
                     condition.number_value = (double)i;
                     condition.char_value = false;
                     condition.string_value = false;
                     condition.type = TOKEN_NUMBER;
                  }else if(val.type == TOKEN_CHAR){
                     long long i = ((long)condition.number_value & (long)val.char_value);
                     condition.number_value = (double)i;
                     condition.char_value = false;
                     condition.string_value = false;
                     condition.type = TOKEN_NUMBER;
                  }else{
                     if(strlen(val.string_value) > (int)condition.number_value){
                        char *strn = (char *)malloc(sizeof(char) * (strlen(val.string_value) + 1));
                        memset(strn, '\0', sizeof(char) * (strlen(val.string_value) + 1));
                        int u0 = 0;
                        for(int x = (int)condition.number_value - 1; x >= 0 ; x--){
                            strn[u0] = val.string_value[x];
                            u0++;
                        }
                        
                        int u1 = u0;
                        for(int y = u0; y < strlen(val.string_value); y++){
                            strn[u1] = val.string_value[y];
                            u1++;
                        }
                        free(val.string_value);
                        val.string_value = NULL;
                        condition.string_value = strn;
                        condition.type = TOKEN_STRING;
                        lexer->char_status = false;
                        lexer->string_status = true;
                     }
                  }
              }
          }else if(lexer->current_token.type == TOKEN_OR){
              // TOKEN_OR |
              Token val = parse_or(lexer);
              if(condition.type == TOKEN_STRING){
                  if(val.type == TOKEN_NUMBER){
                     char *strn = (char *)malloc(sizeof(char) * 2);
                     memset(strn, '\0', sizeof(char) * 2);
                     long a = 0;
                     if((long)val.number_value > 0){
                         if(condition.string_value[strlen(condition.string_value) + (int)val.number_value] != '\0'){
                            for(int x = (strlen(condition.string_value) + (int)val.number_value); ; x++){
                                char c0 = condition.string_value[x];
                                if(c0 != '\0'){
                                   char *tmp = (char *)malloc(sizeof(char) * strlen(strn) + 2);
                                   memset(tmp, '\0', sizeof(char) * strlen(strn) + 2);
                                   strcpy(tmp, strn);
                                   tmp[strlen(tmp)] = c0;
                                   free(strn);
                                   strn = tmp;
                                }else{
                                    break;
                                }
                            }
                            free(condition.string_value);
                            condition.string_value = strn;
                            condition.type = TOKEN_STRING;
                            lexer->char_status = false;
                            lexer->string_status = true;      
                         }else{
                            free(strn);
                         }
                     }else{
                        free(strn);
                     }
                  }else if(val.type == TOKEN_CHAR){
                     //"aa" | 'a'
                     char *strn = (char *)malloc(sizeof(char) * (strlen(condition.string_value) + 1));
                     memset(strn, '\0', sizeof(char) * (strlen(condition.string_value) + 1));
                     int u0 = 0;
                     for(int x = 0; x < strlen(condition.string_value); x++){
                         char c0 = condition.string_value[x];
                         if(c0 != val.char_value){
                            strn[u0] = c0;
                            u0++;
                         }
                     }

                     for(int y = 0; y < strlen(condition.string_value); y++){
                         char c1 = condition.string_value[y];
                         if(c1 == val.char_value){
                            strn[u0] = c1;
                            u0++;
                         }
                     }
                     free(condition.string_value);
                     condition.string_value = strn;
                     condition.type = TOKEN_STRING;
                     lexer->char_status = false;
                     lexer->string_status = true;
                  }else{
                     char *replace = (char *)malloc(sizeof(char) * strlen(val.string_value));
                     memset(replace, '\0', sizeof(char) * strlen(val.string_value));
                     char *of = (char *)malloc(sizeof(char) * strlen(val.string_value));
                     memset(of, '\0', sizeof(char) * strlen(val.string_value));
                     bool isdetect = false;
                     int u = 0, k = 0;
                     for(int x = 0; x < strlen(val.string_value); x++){
                         char c0 = val.string_value[x];
                         if(isdetect){
                            of[k] = c0;
                            k++;
                         }else{
                            if(c0 != ':'){
                               replace[u] = c0;
                               u++;
                            }else{
                               isdetect = true;
                            }
                         }
                     }
      
                     long words = 0;
                     for(int y = 0; y < strlen(condition.string_value); y++){
                         char c0 = condition.string_value[y];
                         if(c0 == replace[0]){
                            bool isn = true;
                            for(int l = 0, o = y; l < strlen(replace); l++, o++){
                                char p0 = replace[l];
                                char p1 = condition.string_value[o];
                                if(p0 != p1){
                                   isn = false;
                                   break;
                                }
                            }
      
                            if(isn){
                               y += (strlen(replace) - 1);
                               words++;
                            }
                         }
                     }
      
                     char *strn = (char *)malloc(sizeof(char) * (strlen(condition.string_value) + ((strlen(replace) > strlen(of) ? strlen(replace) * words : strlen(of) * words))));
                     memset(strn, '\0', sizeof(char) * (strlen(condition.string_value) + ((strlen(replace) > strlen(of) ? strlen(replace) * words : strlen(of) * words))));
                     long wp = 0;
                     for(int i = 0; i < strlen(condition.string_value); i++){
                         char j = condition.string_value[i];
                         if(j == replace[0]){
                            bool isn = true;
                            for(int a = 0, b = i; a < strlen(replace); a++, b++){
                                char p0 = replace[a];
                                char p1 = condition.string_value[b];
                                if(p0 != p1){
                                   isn = false;
                                   break;
                                }
                            }
      
                            if(isn){
                               for(int g = 0; g < strlen(of); g++){
                                   strn[wp] = of[g];
                                   wp++;  
                               }
                               i += (strlen(replace) - 1);
                            }else{
                              strn[wp] = j;
                              wp++;  
                            }
                         }else{
                            strn[wp] = j;
                            wp++;
                         }
                     }
      
                     free(condition.string_value);
                     free(val.string_value);
                     condition.string_value = NULL;
                     val.string_value = NULL;
                     condition.string_value = strn;
                     condition.type = TOKEN_STRING;
                     lexer->char_status = false;
                     lexer->string_status = true; 
                  }
              }else if(condition.type == TOKEN_CHAR){
                  if(val.type == TOKEN_NUMBER){
                     long long i = ((long)condition.char_value | (long)val.number_value);
                     condition.char_value = (char)i;
                     condition.char_value = true;
                     condition.string_value = false;
                     condition.type = TOKEN_CHAR;
                  }else if(val.type == TOKEN_CHAR){
                     long long i = ((long)condition.char_value | (long)val.char_value);
                     condition.char_value = (char)i;
                     condition.char_value = true;
                     condition.string_value = false;
                     condition.type = TOKEN_CHAR;
                  }else{
                      //'a' | "aaa"
                     char *strn = (char *)malloc(sizeof(char) * (strlen(val.string_value) + 1));
                     memset(strn, '\0', sizeof(char) * (strlen(val.string_value) + 1));
                     int u0 = 0;

                     for(int y = 0; y < strlen(val.string_value); y++){
                         char c1 = val.string_value[y];
                         if(c1 == condition.char_value){
                            strn[u0] = c1;
                            u0++;
                         }
                     }

                     for(int x = 0; x < strlen(val.string_value); x++){
                         char c0 = val.string_value[x];
                         if(c0 != condition.char_value){
                            strn[u0] = c0;
                            u0++;
                         }
                     }

                     free(val.string_value);
                     val.string_value = NULL;
                     condition.string_value = strn;
                     condition.type = TOKEN_STRING;
                     lexer->char_status = false;
                     lexer->string_status = true;
                  }
              }else{
                  if(val.type == TOKEN_NUMBER){
                     long long i = ((long)condition.number_value | (long)val.number_value);
                     condition.number_value = (double)i;
                     condition.char_value = false;
                     condition.string_value = false;
                     condition.type = TOKEN_NUMBER;
                  }else if(val.type == TOKEN_CHAR){
                     long long i = ((long)condition.number_value | (long)val.char_value);
                     condition.number_value = (double)i;
                     condition.char_value = false;
                     condition.string_value = false;
                     condition.type = TOKEN_NUMBER;
                  }else{
                     char *strn = (char *)malloc(sizeof(char) * 2);
                     memset(strn, '\0', sizeof(char) * 2);
                     long a = 0;
                     if((long)condition.number_value > 0){
                         if(val.string_value[strlen(val.string_value) + (int)condition.number_value] != '\0'){
                            for(int x = (strlen(val.string_value) + (int)condition.number_value); ; x++){
                                char c0 = val.string_value[x];
                                if(c0 != '\0'){
                                   char *tmp = (char *)malloc(sizeof(char) * strlen(strn) + 2);
                                   memset(tmp, '\0', sizeof(char) * strlen(strn) + 2);
                                   strcpy(tmp, strn);
                                   tmp[strlen(tmp)] = c0;
                                   free(strn);
                                   strn = tmp;
                                }else{
                                    break;
                                }
                            }
                            char *strn2 = (char *)malloc(sizeof(char) * (strlen(strn) + 1));
                            for(int a = (strlen(strn) - 1), b = 0; a >= 0; a--, b++){
                                strn2[b] = strn[a];
                            }
                            free(strn);
                            strn = strn2;
                            free(val.string_value);
                            val.string_value = NULL;
                            condition.string_value = strn;
                            condition.type = TOKEN_STRING;
                            lexer->char_status = false;
                            lexer->string_status = true;      
                         }else{
                            free(strn);
                         }
                     }else{
                        free(strn);
                     }
                  }
              }
          }
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
    Lexer lexer = { expression, 0, false, false, TOKEN_END};
    get_next_token(&lexer);
    Token tk = parse_expression(&lexer);
    if(lexer.string_status){
       tk.type = TOKEN_STRING;
    }else if(lexer.char_status){
       tk.type = TOKEN_CHAR;
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
