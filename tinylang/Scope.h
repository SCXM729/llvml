#ifndef SEMA_SCOPE_H
#define SEMA_SCOPE_H
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
class Decl;
class Scope {
  Scope *Parent;
  llvm::StringMap<Decl *> Symbols;

public:
  Scope(Scope *Parent = nullptr) : Parent{Parent} {}
  bool insert(Decl *Declaration);
  Decl *lookup(llvm::StringRef Name);

  Scope *getParent() { return Parent; }
};
#endif
