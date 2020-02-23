#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>

void advance(struct Lexer* lexer) {
    lexer->state = lexer->state.change(lexer);
}

char next_char(struct Lexer* lexer) {
    while(1) {
        switch(lexer->input[lexer->input_position]) {
        case ' ':
        case '\n':
        case '\t':
            lexer->input_position += 1;
            continue;
        break;
        default:
            return lexer->input[lexer->input_position];
        }
    }
    return '\0';
}

char prev_char(struct Lexer* lexer) {
    int index = lexer->input_position-1;
    while(index > 0) {
        switch(lexer->input[index]) {
        case ' ':
        case '\n':
        case '\t':
            index -= 1;
            continue;
        break;
        default:
            return lexer->input[index];
        }
    }
    return '\0';
}

void emit(char c, struct Lexer* lexer) {
    lexer->output[lexer->output_position] = c;
    lexer->output_position += 1;
    lexer->input_position += 1;
}

void unemit(struct Lexer* lexer) {
    lexer->output_position -= 1;
}

void push(Type t, struct Lexer* lexer) {
    lexer->stack[lexer->stack_index] = t;
    lexer->stack_index += 1;
    if(lexer->stack_index >= lexer->stack_size) {
        Type* old_stack = lexer->stack;
        lexer->stack = malloc(2*lexer->stack_size*sizeof(Type));
        memmove(lexer->stack, old_stack, lexer->stack_size);
        free((Type*)old_stack);
        lexer->stack_size *= 2;
    }
}

void pop(struct Lexer* lexer) {
    lexer->stack_index -= 1;
}

Type top(struct Lexer* lexer) {
    return lexer->stack[lexer->stack_index-1];
}

int empty(struct Lexer* lexer) {
    return lexer->stack_index == 0;
}

struct State begin(struct Lexer* lexer) {
    // Assume JSON starts from '{' or '[', otherwise it's an error
    switch(next_char(lexer)) {
    case '{':
        emit('{', lexer);
        push(DICT, lexer);
        struct State dictionary_state = {dictionary};
        return dictionary_state;
    case '[':
        push(ARRAY, lexer);
        emit('[', lexer);
        struct State array_state = {array};
        return array_state;
    default:;
        struct State error_state = {error};
        return error_state;
    }
}

struct State dictionary(struct Lexer* lexer) {
    struct State new_state = {key};
    return new_state;
}

struct State key(struct Lexer* lexer) {
    char c = next_char(lexer);
    switch(c) {
    case '\'':
    case '"':
    case '`':
        lexer->current_quotation = c;
        emit('"', lexer);

        while(1) {
            c = lexer->input[lexer->input_position];
            // handle escape sequences such as \\ and \'
            if(c == '\\'){
                char escaped = lexer->input[lexer->input_position+1];
                if(escaped== '`' || escaped == '\'') {
                    lexer->input_position += 1;
                    emit(escaped, lexer);
                } else {
                    emit('\\', lexer);
                    emit(escaped, lexer);
                }
            }
            // if we're closing the quotations, we're done with the string
            if(c == lexer->current_quotation) {
                emit('"', lexer);
                struct State new_state = {colon};
                return new_state;
            }
            // otherwise, emit character
            emit(c, lexer);
        }
    case '}':
        if(prev_char(lexer) == ',') {
            unemit(lexer);
        }
        emit('}', lexer);
        pop(lexer);
        if(empty(lexer)) {
            struct State new_state = {end};
            return new_state;
        } else {
            struct State new_state = {comma_or_close};
            return new_state;
        }
    }
    if(isalnum(c)) {
        emit('"', lexer);
        lexer->input_position -= 1;
        while(isalnum(c) || c == '$' || c == '_') {
            emit(c, lexer);
            c = lexer->input[lexer->input_position];
        }
        emit('"', lexer);
        lexer->input_position-=1;
        struct State new_state = {colon};
        return new_state;
    }

    struct State error_state = {error};
    return error_state;
}

struct State colon(struct Lexer* lexer) {
    switch(next_char(lexer)) {
    case ':':
        emit(':', lexer);
        struct State new_state = {value};
        return new_state;
    default:;
        struct State error_state = {error};
        return error_state;
    }
}

struct State array(struct Lexer* lexer) {
    struct State new_state = {value};
    return new_state;
}

struct State value(struct Lexer* lexer) {
    char c = next_char(lexer);
    switch(c) {
    case '{':
        emit('{', lexer);
        push(DICT, lexer);
        struct State new_dictionary_state = {dictionary};
        return new_dictionary_state;
    case '[':
        emit('[', lexer);
        push(ARRAY, lexer);
        struct State new_array_state = {array};
        return new_array_state;
    case '\'':
    case '"':
    case '`':
        lexer->current_quotation = c;
        emit('"', lexer);
        
        while(1) {
            c = lexer->input[lexer->input_position];
            // handle escape sequences such as \\ and \'
            if(c == '\\'){
                char escaped = lexer->input[lexer->input_position+1];
                if(escaped== '`' || escaped == '\'') {
                    lexer->input_position += 1;
                    emit(escaped, lexer);
                } else {
                    emit('\\', lexer);
                    emit(escaped, lexer);
                }
                continue;
            }
            // if we're closing the quotations, we're done with the string
            if(c == lexer->current_quotation) {
                emit('"', lexer);
                struct State new_state = {comma_or_close};
                return new_state;            
            }
            // otherwise, emit character
            emit(c, lexer);
        }
    case ']':
        if(prev_char(lexer) == ',') {
            unemit(lexer);
        }
        emit(']', lexer);
        pop(lexer);
        if(empty(lexer)) {
            struct State new_state = {end};
            return new_state;
        } else {
            struct State new_state = {comma_or_close};
            return new_state;
        }
    case '}':
        if(prev_char(lexer) == ',') {
            unemit(lexer);
        }
        emit('}', lexer);
        pop(lexer);
        struct State new_dictionary_close_state = {comma_or_close};
        return new_dictionary_close_state;   
    }
    if(isdigit(c) || c == '.') {
        do {
            emit(c, lexer);
            c = lexer->input[lexer->input_position];
        } while(isdigit(c) || c == '.');
        struct State new_state = {comma_or_close};
        return new_state;           
    }
    if(strncmp(lexer->input + lexer->input_position, "true", 4) == 0) {
        emit('t', lexer);
        emit('r', lexer);
        emit('u', lexer);
        emit('e', lexer);
        struct State new_dictionary_close_state = {comma_or_close};
        return new_dictionary_close_state;
    } else if(strncmp(lexer->input + lexer->input_position, "false", 5) == 0) {
        emit('f', lexer);
        emit('a', lexer);
        emit('l', lexer);
        emit('s', lexer);
        emit('e', lexer);
        struct State new_dictionary_close_state = {comma_or_close};
        return new_dictionary_close_state;
    } else if(strncmp(lexer->input + lexer->input_position, "null", 4) == 0) {
        emit('n', lexer);
        emit('u', lexer);
        emit('l', lexer);
        emit('l', lexer);
        struct State new_dictionary_close_state = {comma_or_close};
        return new_dictionary_close_state;
    }

    struct State error_state = {error};
    return error_state;
}

struct State comma_or_close(struct Lexer* lexer) {
    char c = next_char(lexer);
    switch(c) {
    case ',':
        emit(',', lexer);
        switch(top(lexer)) {
            case DICT:;
                struct State new_dict_state = {dictionary};
                return new_dict_state;
            break;
            case ARRAY:;
                struct State new_aray_state = {array};
                return new_aray_state;
            break;
        }
    case ']':
    case '}':
        if(prev_char(lexer) == ',') {
            unemit(lexer);
        }
        emit(c, lexer);
        pop(lexer);
        if(empty(lexer)) {
            struct State new_state = {end};
            return new_state;
        } else {
            struct State new_state = {comma_or_close};
            return new_state;
        }
    default:;
        struct State error_state = {error};
        return error_state;
    }
}

struct State end(struct Lexer* lexer) {
    lexer->output[lexer->output_position] = '\0';
    lexer->lexer_status = FINISHED;
    return lexer->state;
}

struct State error(struct Lexer* lexer) {
    lexer->output[lexer->output_position] = '\0';
    lexer->lexer_status = ERROR;
    return lexer->state;
}
