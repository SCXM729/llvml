#include "TokenKinds.h"
#include "llvm/Support/ErrorHandling.h"

static const char *const TokNames[] = {
#define TOK(ID) #ID,
#define KEYWORD(ID, FLAG) #ID,
#include "TokenKinds.def"
    nullptr};

const char *getTokenName(TokenKind Kind) {
  if (Kind < NUM_TOKENS)
    return TokNames[Kind];
  llvm_unreachable("unknown TokenKinds");
  return nullptr;
}

const char *getPunctuatorSpelling(TokenKind Kind) {
  switch (Kind) {
    // clang-format off
#define PUNCTUATOR(ID, SP)  case ID:  return SP;
    // clang-format on
#include "TokenKinds.def"
  default:
    break;
  }
  return nullptr;
}

const char *getKeywordSpelling(TokenKind Kind) {
  switch (Kind) {
    // clang-format off
#define KEYWORD(ID, FLAG)  case kw_##ID:  return #ID;
    // clang-format on
#include "TokenKinds.def"
  default:
    break;
  }
  return nullptr;
}
