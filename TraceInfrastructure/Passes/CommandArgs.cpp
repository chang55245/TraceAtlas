#include "Passes/CommandArgs.h"
#include "zlib.h"
cl::opt<int> KernelIndex("ki", cl::desc("Specify kernel index to trace"), cl::value_desc("kernel filename"));
cl::opt<std::string> KernelFilename("k", cl::desc("Specify filename for kernel"), cl::value_desc("kernel filename"));

cl::opt<std::string> DAGBBIOFile("dg", cl::desc("Specify filename for dag"), cl::value_desc("dag filename"));
cl::opt<std::string> NodeBBs("jr", cl::desc("Specify filename for jr"), cl::value_desc("jr filename"));


cl::opt<std::string> LoopTraceFile("lt", cl::desc("LoopTraceFile"), cl::value_desc("LoopTraceFile filename"));
cl::opt<std::string> InlineMap("lin", cl::desc("InlineMap"), cl::value_desc("InlineMap filename"));

cl::opt<bool> DumpLoads("DL", cl::desc("Dump load instruction information"), cl::desc("Dump load instruction information"), cl::init(true));
cl::opt<bool> DumpStores("DS", cl::desc("Dump store instruction information"), cl::desc("Dump store instruction information"), cl::init(true));

cl::opt<std::string> LibraryName("ln", cl::desc("Library Name"), cl::value_desc("Library name"));