/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GeneratorPhysVal_GeneratorPhysValMonitoring_H
#define GeneratorPhysVal_GeneratorPhysValMonitoring_H

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ServiceHandle.h"

// Local includes
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODTruth/TruthVertex.h"
#include <TLorentzVector.h>
#include "GeneratorPlots.h"
#include "GeneratorProductionVertexPlots.h"
#include "GeneratorSelector.h"
#include "GeneratorEventInfo.h"
#include "AsgTools/AnaToolHandle.h"
#include <PathResolver/PathResolver.h>
#include "StoreGate/ReadHandleKey.h"
#include "AtlasHepMC/GenEvent.h"
// Root includes
#include "TH1.h"





namespace GeneratorPhysVal
{
  class GeneratorPhysValMonitoringTool
      : public ManagedMonitorToolBase
  {

  public:

    GeneratorPhysValMonitoringTool(const std::string &type,
                                   const std::string &name,
                                   const IInterface *parent);

    virtual ~GeneratorPhysValMonitoringTool();

    virtual StatusCode initialize();
    virtual StatusCode bookHistograms();
    virtual StatusCode fillHistograms(const EventContext& ctx);
    virtual StatusCode procHistograms();

  private:
    GeneratorPhysValMonitoringTool();
    GeneratorPhysVal::GeneratorPlots m_testPlots;
    GeneratorPhysVal::GeneratorPlots m_ChargedParticlePlots;
    GeneratorPhysVal::GeneratorPlots m_GeneratorLevelPlots;
    GeneratorPhysVal::GeneratorPlots m_SimulationLevelPlots;
    GeneratorPhysVal::GeneratorProductionVertexPlots m_ProductionVertexPlots;
    GeneratorPhysVal::GeneratorEventInfo m_EventInfoPlots;

    StatusCode book(PlotBase& plots);
    TH1D* m_number = nullptr;
    TH1D* m_number_GeneratorLevel = nullptr;
    TH1D* m_number_SimulationLevel = nullptr;

    GeneratorSelector* m_GeneratorSelector;

    
    int m_ref_mcChannelNumber = 0;
    
    Gaudi::Property<std::vector<float>> m_binning_N_TruthParticle{this, "binning_N_TruthParticle", {600,0.,6000},"binning_N_TruthParticle"};
    Gaudi::Property<std::vector<float>> m_binning_N_GeneratorLevelParticle{this, "binning_N_GeneratorLevelParticle", {10,-5.,5},"binning_N_GeneratorLevelParticle"};
    Gaudi::Property<std::vector<float>> m_binning_N_SimulationLevelParticle{this, "binning_N_SimulationLevelParticle", {2000,0,2000},"binning_N_SimulationLevelParticle"};



    SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey{this
      , "EventInfo"
      , "EventInfo"
      , "ReadHandleKey for xAOD::EventInfo" };

  };
}

#endif
