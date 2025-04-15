/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TriggerSkimmingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_TRIGGERSKIMMINGTOOL_H
#define DERIVATIONFRAMEWORK_TRIGGERSKIMMINGTOOL_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"

namespace DerivationFramework {

  class TriggerSkimmingTool : public extends<AthAlgTool, ISkimmingTool> {
    public: 
      TriggerSkimmingTool(const std::string& t, const std::string& n, const IInterface* p);

      bool eventPassesFilter() const override;
      StatusCode initialize() override;

    private:
      Gaudi::Property<std::vector<std::string>> m_triggerListAND{this,"TriggerListAND", {}};
      Gaudi::Property<std::vector<std::string>> m_triggerListOR{this,"TriggerListOR", {}};
      Gaudi::Property<std::vector<std::string>> m_triggerListORHLTOnly{this,"TriggerListORHLTOnly", {}, "Decision is based on HLT only (unseeded triggers)"};
      PublicToolHandle<Trig::TrigDecisionTool> m_trigDec{this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool"};

  }; 
}

#endif // DERIVATIONFRAMEWORK_TRIGGERSKIMMINGTOOL_H
