#ifndef SEMA_H
#define SEMA_H
#include "AST.h"
#include "Diagnostic.h"
#include "Scope.h"
#include "TokenKinds.h"

class Sema {
  friend class EnterDeclScope;
  void enterScope(Decl *);
  void leaveScope();

  bool isOperatorForType(TokenKind Op, TypeDeclaration *Ty);
  void checkFormalAndActualParameters(SMLoc Loc, const FormalParamList &Formals,
                                      const ExprList &Acutals);
};

#endif
