#ifndef CHOMPJS_PARSER_H
#define CHOMPJS_PARSER_H

#include <stddef.h>

#include "buffer.h"

struct Lexer;

struct State begin(struct Lexer* lexer);
struct State json(struct Lexer* lexer);
struct State value(struct Lexer* lexer);
struct State end(struct Lexer* lexer);
struct State error(struct Lexer* lexer);

struct State handle_quoted(struct Lexer* lexer);
struct State handle_numeric(struct Lexer* lexer);
struct State handle_unrecognized(struct Lexer* lexer);

typedef enum {
    OBJECT = 79, // ASCI 'O'
    ARRAY = 65 // ASCII 'A'
} Type;

struct State {
    struct State (*change)(struct Lexer *);
};

typedef enum {
    CAN_ADVANCE,
    FINISHED,
    ERROR,
} LexerStatus;

struct Lexer {
    const char* input;
    size_t output_size;
    struct CharBuffer output;
    size_t input_position;
    size_t output_position;
    LexerStatus lexer_status;
    struct State state;
    struct CharBuffer depth_stack;
    struct CharBuffer helper_buffer;
    bool is_jsonlines;
};


void advance(struct Lexer* lexer);

char next_char(struct Lexer* lexer);

char last_char(struct Lexer* lexer);

void emit(char c, struct Lexer* lexer);

void emit_in_place(char c, struct Lexer* lexer);

void unemit(struct Lexer* lexer);

void emit_string(char *s, size_t size, struct Lexer* lexer);

void emit_string_in_place(char *s, size_t size, struct Lexer* lexer);

void init_lexer(struct Lexer* lexer, const char* string, bool is_jsonlines);

void release_lexer(struct Lexer* lexer);

#endif
