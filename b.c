#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_entire_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");

    if (file == NULL) {
        fprintf(stderr, "error: could not open file '%s': %s\n", file_path,
                strerror(errno));

        exit(1);
    }

    char c;

    fread(&c, 1, 1, file);

    if (ferror(file)) {
        fprintf(stderr, "error: could not read file '%s': %s\n", file_path,
                strerror(errno));

        fclose(file);

        exit(1);
    }

    if (fseek(file, 0, SEEK_END) == -1) {
        fprintf(stderr, "error: could not get size of file '%s': %s\n",
                file_path, strerror(errno));

        fclose(file);

        exit(1);
    }

    long file_size = ftell(file);

    if (file_size == -1) {
        fprintf(stderr, "error: could not get size of file '%s': %s\n",
                file_path, strerror(errno));

        fclose(file);

        exit(1);
    }

    rewind(file);

    char *buffer = malloc(sizeof(char) * (file_size + 1));

    if (buffer == NULL) {
        fprintf(stderr, "error: out of memory\n");

        fclose(file);

        exit(1);
    }

    fread(buffer, 1, file_size, file);

    if (ferror(file)) {
        fprintf(stderr, "error: could not read file '%s': %s\n", file_path,
                strerror(errno));

        fclose(file);

        exit(1);
    }

    buffer[file_size] = 0;

    return buffer;
}

typedef struct {
    const char *buffer;
    size_t index;
} Lexer;

typedef enum {
    TOK_INVALID,
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_PLUS,
    TOK_MINUS,
} TokenTag;

typedef struct {
    uint32_t start;
    uint32_t end;
} Range;

typedef struct {
    TokenTag tag;
    Range range;
} Token;

Token lexer_next(Lexer *l) {
    while (isspace(l->buffer[l->index]))
        l->index++;

    Token token = {.range = {.start = l->index, .end = l->index}};

    char character = l->buffer[l->index];

    switch (character) {
    case 0:
        token.tag = TOK_EOF;
        break;

    case '+':
        token.tag = TOK_PLUS;
        token.range.end = ++l->index;
        break;

    case '-':
        token.tag = TOK_MINUS;
        token.range.end = ++l->index;
        break;

    default:
        if (isalpha(character) || character == '_') {
            token.tag = TOK_IDENTIFIER;

            while (isalnum(l->buffer[l->index]) || l->buffer[l->index] == '_')
                l->index++;

            token.range.end = l->index;
        } else {
            token.tag = TOK_INVALID;
            token.range.end = ++l->index;
        }

        break;
    }

    return token;
}

int main(int argc, const char **argv) {
#define shift_args() (argc--, *argv++)

    const char *program = shift_args();

    if (argc == 0) {
        fprintf(stderr, "error: input file was not provided\n");

        return 1;
    }

    const char *input_file_path = shift_args();

    char *input_file_content = read_entire_file(input_file_path);

    Lexer lexer = {
        .buffer = input_file_content,
        .index = 0,
    };

    for (Token token = lexer_next(&lexer); token.tag != TOK_EOF;
         token = lexer_next(&lexer)) {
        switch (token.tag) {
        case TOK_EOF:
            printf("EOF\n");
            break;

        case TOK_INVALID:
            printf("INVALID\n");
            break;

        case TOK_PLUS:
            printf("+\n");
            break;

        case TOK_MINUS:
            printf("-\n");
            break;

        case TOK_IDENTIFIER:
            printf("IDENTIFIER(\"%.*s\")\n",
                   token.range.end - token.range.start,
                   lexer.buffer + token.range.start);
            break;
        }
    }
}
