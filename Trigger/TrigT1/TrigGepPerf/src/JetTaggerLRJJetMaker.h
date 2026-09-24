/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_JETTAGGERLRJJETMAKER_H
#define TRIGGEPPERF_JETTAGGERLRJJETMAKER_H

/*
  JetTaggerLRJJetMaker:
  --------------------
  Floating point adapter around the bitwise JetTaggerLRJMaker, in the same way
  Gep::WTAConeJetMaker adapts WTAConeMaker. It digitizes the Gep float objects
  on the way in, runs the algorithm, and converts the digitized results back to
  physical units for the xAOD output.

  Clients whose input is already digitized should use JetTaggerLRJMaker
  directly instead: going through this class would re-quantize values that are
  already on the grid, which can cost an LSB.
*/

#include "Jet.h"
#include "Cluster.h"
#include "JetTaggerLargeRJet.h"

#include "TrigGepPerf/JetTaggerLRJMaker.h" // JetTaggerLRJMaker is the core header

#include <vector>

namespace Gep {

  // ------------------------------------------------------------------
  // Which input objects feed the seeds. Only the float path dispatches on
  // this: the bitwise core is handed seeds that are already digitized,
  // whatever they were made from.
  // ------------------------------------------------------------------
  enum class JetTaggerSeedSource {
    WTACone   = 0, // WTA-cone small-R jets (TrigGepPerf output)
    jFexSRJ   = 1, // (re-simulated) jFEX small-R jets
    gFexSRJ   = 2  // (re-simulated) gFEX small-R jets
  };

  class JetTaggerLRJJetMaker : public JetTaggerLRJMaker {
  public:
    JetTaggerLRJJetMaker() = default;

    // Seeds: undigitized Gep::Jet (WTACone, or jFEX/gFEX SRJ converted to
    // Gep::Jet inside GepJetAlg). Constituents: undigitized Gep::Cluster.
    // Et in both is taken from the TLorentzVector in MeV and converted to
    // GeV via m_cfg.inputEtToGeV.
    std::vector<Gep::LargeRJet>
    makeLargeRJets(const std::vector<Gep::Jet>& seeds,
                   const std::vector<Gep::Cluster>& constituents) const;

    void SetSeedSource(JetTaggerSeedSource s)   { m_seedSource = s; }
    JetTaggerSeedSource GetSeedSource() const   { return m_seedSource; }

  private:
    JetTaggerSeedSource m_seedSource {JetTaggerSeedSource::WTACone};

    // ---- input loading (stages 1 and 3 of jetTaggerEmulation.cc::eventLoop) ----
    std::vector<DigiObj> loadSeeds(const std::vector<Gep::Jet>& seeds) const;
    // Returns the digitized constituents (E_T-descending, > 2 GeV); fills
    // originalIndices with each one's index into constituents, so a digitized
    // slot can be mapped back to its source cluster.
    std::vector<DigiObj> loadConstituents(const std::vector<Gep::Cluster>& constituents,
                                          std::vector<int>& originalIndices) const;
  };

}

#endif // TRIGGEPPERF_JETTAGGERLRJJETMAKER_H
