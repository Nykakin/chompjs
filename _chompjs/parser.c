/*
 * Copyright 2020-2026 Mariusz Obajtek. All rights reserved.
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

void advance(struct Parser* parser) {
    parser->state = parser->state->change(parser);
}

char next_char(struct Parser* parser) {
    while(1) {
        if(isspace(parser->input[parser->input_position])) {
            parser->input_position += 1;
            continue;
        }
        return parser->input[parser->input_position];
    }
    return '\0';
}

char last_char(struct Parser* parser) {
    return top(&parser->output);
}

void emit(char c, struct Parser* parser) {
    push(&parser->output, c);
    parser->input_position += 1;   
}

void emit_in_place(char c, struct Parser* parser) {
    push(&parser->output, c);
}

void unemit(struct Parser* parser) {
    pop(&parser->output);
}

void emit_string(const char *s, size_t size, struct Parser* parser) {
    push_string(&parser->output, s, size);
    parser->input_position += size;   
}

void emit_string_in_place(const char *s, size_t size, struct Parser* parser) {
    push_string(&parser->output, s, size);
}

void emit_number_in_place(long value, struct Parser* parser) {
    push_number(&parser->output, value);
}

void init_parser(struct Parser* parser, const char* string) {
    parser->input = string;
    // allocate in advance more memory for output than for input because we might need
    // to add extra characters
    // for example `{a: undefined}` will be translated as `{"a": "undefined"}`
    parser->output_size = 2 * strlen(string) + 1;
    init_char_buffer(&parser->output, parser->output_size);
    parser->input_position = 0;
    init_char_buffer(&parser->nesting_depth, INITIAL_NESTING_DEPTH);
    parser->unrecognized_nesting_depth = 0;
    parser->parser_status = CAN_ADVANCE;
    parser->state = &states[BEGIN_STATE];
    parser->is_key = false;
}

void reset_parser_output(struct Parser* parser) {
    clear(&parser->output);
    parser->parser_status = CAN_ADVANCE;
    parser->state = &states[BEGIN_STATE];
    parser->is_key = false;
    parser->input_position -= 1;
}

void release_parser(struct Parser* parser) {
    release_char_buffer(&parser->output);
}

struct State* begin(struct Parser* parser) {
    // Ignoring characters until either '{' or '[' appears
    for(;;) {
        switch(next_char(parser)) {
        case '{':
            parser->is_key = true;
        case '[':;
            return &states[JSON_STATE];
        break;
        case '\0':;
            return &states[END_STATE];
        case '/':
            {
                char next_c = parser->input[parser->input_position+1];
                if(next_c == '/' || next_c == '*') {
                    handle_comments(parser);
                }
            }                
        default:
            parser->input_position += 1;
        }
    }
    return &states[ERROR_STATE];
}

struct State* json(struct Parser* parser) {
    for(;;) {
        switch(next_char(parser)) {
        case '{':
            push(&parser->nesting_depth, '{');
            parser->is_key = true;
            emit('{', parser);
        break;
        case '[':
            push(&parser->nesting_depth, '[');
            emit('[', parser);
        break;
        case '}':
            if(last_char(parser) == ',') {
                unemit(parser);
            }
            pop(&parser->nesting_depth);
            parser->is_key = top(&parser->nesting_depth) == '{';
            emit('}', parser);
            if(size(&parser->nesting_depth) <= 0) {
                return &states[END_STATE];
            }
        break;
        case ']':
            if(last_char(parser) == ',') {
                unemit(parser);
            }
            pop(&parser->nesting_depth);
            parser->is_key = top(&parser->nesting_depth) == '{';
            emit(']', parser);
            if(size(&parser->nesting_depth) <= 0) {
                return &states[END_STATE];
            }
        break;
        case ':':
            parser->is_key = false;
            emit(':', parser);
        break;
        case ',':
            emit(',', parser);
            parser->is_key = top(&parser->nesting_depth) == '{';
        break;

        case '/':;
            char next_c = parser->input[parser->input_position+1];
            if(next_c == '/' || next_c == '*') {
                handle_comments(parser);
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

struct State* _handle_string(struct Parser* parser, const char* string, size_t length) {
    char next_char = parser->input[parser->input_position+length+1];
    if(next_char == '_' || isalnum(next_char)) {
        return handle_unrecognized(parser);
    }
    emit_string(string, length, parser);
    return &states[JSON_STATE];
}

struct State* value(struct Parser* parser) {
    char c = next_char(parser);
    const char* position = parser->input + parser->input_position;

    if(c == '"' || c == '\'' || c == '`') {
        return handle_quoted(parser);
    } else if(isdigit(c) || c == '.' || c == '-') {
        if(parser->is_key) {
            return handle_unrecognized(parser);
        } else {
            return handle_numeric(parser);
        }
    } else if(strncmp(position, "true", 4) == 0) {
        return _handle_string(parser, "true", 4);
    } else if(strncmp(position, "false", 5) == 0) {
        return _handle_string(parser, "false", 5);
    } else if(strncmp(position, "null", 4) == 0) {
        return _handle_string(parser, "null", 4);
    } else if(c == ']' || c == '}' || c == '[' || c == '{') {
        return &states[JSON_STATE];
    } else if(strncmp(position, "NaN", 3) == 0) {
        return _handle_string(parser, "NaN", 3);
    } else {
        return handle_unrecognized(parser);
    }

    return &states[JSON_STATE];
}

struct State* end(struct Parser* parser) {
    emit('\0', parser);
    parser->parser_status = FINISHED;
    return parser->state;
}

struct State* error(struct Parser* parser) {
    emit('\0', parser);
    parser->parser_status = ERROR;
    return parser->state;
}

struct State* handle_quoted(struct Parser* parser) {
    char current_quotation = next_char(parser);
    emit('"', parser);

    for(;;) {
        char c = parser->input[parser->input_position];
        // handle escape sequences such as \\ and \'
        if(c == '\\') {
            char escaped = parser->input[parser->input_position+1];
            if(escaped == '\'') {
                emit('\'', parser);
                parser->input_position += 1;
            } else {
                emit('\\', parser);
                emit(escaped, parser);   
            }
            continue;
        }
        // in case of malformed quotation we can reach end of the input
        if(c == '\0') {
            return &states[ERROR_STATE];
        }
        // if we're closing the quotations, we're done with the string
        if(c == current_quotation) {
            emit('"', parser);
            return &states[JSON_STATE];
        }
        // otherwise, emit character
        if(c == '"') {
            emit_string_in_place("\\\"", 2, parser);
            parser->input_position += 1;
        } else {
            emit(c, parser);
        }
    }
            
    return &states[ERROR_STATE];
}

struct State* handle_numeric(struct Parser* parser) {
    char c = next_char(parser);
    if(c >= 49 && c <= 57) { // 1-9 range
        return handle_numeric_standard_base(parser);
    } else if(c == '.') {
        emit_in_place('0', parser);
        emit('.', parser);
        return handle_numeric_standard_base(parser);
    } else if(c == '-') {
        emit('-', parser);
        return handle_numeric(parser);
    } else if(c == '0') {
        char nc = tolower(parser->input[parser->input_position+1]);
        if(nc == '.') {
            emit('0', parser);
            emit('.', parser);
            return handle_numeric_standard_base(parser);
        } else if(nc == 'x' || nc == 'X') {
            return handle_numeric_non_standard_base(parser, 16);
        } else if(nc == 'o' || nc == 'O') {
            parser->input_position += 2;
            return handle_numeric_non_standard_base(parser, 8);
        } else if(isdigit(nc)) {
            return handle_numeric_non_standard_base(parser, 8);
        } else if(nc == 'b' || nc == 'B') {
            parser->input_position += 2;
            return handle_numeric_non_standard_base(parser, 2);
        } else {
            emit('0', parser);
            return &states[JSON_STATE];
        }
    } else {
        return &states[ERROR_STATE];
    }
    return &states[JSON_STATE];
}

struct State* handle_numeric_standard_base(struct Parser* parser) {
    char c = next_char(parser);
    do {
        if(c != '_') {
            emit(c, parser);
        } else {
            parser->input_position += 1;
        }
        c = tolower(parser->input[parser->input_position]);
    } while(isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '+' || c =='-' || c == '_');
    if(last_char(parser) == '.') {
        emit_in_place('0', parser);
    }
    return &states[JSON_STATE];
}

struct State* handle_numeric_non_standard_base(struct Parser* parser, int base) {
    char* end;
    long n = strtol(parser->input + parser->input_position, &end, base);
    emit_number_in_place(n, parser);
    parser->input_position = end - parser->input;
    return &states[JSON_STATE];
}

struct State* handle_unrecognized(struct Parser* parser) {
    emit_in_place('"', parser);
    char currently_quoted_with = '\0';

    parser->unrecognized_nesting_depth = 0;
    do {
        char c = parser->input[parser->input_position];

        switch(c) {
            case '\\':
                emit_in_place('\\', parser);
                emit('\\', parser);
            break;

            case '\'':
            case '"':
            case '`':
                if(c == '"') {
                    emit_in_place('\\', parser);
                    emit('"', parser);
                } else {
                    emit(c, parser);
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
                emit(c, parser);
                parser->unrecognized_nesting_depth += 1;
            break;

            case '}':
            case ']':
            case '>':
                if(parser->input[parser->input_position-1] == '=') {
                    emit(c, parser);
                    continue;
                }
            case ')':
                if(currently_quoted_with && parser->unrecognized_nesting_depth > 0) {
                    emit(c, parser);
                } else if(parser->unrecognized_nesting_depth > 0) {
                    emit(c, parser);
                    parser->unrecognized_nesting_depth -= 1;
                } else {
                    // remove trailing whitespaces after value
                    while(isspace(last_char(parser))) {
                        pop(&parser->output);
                    }
                    emit_in_place('"', parser);
                    return &states[JSON_STATE];
                }
            break;

            case ',':
            case ':':
                if(!currently_quoted_with && parser->unrecognized_nesting_depth <= 0) {
                    // remove trailing whitespaces after key
                    while(isspace(last_char(parser))) {
                        pop(&parser->output);
                    }
                    emit_in_place('"', parser);
                    return &states[JSON_STATE];
                } else {
                    emit(c, parser);
                }
            break;

            default:
                emit(c, parser);
        }
    } while (parser->input[parser->input_position] != '\0');

    return &states[ERROR_STATE];
}

void handle_comments(struct Parser* parser) {
    char c, next_c;

    parser->input_position += 1;
    if(parser->input[parser->input_position] == '/' ) {
        for(;;) {
            parser->input_position+=1;
            c = parser->input[parser->input_position];
            if((c == '\0') || (c == '\n')) {
                break;
            }
        }
    } else if(parser->input[parser->input_position] == '*') {
        for(;;) {
            parser->input_position+=1;
            c = parser->input[parser->input_position];
            next_c = parser->input[parser->input_position+1];
            if((c == '\0') || (c == '*' && next_c == '/')) {
                break;
            }
        }
        parser->input_position+=2;
    }
}
