#ifndef TINYLANG_TOKENKINDS_H
#define TINYLANG_TOKENKINDS_H

#include "llvm/Support/Compiler.h"
enum class TokenKind : unsigned short {
#define TOK(ID) ID,
#include "TokenKinds.def"

  NUM_TOKENS
};
const char *getTokenName(TokenKind Kind) LLVM_READNONE;
const char *getPunctuatorSpelling(TokenKind Kind) LLVM_READNONE;
const char *getKeywordSpelling(TokenKind Kind) LLVM_READNONE;
#endif
