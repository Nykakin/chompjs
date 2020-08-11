#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>

void init(struct Lexer* lexer, const char* string, size_t initial_stack_size, int is_jsonlines) {
    lexer->input = string,
    // for output alloc twice the size of input because characters are added
    // when identifiers are quoted, e.g. from '{a:1}' to  '{"a":1}'
    // so output might be larger than input, especially for malicious input
    // such as '{a:1,b:1,c:1,d:1,e:1,f:1,g:1,h:1,i:1,j:1}' that is translated to
    // '{"a":1,"b":1,"c":1,"d":1,"e":1,"f":1,"g":1,"h":1,"i":1}'
    lexer->output_size = 2 * strlen(string);
    lexer->output = malloc(lexer->output_size);
    lexer->input_position = 0;
    lexer->output_position = 0;
    struct State begin_state = {begin};
    lexer->state = begin_state;
    lexer->lexer_status = CAN_ADVANCE;
    lexer->stack_index = 0;
    lexer->stack_size = initial_stack_size;
    lexer->stack = malloc(initial_stack_size*sizeof(Type));
    lexer->current_quotation = '\0';
    lexer->is_jsonlines = is_jsonlines;
}

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

int safe_emit(char c, struct Lexer* lexer) {
    if(lexer->output_position > lexer->output_size) {
        return 0;
    } else {
        emit(c, lexer);
        return 1;
    }
}

void emit_without_advancing(char c, struct Lexer* lexer) {
    lexer->output[lexer->output_position] = c;
    lexer->output_position += 1;
}

void emit_string(char *s, int size, struct Lexer* lexer) {
    memcpy(lexer->output+lexer->output_position, s, size);
    lexer->output_position += size;
    lexer->input_position += size;    
}

void unemit(struct Lexer* lexer) {
    lexer->output_position -= 1;
}

void push(Type t, struct Lexer* lexer) {
    lexer->stack[lexer->stack_index] = t;
    lexer->stack_index += 1;
    if(lexer->stack_index >= lexer->stack_size) {
        Type* new_stack = malloc(2*lexer->stack_size*sizeof(Type));
        memmove(new_stack, lexer->stack, lexer->stack_size*sizeof(Type));
        free((Type*)lexer->stack);
        lexer->stack = new_stack;
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
    // Ignoring characters until either '{' or '[' appears
    for(;;) {
        switch(next_char(lexer)) {
        case '{':
            emit('{', lexer);
            push(DICT, lexer);
            struct State dictionary_state = {dictionary};
            return dictionary_state;
        break;
        case '[':
            push(ARRAY, lexer);
            emit('[', lexer);
            struct State array_state = {array};
            return array_state;
        break;
        case '\0':;
            struct State end_state = {end};
            return end_state;     
        default:;
            lexer->input_position += 1;
        }
    }
    struct State error_state = {error};
    return error_state;
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
                emit('\\', lexer);
                char escaped = lexer->input[lexer->input_position];
                if(escaped == lexer->current_quotation) {
                    lexer->input_position += 1;
                    emit('"', lexer);
                    lexer->input_position -= 1;
                } else if(escaped=='u' || escaped=='U') {
                    emit(escaped, lexer);
                    emit_string((char*)lexer->input+lexer->input_position, 4, lexer);
                } else {
                    lexer->input_position += 1;
                    emit(escaped, lexer);
                    lexer->input_position -= 1;
                }
                continue;
            }
            // if we're closing the quotations, we're done with the string
            if(c == lexer->current_quotation) {
                emit('"', lexer);
                struct State new_state = {colon};
                return new_state;
            }
            // otherwise, emit character
            if(!safe_emit(c, lexer)) {
                struct State error_state = {error};
                return error_state;
            }
        }
    case '}':
        if(last_char(lexer) == ',') {
            unemit(lexer);
        }
        emit('}', lexer);
        pop(lexer);
        struct State new_state = {comma_or_close};
        return new_state;
    }
    if(isalnum(c) || c == '$' || c == '_') {
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
                if(lexer->input[lexer->input_position+1] != '\'') {
                    emit('\\', lexer);
                } else {
                    emit('\'', lexer);
                    lexer->input_position += 1;
                    continue;
                }
                char escaped = lexer->input[lexer->input_position];
                if(escaped == lexer->current_quotation) {
                    lexer->input_position += 1;
                    emit('"', lexer);
                    lexer->input_position -= 1;
                } else if(escaped=='u' || escaped=='U') {
                    emit(escaped, lexer);
                    emit_string((char*)lexer->input+lexer->input_position, 4, lexer);
                } else {
                    lexer->input_position += 1;
                    emit(escaped, lexer);
                    lexer->input_position -= 1;
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
            if(!safe_emit(c, lexer)) {
                struct State error_state = {error};
                return error_state;
            }
        }
    case ']':
        if(last_char(lexer) == ',') {
            unemit(lexer);
        }
        emit(']', lexer);
        pop(lexer);
        struct State new_array_close_state = {comma_or_close};
        return new_array_close_state;
    case '}':
        if(last_char(lexer) == ',') {
            unemit(lexer);
        }
        emit('}', lexer);
        pop(lexer);
        struct State new_dictionary_close_state = {comma_or_close};
        return new_dictionary_close_state;
    }
    if(isdigit(c) || c == '.' || c == '-') {
        if(c == '-') {
            emit('-', lexer);
            c = tolower(lexer->input[lexer->input_position]);
        }
        if(c == '.') {
            emit_without_advancing('0', lexer);
        }

        do {
            if(c != '_' && c != ' ') {
                emit(c, lexer);
            } else {
                lexer->input_position += 1;
            }
            c = tolower(lexer->input[lexer->input_position]);
        } while(isdigit(c) || c == '.' || c == '_' || c == ' ' || c == 'e');

        struct State new_state = {comma_or_close};
        return new_state;      
    }
    if(strncmp(lexer->input + lexer->input_position, "true", 4) == 0) {
        emit_string("true", 4, lexer);
        struct State new_dictionary_close_state = {comma_or_close};
        return new_dictionary_close_state;
    } else if(strncmp(lexer->input + lexer->input_position, "false", 5) == 0) {
        emit_string("false", 5, lexer);
        struct State new_dictionary_close_state = {comma_or_close};
        return new_dictionary_close_state;
    } else if(strncmp(lexer->input + lexer->input_position, "null", 4) == 0) {
        emit_string("null", 4, lexer);
        struct State new_dictionary_close_state = {comma_or_close};
        return new_dictionary_close_state;
    }
    // Handle unusual values such as /d+/ or undefined
    emit_without_advancing('"', lexer);
    while(1) {
        c = lexer->input[lexer->input_position];
        switch(c) {
        case(','):
            emit_without_advancing('"', lexer);
            struct State comma_or_close_state = {comma_or_close};
            return comma_or_close_state;
        case ']':
            emit_without_advancing('"', lexer);
            emit(']', lexer);
            pop(lexer);
            struct State new_array_close_state = {comma_or_close};
            return new_array_close_state;
        case '}':
            emit_without_advancing('"', lexer);
            emit('}', lexer);
            pop(lexer);
            struct State new_dictionary_close_state = {comma_or_close};
            return new_dictionary_close_state;
        case '\0':;
            struct State error_state = {error};
            return error_state;
        default:
            emit(c, lexer);
        }
    }

    struct State error_state = {error};
    return error_state;
}

struct State comma_or_close(struct Lexer* lexer) {
    if(empty(lexer)) {
        if(!lexer->is_jsonlines) {
            struct State new_state = {end};
            return new_state;
        } else {
            emit('\0', lexer);
            struct State new_state = {begin};
            return new_state;                
        }
    } 

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
        if(last_char(lexer) == ',') {
            unemit(lexer);
        }
        emit(c, lexer);
        pop(lexer);
        struct State new_state = {comma_or_close};
        return new_state;
    default:;
        struct State error_state = {error};
        return error_state;
    }
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
