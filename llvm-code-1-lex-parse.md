the first part is lex.  We define several enum Token
```c++
enum Token{
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number =  -5,
};
static std::string IdentifierStr;
static double NumVal;
```
```IdentifierStr``` holds the name of the identifier.
```NumVal``` holds its value

-- lex
The gettok() function is called to return the next token
```c++
static int gettok(){
    static int LastChar = ' ';

    // Skip any whitespace.
    while(isspace(LastChar))
        LastChar = getchar();
```
while we handling code, first skip any whitespace
Then we recognize the identifier and keywords like "def".
```c++
    if (isalpha(LastChar)){
        IdentifierStr = LastChar;
        while (isalnum((LastChar = getchar())))
            IdentifierStr += LastChar;
        
        if (IdentifierStr == "def")
            return tok_def;
        if (IdentifierStr == "extern")
            return tok_extern;
        return tok_identifier;
```

match number, simply processing input. when read "1.23.23", it will not emit error.
```c++
    if (isdigit(LastChar) || LastChar == '.'){
        std::string NumStr;
        do{
            NumStr += LastChar;
            LastChar = getchar();
        }while(isdigit(LastChar) || LastChar == '.');

        NumVal = strtod(NumStr.c_str(), nullptr);
        return tok_number;
    }
    if (LastChar == '#') {
        do 
            LastChar = getchar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok();
    }
```

if the input doesn't match any of the above cases, it either is an operator or the end of the file.
```c++
    if (LastChar == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ascii value
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}
```

--- Abstract Syntax Tree (AST)

using a combination of recursive descent parsing and operator precedence parsing

```c++
// Base class for all expression nodes
class ExprAST{
public:
    virtual ~ExprAST() = default;
    virtual llvm::Value* codegen() = 0;
};

// Expression class for numeric literals like "1.0".
class NumberExprAST: public ExprAST{
    double Val;

public:
    NumberExprAST(double Val): Val(Val){}
    llvm::Value* codegen() override;
};

// Expression class for references a variable, like "a".
class VariableExprAST: public ExprAST{
    std::string Name;

public:
    VariableExprAST(const std::string &Name): Name{Name}{}
};

// Expression class for a binary operator
class BinaryExprAST: public ExprAST{
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS):Op{Op},LHS{std::move(LHS)},RHS{std::move(RHS)}}
};

// Expression class for function calls
class CallExprAST: public ExprAST{
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args):Callee{Callee},Args{std::move(Args)}{}
};

// represents the prototype of a function, which captures its name and its argument names
class PrototypeAST{
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string &Name, std::vector<std::string>Args):Name{Name},Args{std::move(Args)}{}

    const std::string &getName(){return Name;}
};

// represents a function definition itself
class FunctionAST{
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body): Proto{std::move(Proto)}, Body{std::move(Body)}{}
};
```
the function formation in our language only have one expression. the following code won't work
```
def f()
    i = 3
    return i
```

other auxiliary variable and function
```c++
// Provide a simple token buffer. CurTok is the current token the parser is looking at. getNextToken() reads another token from the lexer and updates CurTok with its results.
static int CurTok;
static int getNextToken(){
    return CurTok = gettok();
}

std::unique_ptr<ExprAST> LogError(const char *Str){
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str){
    LogError(Str);
    return nullptr;
}
```

when counter tok_number token, it takes the current number value, creates a Num
```c++
// numberexpr -> number
static std::unique_ptr<ExprAST> ParseNumberExpr(){
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return std::move(Result);
}

// parenexpr -> '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr(){
    getNextToken(); // eat '('
    auto V = ParseExpression();
    if(!V)
        return nullptr;

    if(CurTok != ')')
        return LogError("expected ')' ");
    getNextToken(); // eat ')'
    return V;
}

// identifierexpr -> identifier
//                -> identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr(){
    std::string IdName = IdentifierStr;

    getNextToken(); // eat identifier

    if(CurTok != '(') // simple variable ref
        return std::make_unique<VariableExprAST>(IdName);

    // Call
    getNextToken(); // eat '('
    std::vector<std::unique_ptr<ExprAST>>Args;
    if(CurTok != ')'){
        while(true){
            if(auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else 
                return nullptr;

            if(CurTok == ')')
                break;

            if(CurTok != ',')
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    // eat the ')' 
    getNextToken();

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

// primary -> identifierexpr
//         -> numberexpr
//         -> parenexpr
static std::unique_ptr<ExprAST> ParsePrimary(){
    switch(CurTok){
        default:
            return LogError("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}
```

Binary expression is the harder part, so we use Operator-Precedence Parsing
```c++
// holds the precedence for each binary operator that is defined
static std::map<char,int> BinopPrecedence;

// Get the precedence of the pending binary operator token.
static int GetTokPrecedence(){
    if(!isascii(CurTok))
        return -1;

    // make sure it's a declared binop
    int TokPrec = BinopPrecedence[CurTok];
    if(TokPrec <= 0) return -1;
    return TokPrec;
}
```

The basic idea of operator precedence parsing is to break down an expression with potentially ambiguous binary operators into pieces. Operator precedence parsing considers this as a stream of primary expressions separated by binary operators. the expression "a + b + (c + d) * e * f + g" , first parse the leading primary expression "a", then it will see the pairs \[+, b][+, (c+d)][\*,e][*,f] and [\+,g]

```c++
// expression -> primary binoprhs
static std::unique_ptr<ExprAST> ParseExpression(){
    auto LHS = ParsePrimary();
    if(!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}
```
ParseBinOpRHS is the function that parses the sequences of pairs for us. It takes a precedence and a pointer to an expression for the part that has been parsed so far.

a example is process " x + y * z - a", first parse "x" as primary, then we see " + y ", the plus's prec is 20 as TokPrec, lookahead a operator, it is multiple sign as NextPrec, its prec is 40 greater than 20. So we need to process "* z" and consider it as RHS. When encounter "-", the prec of "-" is lower than the sum of the prec of "+" and 1, so consider "x + y * z" as LHS. The rest part is same.
```c++
// binoprhs -> '(' '+' primary ')'*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS){
    // if this is a binop, find its precedence.
    while(true){
        int TokPrec = GetTokPrecedence();

        // if this is a binop that binds at least as tightly as the current binop, consume it, otherwise we are done
        if(TokPrec < ExprPrec)
            return LHS;
```
This code gets the precedence of the current token and checks to see if it is too low. Because we defined invalid tokens to have a precedence of -1, this check implicitly knows that the pair-tream ends when the token stream runs out of binary operators.
if the current token's precedence is lower than the previous one, it determine that the original LHS part is constructed well.
```c++
        // this is a binop
        int BinOp = CurTok;
        getNextToken(); // eat binop

        // Parse the primary expression after the binary operator
        auto RHS = ParsePrimary();
        if(!RHS)
            return nullptr;
```
look ahead at "binop" to determine its precedence and compare it to BinOp's precedence. If the next operator's precedence is higher than the current one, integrate it as a whole. The "+1" indicate that if the following operator shouldn't lower than the current one
```c++
        // if BinOp binds less tightly with RHS than the operator after RHS, let the pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec){
            RHS = ParseBinOpRHS(TokPrec+1, std::move(RHS));
            if(!RHS)
                return nullptr;
        }
        // merge LHS/RHS
        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    } // loop around to the top of the while loop
}
```
```c++
/// prototype
///   -> id '(' id* (',' id)* ')'
///   -> binary LETTER number? (id, id)
static std::unique_ptr<PrototypeAST> ParsePrototype() {
  std::string FnName;

  unsigned Kind = 0; // 0 = identifier, 1 = unary , 2 = binary
  unsigned BinaryPrecedence = 30;

  switch (CurTok) {
  default:
    return LogErrorP("Expected function name in prototype");
  case tok_identifier:
    FnName = IdentifierStr;
    Kind = 0;
    getNextToken();
    break;
  case tok_unary:
    getNextToken();
    if (!isascii(CurTok))
      return LogErrorP("Expected unary operator");
    FnName = "unary";
    FnName += (char)CurTok;
    Kind = 1;
    getNextToken();
    break;
  case tok_binary:
    getNextToken(); // eat 'binary'
    if (!isascii(CurTok))
      return LogErrorP("Expected binary operator");
  ```
symbol names in LLVM symbol table are allowed to have any character 
in them, including embedded nul character. FnName builds names like 
"binary@"
```c++
    FnName = "binary";
    FnName += (char)CurTok;
  ```
```c++
    Kind = 2;
    getNextToken(); // eat LETTER

    // read the precedence if present
    if (CurTok == tok_number) {
      if (NumVal < 1 || NumVal > 100)
        return LogErrorP("invalid precedence: must be 1..100");
      BinaryPrecedence = (unsigned)NumVal;
      getNextToken(); // eat number
    }
    break;
  }

  if (CurTok != '(')
    return LogErrorP("Expected '(' in prototype");

  std::vector<std::string> ArgNames;
  while (getNextToken() == tok_identifier) { // eat id
    ArgNames.push_back(IdentifierStr);
    getNextToken(); // eat ','
    if (CurTok != ')') {
      if (CurTok != ',')
        LogErrorP("Expected ',' in prototypes");
    } else
      break;
  }
  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  // success.
  getNextToken(); // eat ')'.

  // verify right number of names for operator
  if (Kind && ArgNames.size() != Kind)
    return LogErrorP("Invalid number of operands for operator");

  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames), Kind != 0,
                                        BinaryPrecedence);
}
```
the function in our language only have one statement
```c++
// definition -> 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition(){
    getNextToken();   // eat 'def'
    auto Proto = ParsePrototype();
    if(!Proto) return nullptr;

    if(auto E = ParseExpression())
        return std::make_unique<FunctionAST>(std::move(Prot),std::move(E));
    return nullptr;
}

// external -> extern prototype
static std::unique_ptr<PrototypeAST> ParseExtern(){
    getNextToken();   // eat 'extern'
    return ParsePrototype();
}

// toplevelexpr -> expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr(){
    if(auto E = ParseExpression()){
        // make an anonymous proto
        auto Proto = std::make_unique<PrototypeAST>("__anon_expr",std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(Prot),std::move(E));
    }
    return nullptr;
}

static void HandleDefinition(){
    if(ParseDefinition()){
        fprintf(stderr,"Parsed a function definition.\n");
    }else{
        // skip token for error recovery
        getNextToken();
    }
}

static void HandleTopLevelExpression(){
    // Evaluate a top-level expression into an anonymous function.
    if(ParseTopLevelExpr()){
        fprintf(stderr,"Parsed a top-level expr\n");
    }else{
        // skip token for error recovery
        getNextToken();
    }
}
// top -> definition | external | expression | ';'
static void MainLoop(){
    while(true){
        fprintf(stderr,"ready> ");
        switch(CurTok){
            case tok_eof;
                return;
            case ';':  // ignore top-level semicolons
                getNextToken();
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExten();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}
        
```
```c++
int main(){
    // Install standard binary operators
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;

    // prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    // run the main "intepreter loop" now
    MainLoop();

    return 0;
}
```
