#include <stddef.h>

struct Token;
struct Lexer;

struct State begin(struct Lexer* lexer);
struct State dictionary(struct Lexer* lexer);
struct State key(struct Lexer* lexer);
struct State colon(struct Lexer* lexer);
struct State value(struct Lexer* lexer);
struct State array(struct Lexer* lexer);
struct State comma_or_close(struct Lexer* lexer);
struct State end(struct Lexer* lexer);
struct State error(struct Lexer* lexer);

struct State {
    struct State (*change)(struct Lexer *);
};

typedef enum {
    DICT,
    ARRAY
} Type;

typedef enum {
    CAN_ADVANCE,
    FINISHED,
    ERROR,
} LexerStatus;

struct Lexer {
    const char* input;
    size_t output_size;
    char* output;
    size_t input_position;
    size_t output_position;
    struct State state;
    LexerStatus lexer_status;

    size_t stack_index;
    size_t stack_size;
    Type *stack;
    char current_quotation;
    int is_jsonlines;
};

void advance(struct Lexer* lexer);
void init(struct Lexer* lexer, const char* string, size_t initial_stack_size, int is_jsonlines);
