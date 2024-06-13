/*
 * Copyright 2020-2024 Mariusz Obajtek. All rights reserved.
 * License: https://github.com/Nykakin/chompjs/blob/master/LICENSE
 */

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>

#define INITIAL_NESTING_DEPTH 20

struct State states[] = {
    {begin},
    {json},
    {value},
    {end},
    {error},
};

enum StateIndex {
    BEGIN_STATE, JSON_STATE, VALUE_STATE, END_STATE, ERROR_STATE
};

void advance(struct Lexer* lexer) {
    lexer->state = lexer->state->change(lexer);
}

char next_char(struct Lexer* lexer) {
    while(1) {
        if(isspace(lexer->input[lexer->input_position])) {
            lexer->input_position += 1;
            continue;
        }
        return lexer->input[lexer->input_position];
    }
    return '\0';
}

char last_char(struct Lexer* lexer) {
    return top(&lexer->output);
}

void emit(char c, struct Lexer* lexer) {
    push(&lexer->output, c);
    lexer->input_position += 1;   
}

void emit_in_place(char c, struct Lexer* lexer) {
    push(&lexer->output, c);
}

void unemit(struct Lexer* lexer) {
    pop(&lexer->output);
}

void emit_string(const char *s, size_t size, struct Lexer* lexer) {
    push_string(&lexer->output, s, size);
    lexer->input_position += size;   
}

void emit_string_in_place(const char *s, size_t size, struct Lexer* lexer) {
    push_string(&lexer->output, s, size);
}

void emit_number_in_place(long value, struct Lexer* lexer) {
    push_number(&lexer->output, value);
}

void init_lexer(struct Lexer* lexer, const char* string) {
    lexer->input = string;
    // allocate in advance more memory for output than for input because we might need
    // to add extra characters
    // for example `{a: undefined}` will be translated as `{"a": "undefined"}`
    lexer->output_size = 2 * strlen(string) + 1;
    init_char_buffer(&lexer->output, lexer->output_size);
    lexer->input_position = 0;
    init_char_buffer(&lexer->nesting_depth, INITIAL_NESTING_DEPTH);
    lexer->unrecognized_nesting_depth = 0;
    lexer->lexer_status = CAN_ADVANCE;
    lexer->state = &states[BEGIN_STATE];
    lexer->is_key = false;
}

void reset_lexer_output(struct Lexer* lexer) {
    clear(&lexer->output);
    lexer->lexer_status = CAN_ADVANCE;
    lexer->state = &states[BEGIN_STATE];
    lexer->is_key = false;
    lexer->input_position -= 1;
}

void release_lexer(struct Lexer* lexer) {
    release_char_buffer(&lexer->output);
}

struct State* begin(struct Lexer* lexer) {
    // Ignoring characters until either '{' or '[' appears
    for(;;) {
        switch(next_char(lexer)) {
        case '{':
            lexer->is_key = true;
        case '[':;
            return &states[JSON_STATE];
        break;
        case '\0':;
            return &states[END_STATE];
        default:
            lexer->input_position += 1;
        }
    }
    return &states[ERROR_STATE];
}

struct State* json(struct Lexer* lexer) {
    for(;;) {
        switch(next_char(lexer)) {
        case '{':
            push(&lexer->nesting_depth, '{');
            lexer->is_key = true;
            emit('{', lexer);
        break;
        case '[':
            push(&lexer->nesting_depth, '[');
            emit('[', lexer);
        break;
        case '}':
            if(last_char(lexer) == ',') {
                unemit(lexer);
            }
            pop(&lexer->nesting_depth);
            lexer->is_key = top(&lexer->nesting_depth) == '{';
            emit('}', lexer);
            if(size(&lexer->nesting_depth) <= 0) {
                return &states[END_STATE];
            }
        break;
        case ']':
            if(last_char(lexer) == ',') {
                unemit(lexer);
            }
            pop(&lexer->nesting_depth);
            lexer->is_key = top(&lexer->nesting_depth) == '{';
            emit(']', lexer);
            if(size(&lexer->nesting_depth) <= 0) {
                return &states[END_STATE];
            }
        break;
        case ':':
            lexer->is_key = false;
            emit(':', lexer);
        break;
        case ',':
            emit(',', lexer);
            lexer->is_key = top(&lexer->nesting_depth) == '{';
        break;

        case '/':;
            char next_c = lexer->input[lexer->input_position+1];
            if(next_c == '/' || next_c == '*') {
                handle_comments(lexer);
            } else {
                return &states[VALUE_STATE];
            }
        break;

        // This should never happen, but an malformed input can
        // cause an infinite loop without this check
        case '>':
        case ')':;
            return &states[ERROR_STATE];
        break;

        default:
            return &states[VALUE_STATE];
        }
    }

    return &states[ERROR_STATE];
}

struct State* _handle_string(struct Lexer* lexer, const char* string, size_t length) {
    char next_char = lexer->input[lexer->input_position+length+1];
    if(next_char == '_' || isalnum(next_char)) {
        return handle_unrecognized(lexer);
    }
    emit_string(string, length, lexer);
    return &states[JSON_STATE];
}

struct State* value(struct Lexer* lexer) {
    char c = next_char(lexer);
    const char* position = lexer->input + lexer->input_position;

    if(c == '"' || c == '\'' || c == '`') {
        return handle_quoted(lexer);
    } else if(isdigit(c) || c == '.' || c == '-') {
        if(lexer->is_key) {
            return handle_unrecognized(lexer);
        } else {
            return handle_numeric(lexer);
        }
    } else if(strncmp(position, "true", 4) == 0) {
        return _handle_string(lexer, "true", 4);
    } else if(strncmp(position, "false", 5) == 0) {
        return _handle_string(lexer, "false", 5);
    } else if(strncmp(position, "null", 4) == 0) {
        return _handle_string(lexer, "null", 4);
    } else if(c == ']' || c == '}' || c == '[' || c == '{') {
        return &states[JSON_STATE];
    } else if(strncmp(position, "NaN", 3) == 0) {
        return _handle_string(lexer, "NaN", 3);
    } else {
        return handle_unrecognized(lexer);
    }

    return &states[JSON_STATE];
}

struct State* end(struct Lexer* lexer) {
    emit('\0', lexer);
    lexer->lexer_status = FINISHED;
    return lexer->state;
}

struct State* error(struct Lexer* lexer) {
    emit('\0', lexer);
    lexer->lexer_status = ERROR;
    return lexer->state;
}

struct State* handle_quoted(struct Lexer* lexer) {
    char current_quotation = next_char(lexer);
    emit('"', lexer);

    for(;;) {
        char c = lexer->input[lexer->input_position];
        // handle escape sequences such as \\ and \'
        if(c == '\\') {
            char escaped = lexer->input[lexer->input_position+1];
            if(escaped == '\'') {
                emit('\'', lexer);
                lexer->input_position += 1;
            } else {
                emit('\\', lexer);
                emit(escaped, lexer);   
            }
            continue;
        }
        // in case of malformed quotation we can reach end of the input
        if(c == '\0') {
            return &states[ERROR_STATE];
        }
        // if we're closing the quotations, we're done with the string
        if(c == current_quotation) {
            emit('"', lexer);
            return &states[JSON_STATE];
        }
        // otherwise, emit character
        if(c == '"') {
            emit_string_in_place("\\\"", 2, lexer);
            lexer->input_position += 1;
        } else {
            emit(c, lexer);
        }
    }
            
    return &states[ERROR_STATE];
}

struct State* handle_numeric(struct Lexer* lexer) {
    char c = next_char(lexer);
    if(c >= 49 && c <= 57) { // 1-9 range
        return handle_numeric_standard_base(lexer);
    } else if(c == '.') {
        emit_in_place('0', lexer);
        emit('.', lexer);
        return handle_numeric_standard_base(lexer);
    } else if(c == '-') {
        emit('-', lexer);
        return handle_numeric(lexer);
    } else if(c == '0') {
        char nc = tolower(lexer->input[lexer->input_position+1]);
        if(nc == '.') {
            emit('0', lexer);
            emit('.', lexer);
            return handle_numeric_standard_base(lexer);
        } else if(nc == 'x' || nc == 'X') {
            return handle_numeric_non_standard_base(lexer, 16);
        } else if(nc == 'o' || nc == 'O') {
            lexer->input_position += 2;
            return handle_numeric_non_standard_base(lexer, 8);
        } else if(isdigit(nc)) {
            return handle_numeric_non_standard_base(lexer, 8);
        } else if(nc == 'b' || nc == 'B') {
            lexer->input_position += 2;
            return handle_numeric_non_standard_base(lexer, 2);
        } else {
            emit('0', lexer);
            return &states[JSON_STATE];
        }
    } else {
        return &states[ERROR_STATE];
    }
    return &states[JSON_STATE];
}

struct State* handle_numeric_standard_base(struct Lexer* lexer) {
    char c = next_char(lexer);
    do {
        if(c != '_') {
            emit(c, lexer);
        } else {
            lexer->input_position += 1;
        }
        c = tolower(lexer->input[lexer->input_position]);
    } while(isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '+' || c =='-' || c == '_');
    if(last_char(lexer) == '.') {
        emit_in_place('0', lexer);
    }
    return &states[JSON_STATE];
}

struct State* handle_numeric_non_standard_base(struct Lexer* lexer, int base) {
    char* end;
    long n = strtol(lexer->input + lexer->input_position, &end, base);
    emit_number_in_place(n, lexer);
    lexer->input_position = end - lexer->input;
    return &states[JSON_STATE];
}

struct State* handle_unrecognized(struct Lexer* lexer) {
    emit_in_place('"', lexer);
    char currently_quoted_with = '\0';

    lexer->unrecognized_nesting_depth = 0;
    do {
        char c = lexer->input[lexer->input_position];

        switch(c) {
            case '\\':
                emit_in_place('\\', lexer);
                emit('\\', lexer);
            break;

            case '\'':
            case '"':
            case '`':
                if(c == '"') {
                    emit_in_place('\\', lexer);
                    emit('"', lexer);
                } else {
                    emit(c, lexer);
                }

                if(!currently_quoted_with) {
                    currently_quoted_with = c;
                } else if (currently_quoted_with == c) {
                    currently_quoted_with = '\0';
                }
            break;

            case '{':
            case '[':
            case '<':
            case '(':
                emit(c, lexer);
                lexer->unrecognized_nesting_depth += 1;
            break;

            case '}':
            case ']':
            case '>':
            case ')':
                if(currently_quoted_with && lexer->unrecognized_nesting_depth > 0) {
                    emit(c, lexer);
                } else if(lexer->unrecognized_nesting_depth > 0) {
                    emit(c, lexer);
                    lexer->unrecognized_nesting_depth -= 1;
                } else {
                    // remove trailing whitespaces after value
                    while(isspace(last_char(lexer))) {
                        pop(&lexer->output);
                    }
                    emit_in_place('"', lexer);
                    return &states[JSON_STATE];
                }
            break;

            case ',':
            case ':':
                if(!currently_quoted_with && lexer->unrecognized_nesting_depth <= 0) {
                    // remove trailing whitespaces after key
                    while(isspace(last_char(lexer))) {
                        pop(&lexer->output);
                    }
                    emit_in_place('"', lexer);
                    return &states[JSON_STATE];
                } else {
                    emit(c, lexer);
                }
            break;

            default:
                emit(c, lexer);
        }
    } while (lexer->input[lexer->input_position] != '\0');

    return &states[ERROR_STATE];
}

void handle_comments(struct Lexer* lexer) {
    char c, next_c;

    lexer->input_position += 1;
    if(lexer->input[lexer->input_position] == '/' ) {
        for(;;) {
            lexer->input_position+=1;
            c = lexer->input[lexer->input_position];
            if((c == '\0') || (c == '\n')) {
                break;
            }
        }
    } else if(lexer->input[lexer->input_position] == '*') {
        for(;;) {
            lexer->input_position+=1;
            c = lexer->input[lexer->input_position];
            next_c = lexer->input[lexer->input_position+1];
            if((c == '\0') || (c == '*' && next_c == '/')) {
                break;
            }
        }
        lexer->input_position+=2;
    }
}
