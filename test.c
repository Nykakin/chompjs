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
struct State comma_or_close(struct Lexer* lexer);
struct State end(struct Lexer* lexer);
struct State error(struct Lexer* lexer);

struct State {
    struct State (*change)(struct Lexer *);
};

struct Lexer {
    const char* input;
    long position;
    struct State state;
    int can_advance;

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

struct State begin(struct Lexer* lexer) {
    // Assume JSON starts from '{' or '[', otherwise it's an error
    switch(next_char(lexer)) {
    case '{':
        emit('{', lexer);
        struct State dictionary_state = {dictionary};
        return dictionary_state;
    default:;
        struct State error_state = {error};
        return error_state;
    }
}

struct State dictionary(struct Lexer* lexer) {
    // Assume dictionary key starts with a quotation - ', ", or ` - otherwise it's an error
    char c = next_char(lexer);
    switch(c) {
    case '\'':
    case '"':
    case '`':
        lexer->current_quotation = c;
        emit('"', lexer);
        
        struct State new_state = {key};
        return new_state;
    break;
    case '}':;
        emit('}', lexer);
        struct State next_key_state = {comma_or_close};
        return next_key_state;
    default:;
        struct State error_state = {error};
        return error_state;
    }
}

struct State key(struct Lexer* lexer) {
    while(1) {
        char c = lexer->input[lexer->position];
        // handle escape sequences such as \\ and \'
        if(c == '\\'){
            emit('\\', lexer);
            emit(lexer->input[lexer->position+1], lexer);

            lexer->position += 2;
            c = lexer->input[lexer->position];
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
        struct State new_state = {dictionary};
        return new_state;
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

struct State comma_or_close(struct Lexer* lexer) {
    switch(next_char(lexer)) {
    case ',':
        emit(',', lexer);
        struct State new_state = {dictionary};
        return new_state;
    case '}':
        emit('}', lexer);
        char c = next_char(lexer);
        if(c == ',') {
            emit(',', lexer);
            struct State next_key_state = {dictionary};
            return next_key_state;
        } else {
            struct State final_state = {end};
            return final_state;
        }
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
    };

    while(lexer.can_advance) {
        advance(&lexer);
    }
    putchar('\n');
    fflush(stdout);  
}

int main(){
    parse("{'hello': 'world'}");
    parse("{'hello': 'world', 'my': 'master'}");
    parse("{'hello': 'world', 'my': {'master': 'of Orion'}, 'test': 'xx'}");
    parse("{\"hello\": 12, 'world': 10002.21}");
    parse("{'hello': {}}");
    parse("{}");
}
