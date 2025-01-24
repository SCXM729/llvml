-- Constants
numeric constants are represented with ConstantFP class, which
holds the numeric value in an APFloat internally (APFloat has the
capability of holding floating point constants of Arbitrary).
    In LLVM IR, constants are all unique together and shared.
For this reason, The API uses the "foo::get()" idiom instead of
"new foo()" or "foo::Create()"
    The left and right operands of an add instruction must have 
the same type, and the result type of the add must match the 
operand types

-
```c++
Function *F = Function::Create(FunctionType, Function::ExternalLinkage, Name, TheModule.get())
```
"external linage" means that the function may be defined outside 
the current module and/or that it is callable by functions outside 
the module.