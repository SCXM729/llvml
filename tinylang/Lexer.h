#ifndef TINYLANG_LEXER_H
#define TINYLANG_LEXER_H
#include "Diagnostic.h"
#include "Token.h"
#include "llvm/ADT/StringMap.h"
class KeywordFilter {
  llvm::StringMap<TokenKind> HashTables;
  void addKeyword(llvm::StringRef Keyword, TokenKind TokenCode);

public:
  void addKeywords();
  // return the token kind of the given string, or a default value if the string
  // does not represent a keyword
  TokenKind getKeyword(StringRef Name,
                       TokenKind DefaultTokenCode = TokenKind::unknown) {
    auto Result = HashTables.find(Name);
    if (Result != HashTables.end())
      return Result->second;
    return DefaultTokenCode;
  }
};

class Lexer {
  SourceMgr &SrcMgr;
  DiagnosticsEngine &Diags;

  const char *CurPtr;
  StringRef CurBuf;

  // current buffer index we're lexing from as managed by the SourceMgr object
  unsigned CurBuffer = 0;
  KeywordFilter Keywords;

public:
  Lexer(SourceMgr &SrcMgr, DiagnosticsEngine &Diags)
      : SrcMgr{SrcMgr}, Diags{Diags} {
    CurBuffer = SrcMgr.getMainFileID();
    CurBuf = SrcMgr.getMemoryBuffer(CurBuffer)->getBuffer();
    CurPtr = CurBuf.begin();
    Keywords.addKeywords();
  }

  DiagnosticsEngine &getDiagnostics() const { return Diags; }

  // return the next token from the input
  void next(Token &Result);

  // gets source code buffer
  StringRef getBuffer() const { return CurBuf; }

private:
  void identifier(Token &Result);
  void number(Token &Result);
  void string(Token &Result);
  void comment();

  SMLoc getLoc() { return SMLoc::getFromPointer(CurPtr); }
  void formToken(Token &Result, const char *TokEnd, TokenKind Kind);
};
#endif
