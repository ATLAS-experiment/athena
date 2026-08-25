/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#include "SpecRegistry.h"

#include "GlobalSimulation/Object.h"
#include "GlobalSimulation/main_output.h"
#include "GlobalSimulation/topoc_pu_type.h"

#include "Utilities/binStrToHexStr.h"
#include "Utilities/hexStrToBinStr.h"

namespace GlobalSim {

  namespace {

    // Both directions for one spec. This is the only templated code left in
    // the text I/O: everything above it works through SpecEntry.
    template<class Spec>
    SpecEntry entryFor() {
      return {
          Spec::width,

          [](const SG::AuxElement& tob) {
            return binStrToHexStr(Object<Spec>(tob).bits().to_string());
          },

          [](SG::AuxElement& tob, const std::string& token) {
            Object<Spec> obj(tob);
            obj = typename Spec::bitset_type(hexStrToBinStr(token));
          }};
    }

  } // anonymous namespace

  const std::map<std::string, SpecEntry>& specRegistry() {
    // Function-local so that it is built on first use, rather than during
    // static initialisation where the order across translation units is
    // not defined.
    //
    // ---- Adding a TOB type: one line here, and nothing else. ----
    static const std::map<std::string, SpecEntry> specs = {
        {"topoc_pu_type", entryFor<topoc_pu_type>()},
        {"main_output_type", entryFor<main_output_type>()},
    };
    return specs;
  }

  std::string knownSpecNames() {
    std::string names;
    for (const auto& spec : specRegistry()) {
      if (not names.empty()) { names += ", "; }
      names += spec.first;
    }
    return names;
  }

}
