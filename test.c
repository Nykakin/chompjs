#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

struct Token;
struct Lexer;

struct State begin(struct Lexer* lexer);
struct State dictionary(struct Lexer* lexer);
struct State key(struct Lexer* lexer);
struct State colon(struct Lexer* lexer);
struct State value(struct Lexer* lexer);
struct State array(struct Lexer* lexer);
struct State element(struct Lexer* lexer);
struct State comma_or_close(struct Lexer* lexer);
struct State end(struct Lexer* lexer);
struct State error(struct Lexer* lexer);

struct State {
    struct State (*change)(struct Lexer *);
};

typedef enum {
    DICT,
    ARRAY
} type;

struct Lexer {
    const char* input;
    long position;
    struct State state;
    int can_advance;

    short stack_index;
    type stack[10];
    char current_quotation;    
};

void advance(struct Lexer* lexer) {
    lexer->state = lexer->state.change(lexer);
}

char next_char(struct Lexer* lexer) {
    while(1) {
        switch(lexer->input[lexer->position]) {
        case ' ':
        case '\n':
        case '\t':
            lexer->position += 1;
            continue;
        break;
        default:
            return lexer->input[lexer->position];
        }
    }
    return '\0';
}

void emit(char c, struct Lexer* lexer) {
    putchar(c);
    lexer->position += 1;
}

void push(type t, struct Lexer* lexer) {
    lexer->stack[lexer->stack_index] = t;
    lexer->stack_index += 1;
}

void pop(struct Lexer* lexer) {
    lexer->stack_index -= 1;
}

type top(struct Lexer* lexer) {
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
            c = lexer->input[lexer->position];
            // handle escape sequences such as \\ and \'
            if(c == '\\'){
                emit('\\', lexer);
                emit(lexer->input[lexer->position+1], lexer);
                continue;
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
        emit('}', lexer);
        pop(lexer);
        struct State new_state = {comma_or_close};
        return new_state;   
    default:;
        struct State error_state = {error};
        return error_state;
    }
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
            c = lexer->input[lexer->position];
            // handle escape sequences such as \\ and \'
            if(c == '\\'){
                emit('\\', lexer);
                emit(lexer->input[lexer->position+1], lexer);
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
    }
    if(isdigit(c) || c == '.') {
        do {
            emit(c, lexer);
            c = lexer->input[lexer->position];
        } while(isdigit(c) || c == '.');
        struct State new_state = {comma_or_close};
        return new_state;           
    }

    struct State error_state = {error};
    return error_state;
}

struct State array(struct Lexer* lexer) {
    struct State new_state = {element};
    return new_state;
}

struct State element(struct Lexer* lexer) {
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
            c = lexer->input[lexer->position];
            // handle escape sequences such as \\ and \'
            if(c == '\\'){
                emit('\\', lexer);
                emit(lexer->input[lexer->position+1], lexer);
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
        emit(']', lexer);
        pop(lexer);
        struct State new_state = {comma_or_close};
        return new_state;   
    }
    if(isdigit(c) || c == '.') {
        do {
            emit(c, lexer);
            c = lexer->input[lexer->position];
        } while(isdigit(c) || c == '.');
        struct State new_state = {comma_or_close};
        return new_state;           
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
    lexer->can_advance = 0;
    return lexer->state;
}

struct State error(struct Lexer* lexer) {
    lexer->can_advance = 0;
    return lexer->state;
}


void parse(const char* string) {
    struct Lexer lexer = {
        string,
        0,
        {begin},
        1,
        0,
    };

    while(lexer.can_advance) {
        advance(&lexer);
    }
    putchar('\n');
    fflush(stdout);  
}

int main(){
    parse("{'hello': 'world'}");
    parse("{'hello': 'world',}");
    parse("{'hello': 'world', 'my': 'master'}");
    parse("{'hello': 'world', 'my': {'master': 'of Orion'}, 'test': 'xx'}");
    parse("{\"hello\": 12, 'world': 10002.21}");
    parse("{'hello': {}}");
    parse("{}");
    parse("[]");
    parse("[[[]]]");
    parse("[[[1]]]");
    parse("[1]");
    parse("[1,]");
    parse("[1, 2, 3, 4]");
    parse("['h', 'e', 'l', 'l', 'o']");
    parse("{'hello': [], 'world': [0]}");
    parse("{'hello': [1, 2, 3, 4]}");
    parse("[{'a':12}, {'b':33}]");
    parse("{\"a\":[{\"a\":12}, {\"b\":33}]}");
}
