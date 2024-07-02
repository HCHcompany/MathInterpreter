#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CODE_LENGTH 10000

typedef struct {
    char type[20];
    char code[MAX_CODE_LENGTH];
} CodeBlock;

void extractBlock(const char *start, const char *end, CodeBlock *block, const char *type) {
    strncpy(block->code, start, end - start + 1);
    block->code[end - start + 1] = '\0';
    strcpy(block->type, type);
}

const char* findMatchingBrace(const char *start) {
    const char *p = start;
    int openBraces = 0;
    while (*p) {
        if (*p == '{') openBraces++;
        if (*p == '}') openBraces--;
        if (openBraces == 0) return p;
        p++;
    }
    return NULL;
}

void findCodeBlocks(const char *code, CodeBlock blocks[], int *blockCount) {
    const char *keywords[] = {"if", "for", "while", "do", "switch", "foreach"};
    int keywordCount = sizeof(keywords) / sizeof(keywords[0]);
    const char *p = code;
    *blockCount = 0;

    while (*p) {
        for (int i = 0; i < keywordCount; i++) {
            const char *keyword = keywords[i];
            int keywordLen = strlen(keyword);
            if (strncmp(p, keyword, keywordLen) == 0 && (p[keywordLen] == '(' || p[keywordLen] == ' ')) {
                const char *blockStart = p;
                const char *blockEnd = p + keywordLen;

                if (strcmp(keyword, "do") == 0) {
                    blockEnd = strstr(p, "while");
                    if (blockEnd != NULL) {
                        blockEnd = findMatchingBrace(blockEnd);
                    }
                } else {
                    const char *braceStart = strchr(p, '{');
                    if (braceStart != NULL) {
                        blockEnd = findMatchingBrace(braceStart);
                    } else {
                        blockEnd = NULL;
                    }
                }

                if (blockEnd != NULL) {
                    if (strcmp(keyword, "if") == 0) {
                        const char *next = blockEnd + 1;
                        while (strncmp(next, "else", 4) == 0) {
                            const char *elseStart = next;
                            next += 4;
                            if (strncmp(next, " if", 3) == 0) {
                                next += 3;
                                const char *elseIfStart = strstr(next, "{");
                                const char *elseIfEnd = findMatchingBrace(elseIfStart);
                                blockEnd = elseIfEnd;
                            } else {
                                const char *elseStart = strchr(next, '{');
                                const char *elseEnd = findMatchingBrace(elseStart);
                                blockEnd = elseEnd;
                            }
                            next = blockEnd + 1;
                        }
                    }
                    extractBlock(blockStart, blockEnd, &blocks[*blockCount], keyword);
                    (*blockCount)++;
                    p = blockEnd;
                }
                break;
            }
        }
        p++;
    }
}

int main() {
    const char *code = "if(12 < 20){\n"
                       "   printf(\"40\");\n"
                       "}\n"
                       "\n"
                       "for(int x = 0; x < 100; x++){\n"
                       "    printf(\"Loop\");\n"
                       "}\n"
                       "\n"
                       "do{\n"
                       "    printf(\"do loop\");\n"
                       "}while(0 < 100);\n"
                       "\n"
                       "foreach(int i in array){\n"
                       "      printf(i);\n"
                       "}\n"
                       "\n"
                       "switch(0){\n"
                       "    case 0: {\n"
                       "       printf(\"0\");\n"
                       "       break;\n"
                       "    }\n"
                       "    default: {\n"
                       "       printf(\"def\");\n"
                       "       break;\n"
                       "    }\n"
                       "}\n"
                       "\n"
                       "if(29 > 12){\n"
                       "    printf(\"YES\");\n"
                       "}else{\n"
                       "    printf(\"NOT\");\n"
                       "}\n"
                       "\n"
                       "if(93 > 20){\n"
                       "    printf(\"YES\");\n"
                       "}else if(102 < 2939){\n"
                       "    printf(\"YES\");\n"
                       "}else{\n"
                       "    printf(\"NOT\");\n"
                       "}";

    CodeBlock blocks[100];
    int blockCount = 0;

    findCodeBlocks(code, blocks, &blockCount);

    for (int i = 0; i < blockCount; i++) {
        printf("Block Type: %s\n", blocks[i].type);
        printf("Code:\n%s\n\n", blocks[i].code);
    }

    return 0;
}
