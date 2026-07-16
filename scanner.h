/**
 * @file scanner.h
 * Introduction
 * ------------
 * Token types and the scanner's public API. Tokens are lightweight slices into
 * the original source buffer, so the source must outlive the tokens produced
 * from it.
 *
 * Lexical analysis of Lox source code.
 *
 * The scanner is a single-pass lexer that turns a raw source string into a
 * stream of `Token`s. It is intentionally stateful: a module-global scanner
 * holds the cursor, and `initScanner()`/`scanToken()` are the only public
 * entry points. Callers invoke `scanToken()` repeatedly until they hit
 * `TOKEN_EOF`.
 *
 * A `Token` refers back into the original source buffer by pointer + length,
 * so the source string must outlive the tokens derived from it (the compiler
 * does not copy lexeme text yet).
 */

#ifndef clox_scanner_h
#define clox_scanner_h

typedef enum {

    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,

    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,

    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUN,
    TOKEN_IF,
    TOKEN_NIL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,

    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType   type;
    const char* start;
    int         len;
    int         line;
} Token;

void initScanner(const char* source);

Token scanToken();

#endif