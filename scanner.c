/**
 * @file scanner.c
 *
 * Single-pass lexer for Lox.
 *
 * The scanner walks the source string with two pointers: `start` (beginning
 * of the lexeme currently being built) and `curr` (the lookahead position).
 * `scanToken()` skips leading whitespace, sets `start = curr`, then consumes
 * exactly one token and returns it. Whitespace, newlines, and `//` line
 * comments are skipped between tokens.
 *
 * Identifiers are recognized by their leading alpha/underscore character and
 * then disambiguated from keywords using a trie-like switch on the leading
 * character(s) (see `identifierType()`).
 */

#include "scanner.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

/**
 * The scanner's mutable cursor state. A single global instance is used for
 * simplicity, matching the structure of *Crafting Interpreters*.
 */
typedef struct {
    const char* start; /**< start of the current lexeme */
    const char* curr;  /**< current lookahead position */
    int         line;  /**< current 1-based line number */
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    scanner.start = source;
    scanner.curr  = source;
    scanner.line  = 1;
}

/**
 * @return True if `c` is a letter or underscore (identifier start/continue).
 */
static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

/**
 * @return True if `c` is a decimal digit.
 */
static bool isDigit(char c) { return c >= '0' && c <= '9'; }

/**
 * @return True when the cursor sits on the terminating NUL byte.
 */
static bool isAtEnd() { return *scanner.curr == '\0'; }

/**
 * Consume and return the current character, advancing the cursor.
 *
 * @return The character that was at the cursor.
 */
static char advance() {
    scanner.curr++;
    return scanner.curr[-1];
}

/**
 * @return The current character without consuming it.
 */
static char peek() { return *scanner.curr; }

/**
 * @return The character one past the cursor (NUL at end of input).
 */
static char peekNext() {
    if (isAtEnd()) return '\0';

    return scanner.curr[1];
}

/**
 * Conditionally consume an expected character.
 *
 * If the current character matches `expected`, consume it and return true;
 * otherwise leave the cursor untouched. Used to distinguish one- and
 * two-character operators (e.g. `!` vs `!=`).
 *
 * @param `expected`  Character to test for.
 * @return          True if matched and consumed, false otherwise.
 */
static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.curr != expected) return false;
    scanner.curr++;

    return true;
}

/**
 * Build a token of `type` spanning `[start, curr)`.
 *
 * @param `type`  Category for the new token.
 * @return      Token referring to the lexeme between start and curr.
 */
static Token makeToken(TokenType type) {
    Token token;
    token.type  = type;
    token.start = scanner.start;
    token.len   = (int)(scanner.curr - scanner.start);
    token.line  = scanner.line;

    return token;
}

/**
 * Produce an error token.
 *
 * @param `message`  Static error string used in lieu of a lexeme.
 * @return         A `TOKEN_ERROR` carrying `message`.
 */
static Token errorToken(const char* message) {
    Token token;
    token.type  = TOKEN_ERROR;
    token.start = message;
    token.len   = (int)strlen(message);
    token.line  = scanner.line;

    return token;
}

/**
 * Skip spaces, tabs, carriage returns, newlines, and `//` comments.
 *
 * Returns (without consuming) when the next character begins a real token.
 */
static void skipWhitespace() {
    for (;;) {
        char c = peek();

        switch (c) {
            case ' ':
            case '\r':
            case '\t':

                advance();
                break;
            case '\n':

                scanner.line++;
                advance();
                break;
            case '/':

                if (peekNext() == '/') {
                    // Consume to end of line (or end of input). The guard on
                    // !isAtEnd() prevents reading past the NUL terminator when
                    // a `//` comment is the last thing in the file.
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }

                break;
            default:
                return;
        }
    }
}

/**
 * Compare the tail of the current lexeme against a keyword.
 *
 * @param `start`  Offset within the lexeme where the keyword tail begins.
 * @param `len`    Length of the keyword tail `rest`.
 * @param `rest`   Expected remaining characters of the keyword.
 * @param `type`   Token type to return on a full match.
 * @return       `type` on match, otherwise `TOKEN_IDENTIFIER`.
 */
static TokenType checkKeyword(int start, int len, const char* rest,
                              TokenType type) {
    if (scanner.curr - scanner.start == start + len &&
        memcmp(scanner.start + start, rest, len) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

/**
 * Classify the just-scanned identifier as a keyword or plain name.
 *
 * Uses a switch on the leading character(s) as a compact trie. Any lexeme
 * that doesn't match a keyword tail falls through to the final
 * `return TOKEN_IDENTIFIER;`.
 *
 * @return The keyword `TokenType`, or `TOKEN_IDENTIFIER` for a user name.
 */
static TokenType identifierType() {
    switch (scanner.start[0]) {
        case 'a':
            return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'c':
            return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e':
            return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'i':
            return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n':
            return checkKeyword(1, 2, "il", TOKEN_NIL);
        case 'o':
            return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p':
            return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r':
            return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's':
            return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 'v':
            return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w':
            return checkKeyword(1, 4, "hile", TOKEN_WHILE);
        case 'f':
            if (scanner.curr - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a':
                        return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o':
                        return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u':
                        return checkKeyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;

        case 't':
            if (scanner.curr - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h':
                        return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r':
                        return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
    }

    return TOKEN_IDENTIFIER;
}

/**
 * Scan an identifier (or keyword).
 *
 * The leading alpha/underscore is consumed by the caller; this consumes any
 * trailing alphanumeric characters, then classifies the whole lexeme.
 *
 * @return A `TOKEN_IDENTIFIER` or keyword `Token`.
 */
static Token identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();

    return makeToken(identifierType());
}

/**
 * Scan a numeric literal.
 *
 * One or more digits, optionally followed by a fractional part of `.` and
 * more digits. A lone trailing `.` is not consumed (it would belong to a
 * following token).
 *
 * @return A `TOKEN_NUMBER`.
 */
static Token number() {
    while (isDigit(peek())) {
        advance();
    }

    if (peek() == '.' && isDigit(peekNext())) {
        advance();

        while (isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

/**
 * Scan a string literal delimited by `"`.
 *
 * Newlines inside the string advance the line counter so error positions
 * stay correct. An unterminated string yields an error token rather than
 * consuming to EOF silently.
 *
 * @return A `TOKEN_STRING`, or `TOKEN_ERROR` if the string is unterminated.
 */
static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    // Consume the closing quote.
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken() {
    skipWhitespace();
    scanner.start = scanner.curr;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (isDigit(c)) return number();
    if (isAlpha(c)) return identifier();

    switch (c) {
        case '(':
            return makeToken(TOKEN_LEFT_PAREN);
        case ')':
            return makeToken(TOKEN_RIGHT_PAREN);
        case '{':
            return makeToken(TOKEN_LEFT_BRACE);
        case '}':
            return makeToken(TOKEN_RIGHT_BRACE);
        case ';':
            return makeToken(TOKEN_SEMICOLON);
        case ',':
            return makeToken(TOKEN_COMMA);
        case '.':
            return makeToken(TOKEN_DOT);
        case '-':
            return makeToken(TOKEN_MINUS);
        case '+':
            return makeToken(TOKEN_PLUS);
        case '/':
            return makeToken(TOKEN_SLASH);
        case '*':
            return makeToken(TOKEN_STAR);
        case '!':
            return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"':
            return string();
    }

    return errorToken("Unexpected character.");
}
