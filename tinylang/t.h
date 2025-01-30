void f();
enum TokenKind : unsigned short {

#define TOK(ID) ID,

#ifndef TOK
#define TOK(ID)
#endif 

#ifndef PUNCTUATOR
#define PUNCTUATOR(ID, SP) TOK(ID)
#endif

#ifndef KEYWORD
#define KEYWORD(ID, FLAG) TOK(kw_ ## ID)
#endif

// These define members of the tok::* namespace.

PUNCTUATOR(r_paren,             ")")

KEYWORD(AND                         , KEYALL)

#undef KEYWORD
#undef PUNCTUATOR
#undef TOK
  NUM_TOKENS
};
