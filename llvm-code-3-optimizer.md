run a few per-function optimizations as the user types the
function in.

# FunctionPassManager

is respondsible for collecting and manager a series of passes specified to function.

```c++
TheFPM = std::make_unique<FunctionPasszManager>(*TheModule);
TheFPM->addPass(ReassociatePass());
```

```
```

# PassInstrumentationCallbacks

This class provides instrumentation entry points for the Pass Manager, doing calls to callbacks registered in PassInstrumentationCallbacks.

# StandardInstrumentations

This class provides an interface to register all the standard pass instrumentations and manages their state (if any).
