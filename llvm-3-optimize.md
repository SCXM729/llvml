IR Builder automatically implements constant folding, but is limited
by the fact that it does all of its analysis inline with the code
as it is built.
passes can be divided into transform pass and analysis pass.

# analysis pass

```c++
using llvm;
TheFPM = std::make_unique<FunctionPassManager>{};
TheLAM = std::make_unique<LoopAnalysisManager>{};
TheFAM = std::make_unique<FunctionAnalysisManager>{};
TheCGAM = std::make_unique<CGSCCAnalysisManager>{};
```

# FunctionPassManager

is respondsible for collecting and manager a series of passes specified to function.

```c++
TheFPM = std::make_unique<FunctionPasszManager>(*TheModule);
// add transform passes
TheFPM->addPass(ReassociatePass());
```

```
```

PassInstrumentationCallbacks和StandardInstrumentations允许开发者自定义在passes之间做些什么

PassBuilder 可以注册被转换pass使用的分析pass

```c++
PassBuilder PB;
PB.registerModuleAnalysis(*TheMAM);
PB.registerFunctionAnalysis(*TheFAM);
PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);
```

在解析完函数后优化

```c++
TheFPM->run(*TheFunction, *TheFAM);
```

可以通过观察clang运行的pass来学习，opt工具可以在命令行上允许体验pass

duplication of symbols in separate modules in not allowed since llvm-9, 主要原因是在并发编译的时候，把符号名称作为追踪键

符号解析规则： 先从所有添加在jit的module里面搜索，其次调用dlsym("sin")
,这里的sin未定义。它调用了libm版本的sin函数。
直接的好处我们可以通过编写任意c++代码以支持操作来扩展语言

```c++
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a double and returns 0.
extern "C" DLLEXPORT double putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}
```

# 数据类型

llvm::ConstantFP 浮点数(float, double)
ConstantFP *get(LLVMContext &Context, const APFloat &V);
构建一个数值

```c++
llvm::ConstantFP::get(*TheContext, llvm::APFloat(1,0));
```
