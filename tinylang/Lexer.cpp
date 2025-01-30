#include "Lexer.h"
#include "Diagnostic.h"
void KeywordFilter::addKeyword(StringRef Keyword, TokenKind TokenCode) {
  HashTables.insert(std::make_pair(Keyword, TokenCode));
}

void KeywordFilter::addKeywords() {
#define KEYWORD(NAME, FLAGS) addkeyword(StringRef(#NAME), kw_##NAME);
#include "TokenKinds.h"
}
LLVM_READNONE inline bool isASCII(char Ch) {
  return static_cast<unsigned char>(Ch) <= 127;
}

LLVM_READNONE inline bool isVerticalWhitespace(char Ch) {
  return isASCII(Ch) && (Ch == '\r' || Ch == '\n');
}

LLVM_READNONE inline bool isHorizontalWhitespace(char Ch) {
  return isASCII(Ch) && (Ch == ' ' || Ch == '\t' || Ch == '\f' || Ch == '\v');
}

LLVM_READNONE inline bool isWhitespace(char Ch) {
  return isHorizontalWhitespace(Ch) || isVerticalWhitespace(Ch);
}

LLVM_READNONE inline bool isDigit(char Ch) {
  return isASCII(Ch) && Ch >= '0' && Ch <= '9';
}

LLVM_READNONE inline bool isHexDigit(char Ch) {
  return isASCII(Ch) && (isDigit(Ch) || (Ch >= 'A' && Ch <= 'F'));
}

LLVM_READNONE inline bool isIdentifierHead(char Ch) {
  return isASCII(Ch) &&
         (Ch == '_' || Ch >= 'A' && Ch <= 'Z' || (Ch >= 'a' && Ch <= 'z'));
}

LLVM_READNONE inline bool isIdentifierBody(char Ch) {
  return isIdentifierHead(Ch) || isDigit(Ch);
}

void Lexer::next(Token &Result) {
  while (*CurPtr && isWhitespace(*CurPtr)) {
    ++CurPtr;
  }
  if (!*CurPtr) {
    Result.setKind(TokenKind::eof);
    return;
  }

  if (isIdentifierHead(*CurPtr)) {
    identifier(Result);
    return;
  } else if (isDigit(*CurPtr)) {
    number(Result);
    return;
  } else if (*CurPtr == '"' || *CurPtr == '\'') {
    string(Result);
    return;
  } else {
    switch (*CurPtr) {
      // clang-format off
#define CASE(ch, tok) case ch: formToken(Result, CurPtr + 1, TokenKind::tok); break
      // clang-format on
      CASE('=', equal);
      CASE('#', hash);
      CASE('+', plus);
      CASE('-', minus);
      CASE('*', star);
      CASE('/', slash);
      CASE(',', comma);
      CASE('.', period);
      CASE(';', semi);
      CASE(')', r_paren);
#undef CASE
    case '(':
      if (*(CurPtr + 1) == '*') {
        comment();
        next(Result);
      } else
        formToken(Result, CurPtr + 1, TokenKind::l_paren);
      break;
    case ':':
      if (*(CurPtr + 1) == '=')
        formToken(Result, CurPtr + 2, TokenKind::lessequal);
      else
        formToken(Result, CurPtr + 1, TokenKind::less);
      break;
    case '<':
      if (*(CurPtr + 1) == '=')
        formToken(Result, CurPtr + 2, TokenKind::lessequal);
      else
        formToken(Result, CurPtr + 1, TokenKind::less);
      break;
    case '>':
      if (*(CurPtr + 1) == '=')
        formToken(Result, CurPtr + 2, TokenKind::greaterequal);
      else
        formToken(Result, CurPtr + 1, TokenKind::greater);
      break;
    default:
      Result.setKind(TokenKind::unknown);
    }
    return;
  }
}

void Lexer::identifier(Token &Result) {
  const char *Start = CurPtr;
  const char *End = CurPtr + 1;
  while (isIdentifierBody(*End))
    ++End;
  StringRef Name{Start, static_cast<size_t>(End - Start)};
  formToken(Result, End, Keywords.getKeyword(Name, TokenKind::identifier));
}

void Lexer::number(Token &Result) {
  const char *End = CurPtr + 1;
  TokenKind Kind = TokenKind::unknown;
  bool IsHex = false;
  while (*End) {
    if (!isHexDigit(*End))
      IsHex = true;
    ++End;
  }
  switch (*End) {
  case 'H': // hex number
    Kind = TokenKind::integer_literal;
    ++End;
    break;
  default: // decimal number
    if (IsHex)
      Diags.report(getLoc(), diag::err_hex_digit_in_decimal);
    Kind = TokenKind::integer_literal;
    break;
  }
  formToken(Result, End, Kind);
}

void Lexer::string(Token &Result) {
  const char *Start = CurPtr;
  const char *End = CurPtr + 1;
  while (*End && *End != *Start && !isVerticalWhitespace(*End))
    ++End;
  if (isVerticalWhitespace(*End)) {
    Diags.report(getLoc(), diag::err_unterminated_char_or_string);
  }
  formToken(Result, End + 1, TokenKind::string_literal);
}

void Lexer::comment() {
  const char *End = CurPtr + 2;
  unsigned Level = 1;
  while (*End && Level) {
    // check for nested comment
    if (*End == '(' && *(End + 1) == '*') {
      End += 2;
      Level++;
    }

    // check for end of comment
    else if (*End == '*' && *(End + 1) == ')') {
      End += 2;
      Level++;
    } else
      ++End;
  }
  if (!*End) {
    Diags.report(getLoc(), diag::err_unterminated_block_comment);
  }
  CurPtr = End;
}

void Lexer::formToken(Token &Result, const char *TokEnd, TokenKind Kind) {
  size_t TokLen = TokEnd - CurPtr;
  Result.Ptr = CurPtr;
  Result.Length = TokLen;
  Result.Kind = Kind;
  CurPtr = TokEnd;
}
