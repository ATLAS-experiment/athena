/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_BOOSTEDTTBARSKIMMINGTOOLALG_H
#define DERIVATIONFRAMEWORK_BOOSTEDTTBARSKIMMINGTOOLALG_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"
#include "Gaudi/Property.h"
#include <atomic>

namespace DerivationFramework {

  /** @class BoostedTTbarSkimmingToolAlg
      @brief Skimming tool that requires exactly one lepton and truth-level m_ttbar > cut
  */
  class BoostedTTbarSkimmingToolAlg : public extends<AthAlgTool, ISkimmingTool>
  {
    public:
      BoostedTTbarSkimmingToolAlg(const std::string& t, const std::string& n, const IInterface* p);
      virtual ~BoostedTTbarSkimmingToolAlg() = default;

      virtual StatusCode initialize() override { return StatusCode::SUCCESS; }
      virtual StatusCode finalize() override;

      /** Returns true if event passes the filter */
      virtual bool eventPassesFilter(const EventContext& ctx) const override;

      /** Property: m_ttbar cut in MeV */
      Gaudi::Property<double> m_ttbarCut{this, "ttbarCut", 0.0, "ttbar mass cut"};

    private:
      mutable std::atomic<unsigned int> m_ntot{0};
      mutable std::atomic<unsigned int> m_npass{0};
  };

} // namespace DerivationFramework

#endif // DERIVATIONFRAMEWORK_BOOSTEDTTBARSKIMMINGTOOLALG_H