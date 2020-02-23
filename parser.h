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
    char* output;
    long input_position;
    long output_position;
    struct State state;
    LexerStatus lexer_status;

    short stack_index;
    int stack_size;
    Type *stack;
    char current_quotation;
};

void advance(struct Lexer* lexer);
