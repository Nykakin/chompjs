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
} type;

struct Lexer {
    const char* input;
    char* output;
    long input_position;
    long output_position;
    struct State state;
    int can_advance;

    short stack_index;
    int stack_size;
    type *stack;
    char current_quotation;
};

void advance(struct Lexer* lexer);
