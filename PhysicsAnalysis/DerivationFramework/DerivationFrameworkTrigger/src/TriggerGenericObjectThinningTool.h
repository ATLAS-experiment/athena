/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TriggerGenericObjectThinningTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

//Thin collection based if a trigger has passed or not
//Just save an empty collection 

#ifndef DERIVATIONFRAMEWORK_TRIGGERGENERICOBJECTTHINNINGTOOL_H
#define DERIVATIONFRAMEWORK_TRIGGERGENERICOBJECTTHINNINGTOOL_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"


#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODBase/IParticleContainer.h"
#include "StoreGate/ThinningHandleKey.h"

#include "xAODBase/IParticleContainer.h"

namespace DerivationFramework {

  class TriggerGenericObjectThinningTool : public extends<AthAlgTool, IThinningTool> {
    public: 
      TriggerGenericObjectThinningTool(const std::string& t, const std::string& n, const IInterface* p);
      ~TriggerGenericObjectThinningTool();
      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
      virtual StatusCode doThinning() const override;

    private:
      StringProperty m_streamName
        { this, "StreamName", "", "Name of the stream being thinned" };
      SG::ThinningHandleKey<xAOD::IParticleContainer> m_SGKey
        { this, "ContainerName", "", "" };

    private:
      Gaudi::Property<std::vector<std::string>> m_triggerListAND{this,"TriggerListAND", {}};
      Gaudi::Property<std::vector<std::string>> m_triggerListOR{this,"TriggerListOR", {}};
      Gaudi::Property<std::vector<std::string>> m_triggerListORHLTOnly{this,"TriggerListORHLTOnly", {}, "Decision is based on HLT only (unseeded triggers)"};
      PublicToolHandle<Trig::TrigDecisionTool> m_trigDec{this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool"};

      bool eventPassedFilter() const;
  }; 
}

#endif // DERIVATIONFRAMEWORK_TRIGGERSKIMMINGTOOL_H