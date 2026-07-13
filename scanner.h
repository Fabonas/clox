/**
 * @file scanner.h
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

/**
 * Every lexical category the language distinguishes, grouped by shape:
 * single-character operators, one-or-two character operators, literals, and
 * reserved words.
 */
typedef enum {
    // Single-character tokens.
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
    // One or two character tokens.
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    // Literals.
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    // Keywords.
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

    TOKEN_ERROR,  /**< carries a message string instead of a lexeme */
    TOKEN_EOF
} TokenType;

/**
 * A single lexeme: its category and a `[start, start+len)` slice into the
 * original source, plus the 1-based line where it begins.
 */
typedef struct {
    TokenType   type;   /**< lexical category */
    const char* start;  /**< pointer to first char of the lexeme in source */
    int         len;    /**< lexeme length in bytes */
    int         line;   /**< 1-based source line where the lexeme begins */
} Token;

/**
 * Point the scanner at a fresh source string.
 *
 * Resets the cursor to the start of `source` and line counting to 1. The
 * caller must keep `source` alive for as long as tokens are read from it.
 *
 * @param `source`  NUL-terminated source text to scan.
 */
void initScanner(const char* source);

/**
 * Scan and return the next token from the source.
 *
 * Skips leading whitespace/comments, then consumes exactly one lexeme. At
 * end of input returns `TOKEN_EOF` (repeatedly). On a lexical error returns
 * a `TOKEN_ERROR` whose `start` points at a static message string.
 *
 * @return The next `Token` in the stream.
 */
Token scanToken();

#endif
