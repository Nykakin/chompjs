/*
 * Copyright 2020-2021 Mariusz Obajtek. All rights reserved.
 * License: https://github.com/Nykakin/chompjs/blob/master/LICENSE
 */

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>

const size_t INITIAL_DEPTH_BUFFER_SIZE = 10;
const size_t INITIAL_HELPER_BUFFER_SIZE = 10;

void advance(struct Lexer* lexer) {
    lexer->state = lexer->state.change(lexer);
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

void emit_string(char *s, size_t size, struct Lexer* lexer) {
    push_string(&lexer->output, s, size);
    lexer->input_position += size;   
}

void emit_string_in_place(char *s, size_t size, struct Lexer* lexer) {
    push_string(&lexer->output, s, size);
}

void init_lexer(struct Lexer* lexer, const char* string, bool is_jsonlines) {
    lexer->input = string;
    // allocate in advance more memory for output than for input because we might need
    // to add extra characters
    // for example `{a: undefined}` will be translated as `{"a": "undefined"}`
    lexer->output_size = 2 * strlen(string);
    init_char_buffer(&lexer->output, lexer->output_size);
    lexer->input_position = 0;
    struct State begin_state = {begin};
    lexer->lexer_status = CAN_ADVANCE;
    lexer->state = begin_state;
    init_char_buffer(&lexer->depth_stack, INITIAL_DEPTH_BUFFER_SIZE);
    init_char_buffer(&lexer->helper_buffer, INITIAL_HELPER_BUFFER_SIZE);
    lexer->is_jsonlines = is_jsonlines;
}

void release_lexer(struct Lexer* lexer) {
    release_char_buffer(&lexer->output);
    release_char_buffer(&lexer->depth_stack);
    release_char_buffer(&lexer->helper_buffer);
}

struct State begin(struct Lexer* lexer) {
    // Ignoring characters until either '{' or '[' appears
    for(;;) {
        switch(next_char(lexer)) {
        case '{':
        case '[':;
            struct State value_state = {json};
            return value_state; 
        break;
        case '\0':;
            struct State end_state = {end};
            return end_state; 
        default:
            lexer->input_position += 1;
        }
    }
    struct State error_state = {error};
    return error_state;
}

struct State json(struct Lexer* lexer) {
    for(;;) {
        switch(next_char(lexer)) {
        case '{':
            push(&lexer->depth_stack, OBJECT);
            emit('{', lexer);
        break;
        case '[':
            push(&lexer->depth_stack, ARRAY);
            emit('[', lexer);
        break;
        case '}':
            if(last_char(lexer) == ',') {
                unemit(lexer);
            }
            pop(&lexer->depth_stack);
            emit('}', lexer);
            if(empty(&lexer->depth_stack)) {
                if(lexer->is_jsonlines) {
                    emit_in_place('\0', lexer);
                    struct State begin_state = {begin};
                    return begin_state;
                } else {
                    struct State end_state = {end};
                    return end_state;
                }
            }
        break;
        case ']':
            if(last_char(lexer) == ',') {
                unemit(lexer);
            }
            pop(&lexer->depth_stack);
            emit(']', lexer);
            if(empty(&lexer->depth_stack)) {
                if(lexer->is_jsonlines) {
                    emit_in_place('\0', lexer);
                    struct State begin_state = {begin};
                    return begin_state;
                } else {
                    struct State end_state = {end};
                    return end_state;
                }
            }
        break;
        case ':':
            emit(':', lexer);
        break;
        case ',':
            emit(',', lexer);
        break;

        case '/':;
            char next_c = lexer->input[lexer->input_position+1];
            if(next_c == '/' || next_c == '*') {
                handle_comments(lexer);
            } else {
                struct State value_state = {value};
                return value_state;
            }
        break;

        default:;
            struct State value_state = {value};
            return value_state;
        }
    }

    struct State error_state = {error};
    return error_state; 
}

struct State value(struct Lexer* lexer) {
    char c = next_char(lexer);

    if(c == '"' || c == '\'' || c == '`') {
        return handle_quoted(lexer);
    } else if(isdigit(c) || c == '.' || c == '-') {
        return handle_numeric(lexer);
    } else if(strncmp(lexer->input + lexer->input_position, "true", 4) == 0) {
        emit_string("true", 4, lexer);
    } else if(strncmp(lexer->input + lexer->input_position, "false", 5) == 0) {
        emit_string("false", 5, lexer);
    } else if(strncmp(lexer->input + lexer->input_position, "null", 4) == 0) {
        emit_string("null", 4, lexer);
    } else if(c == ']' || c == '}' || c == '[' || c == '{') {
        struct State json_state = {json};
        return json_state; 
    } else {
        return handle_unrecognized(lexer);
    }

    struct State json_state = {json};
    return json_state; 
}

struct State end(struct Lexer* lexer) {
    if(!lexer->is_jsonlines) {
        emit('\0', lexer);
    }
    lexer->lexer_status = FINISHED;
    return lexer->state;
}

struct State error(struct Lexer* lexer) {
    emit('\0', lexer);
    lexer->lexer_status = ERROR;
    return lexer->state;
}

struct State handle_quoted(struct Lexer* lexer) {
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
        // if we're closing the quotations, we're done with the string
        if(c == current_quotation) {
            emit('"', lexer);
            struct State json_state = {json};
            return json_state;            
        }
        // otherwise, emit character
        if(c == '"') {
            emit_string_in_place("\\\"", 2, lexer);
            lexer->input_position += 1;
        } else {
            emit(c, lexer);
        }
    }
            
    struct State error_state = {error};
    return error_state; 
}

struct State handle_numeric(struct Lexer* lexer) {
    char c = next_char(lexer);
    if(c == '-') {
        emit('-', lexer);
        c = next_char(lexer);
    }
    if(c == '.') {
        emit_in_place('0', lexer);
    }

    bool to_be_quoted = false;
    c = next_char(lexer);
    if(c == '0') {
        char next_c = tolower(lexer->input[lexer->input_position+1]);
        if(next_c == 'x' || next_c == 'b' || next_c == 'o' || isdigit(next_c)) {
            to_be_quoted = true;
            emit_in_place('"', lexer);
            emit('0', lexer);
            emit(next_c, lexer);
            c = tolower(lexer->input[lexer->input_position]);
        }
    }

    do {
        if(c != '_') {
            emit(c, lexer);
        } else {
            lexer->input_position += 1;
        }
        c = tolower(lexer->input[lexer->input_position]);
    // [97, 102] is ASCII range for a-f, for hex digits
    } while(isdigit(c) || c == '.' || c == '_' || (c >= 97 && c <= 102));

    if(to_be_quoted) {
        emit_in_place('"', lexer);
    }

    struct State json_state = {json};
    return json_state;
}

struct State handle_unrecognized(struct Lexer* lexer) {
    emit_in_place('"', lexer);
    char currently_quoted_with = '\0';

    clear(&lexer->helper_buffer);
    do {
        char c = lexer->input[lexer->input_position];

        switch(c) {
            case '\'':
            case '"':
            case '`':
                emit(c, lexer);
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
                push(&lexer->helper_buffer, c);
            break;

            case '}':
            case ']':
            case '>':
            case ')':
                if(currently_quoted_with && !empty(&lexer->helper_buffer)) {
                    emit(c, lexer);
                } else if(!empty(&lexer->helper_buffer)) {
                    emit(c, lexer);
                    pop(&lexer->helper_buffer);
                } else {
                    emit_in_place('"', lexer);
                    struct State json_state = {json};
                    return json_state;
                }
            break;

            case ',':
            case ':':
                if(!currently_quoted_with && empty(&lexer->helper_buffer)) {
                    emit_in_place('"', lexer);
                    struct State json_state = {json};
                    return json_state;
                } else {
                    emit(c, lexer);
                }
            break;

            default:
                emit(c, lexer);
        }
    } while (lexer->input[lexer->input_position] != '\0');

    struct State error_state = {error};
    return error_state;   
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
