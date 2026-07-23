/**
 * @file compiler.c
 * Introduction
 * ------------
 * This is clox's single-pass expression compiler. It reads tokens from the
 * scanner, builds a Pratt-parser table over token types, and emits bytecode
 * into a `Chunk`. Error reporting uses panic-mode synchronization so that one
 * mistake does not flood the user with cascading diagnostics.
 *
 * Single-pass Pratt parser and bytecode emitter.
 *
 * The compiler consumes tokens from the scanner and emits instructions into
 * a `Chunk` using a precedence-climbing expression parser. It supports
 * literals, grouping, unary operators, and binary operators. Errors are
 * reported with line numbers and switch the parser into panic mode to avoid
 * cascading diagnostics.
 */

#include "compiler.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "object.h"
#include "scanner.h"
#include "table.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token curr;
    Token prev;
    bool  hadError;
    bool  panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct {
    ParseFn    prefix;
    ParseFn    infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int   depth;
} Local;

typedef struct {
    Local locals[UINT8_COUNT];
    int   localCount;
    int   scopeDepth;
} Compiler;

Parser    parser;
Compiler *curr = NULL;
Chunk    *compilingChunk;

static Chunk *currentChunk() { return compilingChunk; }

static void errorAt(Token *token, const char *message) {
    if (parser.panicMode) return;

    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type != TOKEN_ERROR) {
        fprintf(stderr, " at '%.*s'", token->len, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void errorAtCurrent(const char *message) {
    errorAt(&parser.curr, message);
}

static void error(const char *message) { errorAt(&parser.prev, message); }

static void advance() {
    parser.prev = parser.curr;

    for (;;) {
        parser.curr = scanToken();
        if (parser.curr.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.curr.start);
    }
}

static void consume(TokenType type, const char *message) {
    if (parser.curr.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type) { return parser.curr.type == type; }

static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

static void emitByte(u8 byte) {
    writeChunk(currentChunk(), byte, parser.prev.line);
}

static void emitBytes(u8 byte1, u8 byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() { emitByte(OP_RETURN); }

static u8 makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (u8)constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void initCompiler(Compiler *compiler) {
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    curr                 = compiler;
}

static void endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void beginScope() { curr->scopeDepth++; }

static void endScope() {
    curr->scopeDepth--;

    while (curr->localCount > 0 &&
           curr->locals[curr->localCount - 1].depth > curr->scopeDepth) {
        emitByte(OP_POP);
        curr->localCount--;
    }
}

static void       expression();
static void       statement();
static void       declaration();
static ParseRule *getRule(TokenType type);
static void       parsePrecedence(Precedence precedence);

static u8 identifierConstant(Token *name) {
    ObjString *identifier = copyString(name->start, name->len);

    return makeConstant(OBJ_VAL(identifier));
}

static bool identifierEqual(Token *a, Token *b) {
    if (a->len != b->len) return false;
    return memcmp(a->start, b->start, a->len) == 0;
}

static int resolveLocal(Compiler *compiler, Token *name) {
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local *local = &compiler->locals[i];

        if (identifierEqual(name, &local->name)) {
            if (local->depth == -1)
                error("Can't read local variable in its own initializer.");
            return i;
        }
    }

    return -1;
}

static void addLocal(Token name) {
    if (curr->localCount == UINT8_COUNT) {
        error("Too many local variables.");
        return;
    }

    Local *local = &curr->locals[curr->localCount++];
    local->name  = name;
    local->depth = -1;
}

static void declareVariable() {
    if (curr->scopeDepth == 0) return;

    Token *name = &parser.prev;

    for (int i = curr->localCount - 1; i >= 0; i--) {
        Local *local = &curr->locals[i];

        if (local->depth != -1 && local->depth < curr->scopeDepth) {
            break;
        }

        if (identifierEqual(name, &local->name)) {
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(*name);
}

static u8 parseVariable(const char *errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (curr->scopeDepth > 0) return 0;

    return identifierConstant(&parser.prev);
}

static void markInitialized() {
    curr->locals[curr->localCount - 1].depth = curr->scopeDepth;
}

static void defineVariable(u8 global) {
    if (curr->scopeDepth > 0) {
        markInitialized();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

static void binary(bool canAssign) {
    TokenType  operatorType = parser.prev.type;
    ParseRule *rule         = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;

        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;

        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;

        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT);
            break;

        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;

        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT);
            break;

        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;

        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;

        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;

        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;

        default:
            return;
    }
}

static void literal(bool canAssign) {
    switch (parser.prev.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;

        case TOKEN_NIL:
            emitByte(OP_NIL);
            break;

        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;

        default:
            return;
    }
}

static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void expression() { parsePrecedence(PREC_ASSIGNMENT); }

static void block() {
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void varDeclaration() {
    u8 global = parseVariable("Expected variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");

    defineVariable(global);
}

static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression");
    emitByte(OP_POP);
}

static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.curr.type != TOKEN_EOF) {
        if (parser.prev.type == TOKEN_SEMICOLON) return;
        switch (parser.curr.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default:;
        }

        advance();
    }
}

static void declaration() {
    if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }

    if (parser.panicMode) synchronize();
}

static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

static void number(bool canAssign) {
    double value = strtod(parser.prev.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void string(bool canAssign) {
    emitConstant(
        OBJ_VAL(copyString(parser.prev.start + 1, parser.prev.len - 2)));
}

static void namedVariable(Token name, bool canAssign) {
    u8  getOp, setOp;
    int arg = resolveLocal(curr, &name);

    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        arg   = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(setOp, (u8)arg);
    } else {
        emitBytes(getOp, (u8)arg);
    }
}

static void variable(bool canAssign) { namedVariable(parser.prev, canAssign); }

static void unary(bool canAssign) {
    TokenType operatorType = parser.prev.type;

    parsePrecedence(PREC_UNARY);

    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;

        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
        default:
            return;
    }
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]    = {grouping, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN]   = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE]   = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA]         = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT]           = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS]         = {unary, binary, PREC_TERM},
    [TOKEN_PLUS]          = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON]     = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH]         = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR]          = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG]          = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL]    = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL]         = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL]   = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER]       = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS]          = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]    = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER]    = {variable, NULL, PREC_NONE},
    [TOKEN_STRING]        = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER]        = {number, NULL, PREC_NONE},
    [TOKEN_AND]           = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS]         = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE]          = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE]         = {literal, NULL, PREC_NONE},
    [TOKEN_FOR]           = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN]           = {NULL, NULL, PREC_NONE},
    [TOKEN_IF]            = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL]           = {literal, NULL, PREC_NONE},
    [TOKEN_OR]            = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT]         = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN]        = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER]         = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS]          = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE]          = {literal, NULL, PREC_NONE},
    [TOKEN_VAR]           = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE]         = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR]         = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF]           = {NULL, NULL, PREC_NONE},
};

static void parsePrecedence(Precedence precedence) {
    advance();

    ParseFn prefixRule = getRule(parser.prev.type)->prefix;

    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.curr.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.prev.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static ParseRule *getRule(TokenType type) { return &rules[type]; }

bool compile(const char *source, Chunk *chunk) {
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler);
    compilingChunk = chunk;

    parser.hadError  = false;
    parser.panicMode = false;

    advance();

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    endCompiler();

    return !parser.hadError;
}
