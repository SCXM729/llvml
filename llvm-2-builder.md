# llvm::Value

用来表示一个“SSA寄存器”或者“SSA value", SSA value 在于它的值
只在相关指令执行的时候计算

# llvm::LLVMContext

an opaque object that owns a lot of core LLVM data structures, such as the type and constant value tables

# llvm::IRBuilder

a helper object that makes it easy to generate LLVM instructions. Instances of the IRBuilder class template keep track of the current place to insert instructions and has methods to create new instructions

1. 初始化

   ```c++
   std::unique_ptr<llvm::LLVMContext> TheContext;
   std::unique_ptr<llvm::IRBuilder<>> Builder;
   Builder = std::make_unique<llvm::IRBuilder<>>(*Context);
   ```
2. 函数调用指令

   ```c++
   // 原型
   CallInst*CreateCall(FunctionCallee Callee, ArrayRef<Value*>Args, const Twine&Name="");

   // 例子
   std::vector<Value*> ArgsV;
   Function* CalleeF;
   Builder->CreateCall(CalleeF, ArgsV, "calltmp");
   ```
3. 条件(if branch)

   ```c++
   // 条件分支
   BranchInst *CreateCondBr(Value *Cond, BasicBlock *True, BasicBlock *False, MDNode *BranchWeights = nullptr, MDNode *Unpredictable = nullptr);
   // 无条件分支
   BranchInst *CreateBr(BasicBlock *Dest);

   //用法
   Builder->CreateCondBr(Cond, ThenBB, ElseBB);
   Builder->CreateBr(MergeBB);
   ```
4. 比较指令

   ```c++
   // 判断是否不等于0
   Builder->CreateFCmpONE(EndCond, ConstantFP::get(*TheContext,APFloat(0.0)),"loopcond");
   ```

   对应的指令

   ```llvm
   %loopcond = fcmp one double %booltmp, 0.000000e+00
   ```
5. 算术指令
   在llvm IR中常量全部是唯一且共享的, 类型也是和常量一样，即不用new一个，只需要 `get`方法。`ConstantFP` 包含一个 `APFloat`数值，任意精度的浮点数

```c++
return ConstantFP::get(*TheContext, APFloat(Val));
```

- fcmp fcmp instruction总是返回一个“i1”值（只有一个bit位的整数）
  在add instruction中，左右操作数必须有相同的类型，而且返回类型也必须匹配

```c++
Builder->CreateFAdd(L, R, "addtmp");
Builder->CreateFSub(L, R, "subtmp");
Builder->CreateFMul(L, R, "multmp");
```

这里的addtemp，subtemp，multmp指的是计算结果的名字

6. 返回值

```c++
Builder->CreateRet(RetVal);
```

7. 数据类型转变

```c++
Builder->CreateUIToFP(L, Type::getDoubleTy(TheContext), "booltmp");
```

1. 与指令相关的其他操作

```c++
// 新的指令应该添加在指定块的结尾
void SetInsertPoint(INstruction*I)
```

```c++
// 返回当前基本块
BasicBlock *GetInsertBlock()const;
```

6. `PHINode` 表示一个phi节点

```c++
PHI*CreatePHI(Type*ty, unsigned NumReservedValus, const Twine&Name="");

//
PHINode *PN=Builder->CreatePHI(Type::getDoubleTy(*TheContext),2,"iftmp");
// 添加一个PHI节点 在PHI列表的结尾
PN->addIncoming(ThenV,ThenBB);
PN->addIncoming(ElseV,ElseBB);
```

# llvm::Module

contains functions and global variables. It is the top-level structure that the LLVM IR uses to contain code. It owns the memory for all of the IR that we generate.

初始化

```c++
std::unique_ptr<llvm::Module>  TheModule;
TheModule = std::make_unique<llvm::Module>("test", *TheContext);
```

1. `Function getFunction(StringRef Name) const`返回一个指针。如果不存在返回null
2. `setDataLayout(const DataLayout&other)`
   设置数据布局

# llvm::BasicBlock

创建一个新基本块

```c++
static BasicBlock* Create(LLVMContext&Context, const Twine&Name = "", Function*Parent = nullptr, BasicBlock*InsertBefore=nullptr)

// 例子
BasicBlock *BB = BasicBlock::Create(*TheContext, "entry");
```

获得父类，也就是返回一个 `Function`指针

```
BB->getParent();
```

```c++
// 构建一个任意类型的'0'常量
Constant*Constant::getNullValue(Type*Ty)

return Constant::getNullValue(Type::getDoubleTy(*TheContext));
```

# llvm::Function

构建一个函数

```c++
std::vector<Type*> Doubles(n, Type::getDoubleTy(*TheContext));
// 这里的false表示函数不是vararg
FunctionType *FT = FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);
// 生成一个IR函数，external linage表示函数也许不定义在当前module，或者函数可以被其他module调用。
Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());
```

1. `arg_size()` 参数数量

```c++
// 把基本块添加到Position的位置，返回新添加的BB
Function::iterator insert(Function::iterator Position, BasicBlock*BB);

// 用法
TheFunction->insert(TheFunction->end(), MergeBB);
```

2. `args()` 根据函数原型里的函数名字，来为每个函数参数设置名字。这一步不是必须的，但是可以让IR可读性更强

```c++
idx = 0;
for(auto &Arg:F->args())
    Arg.setName(Args[idx++]);
```

3. 检验函数的合法性,`verifyFunction() `对生成的代码做了一系列一致性检测，来捕获一些bug

```c++
verifyFunction(*TheFunction);
```

1. 在生成函数时遇到错误，直接删掉函数,

```c++
TheFunction->eraseFromParent();
```

1. 打印函数到一个输出流

```c++
FnIR->print(errs());
```
