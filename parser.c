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
    int index = lexer->input_position-1;
    while(index > 0) {
        if(isspace(lexer->input[index])) {
            index -= 1;
            continue;
        } else {
            return lexer->input[index];
        }
    }
    return '\0';
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
    // for output alloc twice the size of input because characters are added
    // when identifiers are quoted, e.g. from '{a:1}' to  '{"a":1}'
    // so output might be larger than input, especially for malicious input
    // such as '{a:1,b:1,c:1,d:1,e:1,f:1,g:1,h:1,i:1,j:1}' that is translated to
    // '{"a":1,"b":1,"c":1,"d":1,"e":1,"f":1,"g":1,"h":1,"i":1}'
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
                struct State end_state = {end};
                return end_state;
            }
        break;
        case ']':
            if(last_char(lexer) == ',') {
                unemit(lexer);
            }
            pop(&lexer->depth_stack);
            emit(']', lexer);
            if(empty(&lexer->depth_stack)) {
                struct State end_state = {end};
                return end_state;
            }
        break;
        case ':':
            emit(':', lexer);
        break;
        case ',':
            emit(',', lexer);
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
    char c = lexer->input[lexer->input_position];
    if(c == '-') {
        emit('-', lexer);
        c = next_char(lexer);
    }
    if(c == '.') {
        emit_in_place('0', lexer);
    }

    do {
        if(c != '_') {
            emit(c, lexer);
        } else {
            lexer->input_position += 1;
        }
        c = tolower(lexer->input[lexer->input_position]);
    } while(isdigit(c) || c == '.' || c == '_' || c == 'e');

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
