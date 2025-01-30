#ifndef AST_H
#define AST_H

#include "TokenKinds.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/SMLoc.h"
#include <vector>

class Decl;
class FormalParameterDeclaration;
class Expr;
class Stmt;

using DeclList = std::vector<Decl *>;
using FormalParamList = std::vector<FormalParameterDeclaration *>;
using ExprList = std::vector<Expr *>;
using StmtList = std::vector<Stmt *>;

using namespace llvm;

class Ident {
  SMLoc Loc;
  StringRef Name;

public:
  Ident(SMLoc Loc, const StringRef &Name) : Loc{Loc}, Name{Name} {}
  SMLoc getLocation() { return Loc; }
  const StringRef &getName() { return Name; }
};

using IdentList = std::vector<std::pair<SMLoc, StringRef>>;

class Decl {
public:
  enum class DeclKind {
    DK_Module,
    DK_Const,
    DK_Type,
    DK_Var,
    DK_Param,
    DK_Proc
  };

private:
  const DeclKind Kind;

protected:
  Decl *EnclosingDecl;
  SMLoc Loc;
  StringRef Name;

public:
  Decl(DeclKind Kind, Decl *EnclosingDecl, SMLoc Loc, StringRef Name)
      : Kind{Kind}, EnclosingDecl{EnclosingDecl}, Loc{Loc}, Name{Name} {}
  DeclKind getKind() const { return Kind; }
  SMLoc getLocation() { return Loc; }
  StringRef getName() { return Name; }
  Decl *getEnclosingDecl() { return EnclosingDecl; }
};

class ModuleDeclaration : public Decl {
  std::vector<Decl *> Decls;
  std::vector<Stmt *> Stmts;

public:
  ModuleDeclaration(Decl *EnclosingDecl, SMLoc Loc, StringRef Name)
      : Decl{DeclKind::DK_Module, EnclosingDecl, Loc, Name} {}
  ModuleDeclaration(Decl *EnclosingDecl, SMLoc Loc, StringRef Name,
                    DeclList &Decls, StmtList &Stmts)
      : Decl{DeclKind::DK_Module, EnclosingDecl, Loc, Name}, Decls{Decls},
        Stmts{Stmts} {}
  const DeclList &getDecls() { return Decls; }
  void setDecls(DeclList &D) { Decls = D; }
  const StmtList &getStmts() { return Stmts; }
  void setStmts(StmtList &L) { Stmts = L; }

  static bool classof(const Decl *D) {
    return D->getKind() == DeclKind::DK_Module;
  }
};

class ConstantDeclaration(Decl *EnclosingDecL, SMLoc Loc, )

#endif
