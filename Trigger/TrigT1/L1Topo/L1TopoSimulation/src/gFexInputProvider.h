/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TOPOSIMULATION_GFEXINPUTPROVIDER_H
#define L1TOPOSIMULATION_GFEXINPUTPROVIDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L1TopoSimulation/IInputTOBConverter.h"
#include "GaudiKernel/LockedHandle.h"

// gFEX EDMs
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoIContainer.h"

namespace LVL1 {

   class gFexInputProvider : public extends<AthAlgTool, IInputTOBConverter> {
   public:
      gFexInputProvider(const std::string& type, const std::string& name, const IInterface* parent);
      
      virtual ~gFexInputProvider() = default;

      virtual StatusCode initialize() override final;
      virtual StatusCode fillTopoInputEvent(TCS::TopoInputEvent& ) const override final;

   private:

      StatusCode fillSRJet(TCS::TopoInputEvent& inputEvent) const;
      StatusCode fillLRJet(TCS::TopoInputEvent& inputEvent) const;

      StatusCode fillXEJWOJ(TCS::TopoInputEvent& inputEvent) const;
      StatusCode fillMHT(TCS::TopoInputEvent& inputEvent) const;
      StatusCode fillXENC(TCS::TopoInputEvent& inputEvent) const;
      StatusCode fillXERHO(TCS::TopoInputEvent& inputEvent) const;

      StatusCode fillTE(TCS::TopoInputEvent& inputEvent) const;
      StatusCode fillGESPRESSO(TCS::TopoInputEvent& inputEvent) const;
      
      ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool to create online histograms"};

      SG::ReadHandleKey<xAOD::gFexJetRoIContainer> m_gJet_EDMKey {this, "gFexSRJetRoIKey", "L1_gFexSRJetRoI", "gFEX Jet EDM"};
      SG::ReadHandleKey<xAOD::gFexJetRoIContainer> m_gLJet_EDMKey {this, "gFexLRJetRoIKey", "L1_gFexLRJetRoI", "gFEX LJet EDM"};

      SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer> m_gXEJWOJ_EDMKey {this, "gMETComponentsJwojKey", "L1_gMETComponentsJwoj", "gFEX XEJWOJ EDM"};
      SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer> m_gMHT_EDMKey {this,"gMHTComponentsJwojKey", "L1_gMHTComponentsJwoj", "gFEX MHT EDM"};
      SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer> m_gXENC_EDMKey {this, "gMETComponentsNoiseCutKey", "L1_gMETComponentsNoiseCut", "gFEX XENC EDM"};
      SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer> m_gXERHO_EDMKey {this, "gMETComponentsRmsKey", "L1_gMETComponentsRms", "gFEX RHO ROI EDM"};

      SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer> m_gTE_EDMKey {this, "gScalarEJwojKey", "L1_gScalarEJwoj", "gFEX TE EDM"};
      SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer> m_gESPRESSO_EDMKey {this, "gEspressoKey", "L1_gEspresso", "gFEX gESPRESSO EDM"}; //TODO: update to actual default key once implemented on gFEX side!

      // gFex to L1Topo conversion factors
      static const int m_EtJet_conversion;
      static const double m_EtGlobal_conversion;
      static const int m_phi_conversion;
      static const int m_eta_conversion;
 
      static const double m_EtDoubleJet_conversion;
      static const double m_EtDoubleGlobal_conversion;
      static const double m_phiDouble_conversion;
      static const double m_etaDouble_conversion;

   };
}

#endif
