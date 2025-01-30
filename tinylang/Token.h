#ifndef TINYLANG_TOKEN_H
#define TINYLANG_TOKEN_H

#include "TokenKinds.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/SMLoc.h"
using namespace llvm;
class Token {
  friend class Lexer;
  // point at the start of the token
  const char *Ptr;
  size_t Length;
  TokenKind Kind;

public:
  TokenKind getKind() const { return Kind; }
  void setKind(TokenKind K) { Kind = K; }

  // check if this token is a specific kind
  bool is(TokenKind K) const { return Kind == K; }
  bool isNot(TokenKind K) const { return Kind != K; }
  template <typename... Tokens> bool isOneOf(Tokens &&...Toks) const {
    return (... || is(Toks));
  }

  const char *getName() const { return getTokenName(Kind); }

  // SMLoc denotes the source position in messages
  SMLoc getLocation() const { return SMLoc::getFromPointer(Ptr); }

  size_t getLength() const { return Length; }

  StringRef getIdentifier() {
    assert(is(TokenKind::identifier) &&
           "Cannot get identifier of non-identifier");
    return StringRef(Ptr, Length);
  }

  StringRef getLiteralData() {
    assert(isOneOf(TokenKind::integer_literal, TokenKind::string_literal) &&
           "Cannot get literal data of non-literal");
    return StringRef(Ptr, Length);
  }
};
#endif
