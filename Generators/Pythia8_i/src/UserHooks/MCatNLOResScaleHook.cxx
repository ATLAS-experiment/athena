/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Author: Suat Baris Tuncay, based on a suggestion by Christian Preuss.
// Caps the shower starting scale of the resonances listed in
// MCatNLOResScaleHook:resonanceIDs at the LHE SCALUP, if they come from the
// incoming beam particles, directly or via a chain of listed resonances
// (e.g. t -> W b, W -> q q' with 6 and 24 listed).
// All other resonance decays keep Pythia's default starting scale of M/2.

#include "Pythia8_i/UserHooksFactory.h"
#include "UserHooksUtils.h"
#include "UserSetting.h"
#include <cstdlib>
#include <set>
#include <sstream>
#include <string>
#include <algorithm>
#include <iostream>

namespace Pythia8 {
  class MCatNLOResScaleHook;
}

Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::MCatNLOResScaleHook>
  MCatNLOResScaleHookCreator("MCatNLOResScaleHook");

namespace Pythia8 {

  class MCatNLOResScaleHook : public UserHooks {

  public:

    MCatNLOResScaleHook() : m_resonanceIDs("MCatNLOResScaleHook:resonanceIDs", "") {}

    bool canSetResonanceScale() override { return true; }

    double scaleResonance(int iRes, const Event& event) override {
      if (!m_parsed) parseResonanceIDs();
      const double mRes = event[iRes].mCalc();

      // Only resonance species requested by the user.
      if (m_ids.count(event[iRes].idAbs()) == 0) return 0.5 * mRes;

      // Walk up the decay chain
      int i = iRes;
      while (true) {
        const int iTop  = event[i].iTopCopyId();
        const int iMot1 = event[iTop].mother1();
        const int iMot2 = event[iTop].mother2();
        const bool fromBeamParticles = iMot1 > 0 && iMot2 > 0
          && event[iMot1].status() == -21 && event[iMot2].status() == -21;
        if (fromBeamParticles) return std::min(0.5 * mRes, infoPtr->scalup());
        const bool fromListedResonance = iMot1 > 0
          && (iMot2 == 0 || iMot2 == iMot1)
          && m_ids.count(event[iMot1].idAbs());
        if (!fromListedResonance) return 0.5 * mRes;
        i = iMot1;
      }
    }
  
  private:

    void parseResonanceIDs() {
      std::string ids = m_resonanceIDs(settingsPtr);
      for (char &c : ids)
        if (c == ',' || c == '{' || c == '}') c = ' ';
      std::istringstream in(ids);
      int id;
      while (in >> id) {
        const int idAbs = std::abs(id);
        if (m_allowedIDs.count(idAbs)) m_ids.insert(idAbs);
        else std::cout << "MCatNLOResScaleHook WARNING: resonance ID " << id
                       << " is not in the allowed list and will be ignored."
                       << std::endl;
      }
      if (m_ids.empty())
        std::cout << "MCatNLOResScaleHook WARNING: no allowed resonance requested"
                  << " via MCatNLOResScaleHook:resonanceIDs - the hook will not"
                  << " change any shower starting scale." << std::endl;
      m_parsed = true;
    }

    // Resonances this hook may act on.
    const std::set<int> m_allowedIDs = {6, 23, 24, 25};

    Pythia8_UserHooks::UserSetting<std::string> m_resonanceIDs;
    std::set<int> m_ids;
    bool m_parsed = false;
  
  };

}
