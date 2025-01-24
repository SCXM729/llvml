1. ExecutionSession> ES
在orc(On-Request-Compilation)命名空间里面，是llvm jit编译架构的核心类，它管理着jit代码的执行。 
lookup(): 在jit session里面查找一个符号

2. DataLayout DL
表示目标机器的内存布局，包括数据类型的大小，对齐方式，以及指针的大小
getPointerSize(): 获取指针的大小
getTypeAllocSize(Type::getINt32Ty()) 获得int32类型的大小

3. MangleAndInterner Mangle 结合了名字改编和字符串内部的功能
 Mangles symbol names then uniques them in the context of an ExecutionSession.
 
4. RTDyldObjectLinkingLayer ObjectLayer
用RTDyld(real time dynamic loader)来加载object文件， 负责链接和加载object文件到内存中。
构造时接受一个ExecutionSession和一个DataLayout
5. IRCompileLayer CompileLayer 
Contains the definition for a basic, eagerly compiling layer of the JIT.
编译llvm IR到object文件

6. JITDylib& MainJD
   在jit的编译上下文中， 表示一个动态库。控制编译的代码
   
```c++
// a simple JIT definition
class KaleidoscopeJIT {
private:
  std::unique_ptr<ExecutionSession> ES;

  DataLayout DL;
  MangleAndInterner Mangle;

  RTDyldObjectLinkingLayer ObjectLayer;
  IRCompileLayer CompileLayer;

  JITDylib &MainJD;

public:
  KaleidoscopeJIT(std::unique_ptr<ExecutionSession> ES,
                  JITTargetMachineBuilder JTMB, DataLayout DL)
      : ES(std::move(ES)), DL(std::move(DL)), Mangle(*this->ES, this->DL),
        ObjectLayer(*this->ES,
                    []() { return std::make_unique<SectionMemoryManager>(); }),
        CompileLayer(*this->ES, ObjectLayer,
                     std::make_unique<ConcurrentIRCompiler>(std::move(JTMB))),
        MainJD(this->ES->createBareJITDylib("<main>")) {
    MainJD.addGenerator(
        cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(
            DL.getGlobalPrefix())));
    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
      ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
  }

  ~KaleidoscopeJIT() {
    if (auto Err = ES->endSession())
      ES->reportError(std::move(Err));
  }

  static Expected<std::unique_ptr<KaleidoscopeJIT>> Create() {
    auto EPC = SelfExecutorProcessControl::Create();
    if (!EPC)
      return EPC.takeError();

    auto ES = std::make_unique<ExecutionSession>(std::move(*EPC));

    JITTargetMachineBuilder JTMB(
        ES->getExecutorProcessControl().getTargetTriple());

    auto DL = JTMB.getDefaultDataLayoutForTarget();
    if (!DL)
      return DL.takeError();

    return std::make_unique<KaleidoscopeJIT>(std::move(ES), std::move(JTMB),
                                             std::move(*DL));
  }

  const DataLayout &getDataLayout() const { return DL; }

  JITDylib &getMainJITDylib() { return MainJD; }

  // adds an llvm IR module to the JIT, making its function available for
  // execution(with its memory managed by a ResourceTracker)
  Error addModule(ThreadSafeModule TSM, ResourceTrackerSP RT = nullptr) {
    if (!RT)
      RT = MainJD.getDefaultResourceTracker();
    return CompileLayer.add(RT, std::move(TSM));
  }

  // allow us to look up pointers to the compiled code
  Expected<ExecutorSymbolDef> lookup(StringRef Name) {
    return ES->lookup({&MainJD}, Mangle(Name.str()));
  }
};
```