/*
 * Copyright 2020-2026 Mariusz Obajtek. All rights reserved.
 * License: https://github.com/Nykakin/chompjs/blob/master/LICENSE
 */

#ifndef CHOMPJS_PARSER_H
#define CHOMPJS_PARSER_H

#include <stddef.h>
#include <stdbool.h>

#include "buffer.h"

struct Parser;

/**
    States of internal state machine:
    * begin - start parsing
    * json - handle special characters: "[", "{", "}", "]", ",", ":"
    * value - handle a JSON value, such as strings and numbers
    * end - finish work
    * error - finish work, mark an error
*/
struct State* begin(struct Parser* parser);
struct State* json(struct Parser* parser);
struct State* value(struct Parser* parser);
struct State* end(struct Parser* parser);
struct State* error(struct Parser* parser);

/*
    Helper functions used in "value" state
    * handle_quoted - handles quoted strings
    * handle_numeric - handle numbers
    * handle_numeric_standard_base - handle numbers in standard base-10
    * handle_numeric_non_standard_base - handle numbers in non-standard bases (hex, oct)
    * handle_unrecognized - save all unrecognized data as a string
*/
struct State* handle_quoted(struct Parser* parser);
struct State* handle_numeric(struct Parser* parser);
struct State* handle_numeric_standard_base(struct Parser* parser);
struct State* handle_numeric_non_standard_base(struct Parser* parser, int base);
struct State* handle_unrecognized(struct Parser* parser);

/**
    State wrapper
*/
struct State {
    struct State* (*change)(struct Parser *);
};

/** Possible results of internal state machine state change state */
typedef enum {
    CAN_ADVANCE,
    FINISHED,
    ERROR,
} ParserStatus;

/** Main object, responsible for everything */
struct Parser {
    const char* input;
    size_t output_size;
    struct CharBuffer output;
    size_t input_position;
    ParserStatus parser_status;
    struct State* state;
    struct CharBuffer nesting_depth;
    size_t unrecognized_nesting_depth;
    bool is_key;
};

/** Switch state of internal state machine */
void advance(struct Parser* parser);

/** Get next char, ignore whitespaces */
char next_char(struct Parser* parser);

/** Get previously handled char */
char last_char(struct Parser* parser);

/** Send character to output buffer, advance input position */
void emit(char c, struct Parser* parser);

/** Send character to output buffer, keep old input position */
void emit_in_place(char c, struct Parser* parser);

/** Remove last character from output buffer */
void unemit(struct Parser* parser);

/** Send string to output buffer, advance input position */
void emit_string(const char *s, size_t size, struct Parser* parser);

/** Send string to output buffer, keep old input position */
void emit_string_in_place(const char *s, size_t size, struct Parser* parser);

/** Send number to output buffer, keep old input position */
void emit_number_in_place(long value, struct Parser* parser);

/** Handle comments in JSON body */
void handle_comments(struct Parser* parser);

/** Initialize main parser object */
void init_parser(struct Parser* parser, const char* string);

/** Reset main parser object output buffer */
void reset_parser_output(struct Parser* parser);

/** Release main parser object and its memory */
void release_parser(struct Parser* parser);

#endif
