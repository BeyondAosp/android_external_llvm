//===--- Main.cpp - The LLVM Compiler Driver --------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open
// Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  llvmc::Main function - driver entry point.
//
//===----------------------------------------------------------------------===//

#include "llvm/CompilerDriver/AutoGenerated.h"
#include "llvm/CompilerDriver/BuiltinOptions.h"
#include "llvm/CompilerDriver/CompilationGraph.h"
#include "llvm/CompilerDriver/Error.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/System/Path.h"

#include <sstream>
#include <string>

namespace cl = llvm::cl;
namespace sys = llvm::sys;
using namespace llvmc;

namespace {

  std::stringstream* GlobalTimeLog;

  /// GetTempDir - Get the temporary directory location. Returns non-zero value
  /// on error.
  int GetTempDir(sys::Path& tempDir) {
    // The --temp-dir option.
    if (!TempDirname.empty()) {
      tempDir = TempDirname;
    }
    // GCC 4.5-style -save-temps handling.
    else if (SaveTemps == SaveTempsEnum::Unset) {
      tempDir = sys::Path::GetTemporaryDirectory();
      return 0;
    }
    else if (SaveTemps == SaveTempsEnum::Obj && !OutputFilename.empty()) {
      tempDir = OutputFilename;
      tempDir = tempDir.getDirname();
    }
    else {
      // SaveTemps == Cwd --> use current dir (leave tempDir empty).
      return 0;
    }

    if (!tempDir.exists()) {
      std::string ErrMsg;
      if (tempDir.createDirectoryOnDisk(true, &ErrMsg)) {
        PrintError(ErrMsg);
        return 1;
      }
    }

    return 0;
  }

  /// BuildTargets - A small wrapper for CompilationGraph::Build. Returns
  /// non-zero value in case of error.
  int BuildTargets(CompilationGraph& graph, const LanguageMap& langMap) {
    int ret;
    sys::Path tempDir;
    bool toDelete = (SaveTemps == SaveTempsEnum::Unset);

    if (int ret = GetTempDir(tempDir))
      return ret;

    ret = graph.Build(tempDir, langMap);

    if (toDelete)
      tempDir.eraseFromDisk(true);

    return ret;
  }
}

namespace llvmc {

// Used to implement -time option. External linkage is intentional.
void AppendToGlobalTimeLog(const std::string& cmd, double time) {
  *GlobalTimeLog << "# " << cmd << ' ' << time << '\n';
}

// Sometimes user code wants to access the argv[0] value.
const char* ProgramName;

int Main(int argc, char** argv) {
  int ret = 0;
  LanguageMap langMap;
  CompilationGraph graph;

  ProgramName = argv[0];

  cl::ParseCommandLineOptions
    (argc, argv,
     /* Overview = */ "LLVM Compiler Driver (Work In Progress)",
     /* ReadResponseFiles = */ false);

  if (int ret = autogenerated::RunInitialization(langMap, graph))
    return ret;

  if (CheckGraph) {
    ret = graph.Check();
    if (!ret)
      llvm::errs() << "check-graph: no errors found.\n";

    return ret;
  }

  if (ViewGraph) {
    graph.viewGraph();
    if (!WriteGraph)
      return 0;
  }

  if (WriteGraph) {
    const std::string& Out = (OutputFilename.empty()
                              ? std::string("compilation-graph.dot")
                              : OutputFilename);
    return graph.writeGraph(Out);
  }

  if (Time) {
    GlobalTimeLog = new std::stringstream;
    GlobalTimeLog->precision(2);
  }

  ret = BuildTargets(graph, langMap);

  if (Time) {
    llvm::errs() << GlobalTimeLog->str();
    delete GlobalTimeLog;
  }

  return ret;
}

} // end namespace llvmc
