/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Instantiate BTaggingSelectionJsonTool for every working point shipped in an
// Xbb calibration JSON and assert that each one initialises cleanly. The JSON is
// structured as tagger -> jetAuthor -> { "meta", <OP>, <OP>, ... }; the test
// walks that structure and, for every (tagger, jetAuthor, OP) tuple, builds the
// tool and calls initialize(). It exits 0 only if every tuple initialises; any
// failure (including a bin-count inconsistency caught by loadBinConfig()) makes
// it return non-zero.
//
// Usage: BTaggingSelectionJsonInstantiationTester <json_path> [--allow-bin-mismatch]
//   <json_path> is resolved with PathResolverFindCalibFile (an absolute path is
//   returned as-is), e.g.
//   xAODBTaggingEfficiency/13And13p6TeV/MC20And23_2025-03-17_GN2Xv01-noSF_v1.json
//   --allow-bin-mismatch sets the tool's AllowBinCountMismatch property, which
//   downgrades a bin-count inconsistency from an init failure to a warning, so a
//   known-malformed JSON can be swept without an error exit.

#include "FTagAnalysisInterfaces/IBTaggingSelectionJsonTool.h"
#include "AsgTools/StandaloneToolHandle.h"
#include "PathResolver/PathResolver.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

#ifdef XAOD_STANDALONE
#include "xAODRootAccess/Init.h"
class GaudiException { public: const char* what() const { return ""; } };
#else
#include "POOLRootAccess/TEvent.h"
#endif

ANA_MSG_HEADER(testBTagSelJsonInst)
ANA_MSG_SOURCE(testBTagSelJsonInst, "BTaggingSelectionJsonInstantiationTester")
using namespace testBTagSelJsonInst;

int main ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {
#ifdef XAOD_STANDALONE
    ANA_CHECK_SET_TYPE(int);
#endif

    std::string jsonCalibPath;
    bool allowBinMismatch = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--allow-bin-mismatch") {
            allowBinMismatch = true;
        } else if (jsonCalibPath.empty()) {
            jsonCalibPath = arg;
        } else {
            ANA_MSG_ERROR("Unexpected extra argument: " << arg);
            return 2;
        }
    }
    if (jsonCalibPath.empty()) {
        ANA_MSG_ERROR("Usage: " << argv[0]
            << " <json_path> [--allow-bin-mismatch]");
        return 2;
    }

#ifdef XAOD_STANDALONE
    ANA_CHECK(xAOD::Init());
#else
    POOL::Init();
#endif

    // The tool resolves the calibration path itself, so pass the path straight
    // through to it. Resolve a copy here only to read the WP structure for
    // discovery.
    const std::string resolved = PathResolverFindCalibFile(jsonCalibPath);
    if (resolved.empty()) {
        ANA_MSG_ERROR("PathResolver could not find " << jsonCalibPath);
        return 1;
    }

    std::ifstream jstream(resolved);
    const auto jdata = nlohmann::ordered_json::parse(jstream);

    int nTuples = 0;
    int nFailed = 0;

    // tagger -> jetAuthor -> { "meta", <OP>, <OP>, ... }
    for (const auto& [tagger, authors] : jdata.items()) {
        if (!authors.is_object()) continue;
        for (const auto& [jetAuthor, opdict] : authors.items()) {
            if (!opdict.is_object()) continue;
            for (const auto& [op, opval] : opdict.items()) {
                if (op == "meta") continue;
                ++nTuples;

                asg::StandaloneToolHandle<IBTaggingSelectionJsonTool> tool(
                    "BTaggingSelectionJsonTool/XbbInst_" + tagger + "_" +
                    jetAuthor + "_" + op);

                try {
                  const bool ok =
                      tool.setProperty("JsonConfigFile", jsonCalibPath).isSuccess()
                      && tool.setProperty("OutputName", tagger).isSuccess()
                      && tool.setProperty("JetAuthor", jetAuthor).isSuccess()
                      && tool.setProperty("OperatingPoint", op).isSuccess()
                      && tool.setProperty("AllowBinCountMismatch",
                                          allowBinMismatch).isSuccess()
                      && tool.setProperty("OutputLevel",
                                          static_cast<int>(MSG::WARNING)).isSuccess()
                      && tool.initialize().isSuccess();

                  ANA_MSG_INFO((ok ? "PASS  " : "FAIL  ")
                               << tagger << " / " << jetAuthor << " / " << op);
                  if (!ok) ++nFailed;
                }
                catch (const GaudiException& e) {
                  ANA_MSG_ERROR("setProperty exception: {}", e.what());
                  ++nFailed;
                }
            }
        }
    }

    ANA_MSG_INFO("--- Summary --- tuples=" << nTuples << " failed=" << nFailed);

    if (nTuples == 0) {
        ANA_MSG_ERROR("No (tagger, jetAuthor, OP) tuples discovered in "
                      << jsonCalibPath);
        return 1;
    }
    return nFailed == 0 ? 0 : 1;
}
