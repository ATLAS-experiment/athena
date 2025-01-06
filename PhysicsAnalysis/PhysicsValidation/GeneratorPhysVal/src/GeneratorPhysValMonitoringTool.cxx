/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorPhysValMonitoringTool.h"
#include <vector>
#include "GaudiKernel/IToolSvc.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthLegacySequence.h"
#include "TString.h"
#include <cmath>
#include <limits>
#include <fstream> 



namespace GeneratorPhysVal
{

  GeneratorPhysValMonitoringTool::GeneratorPhysValMonitoringTool(const std::string &type,
                                                                 const std::string &name,                                             
                                                                 const IInterface *parent) : ManagedMonitorToolBase(type, name, parent),
                                                                                             m_testPlots(0, "test/", "test"),                                     // for testing 
                                                                                             m_ChargedParticlePlots(0,"ChargedParticle/","ChargedParticle"),     // save charged particles
                                                                                             m_GeneratorLevelPlots(0,"GeneratorLevel/","GeneratorLevel"),        // save generator level particle 
                                                                                             m_SimulationLevelPlots(0,"SimulationLevel/","SimulationLevel"),     // save simulation level particle 
                                                                                             m_ProductionVertexPlots(0,"ProductionVertex/","ProductionVertex"),  // save production vertex 
                                                                                             m_EventInfoPlots(0,"EventInfo/","EventInfo"),                       // save EventInfo
                                                                                             m_number(nullptr),
                                                                                             m_number_GeneratorLevel(nullptr),
                                                                                             m_number_SimulationLevel(nullptr),
                                                                                             m_GeneratorSelector(new GeneratorSelector)

  {
    
  }

  GeneratorPhysValMonitoringTool::~GeneratorPhysValMonitoringTool()
  {}

  StatusCode GeneratorPhysValMonitoringTool::initialize()
  {
    ATH_MSG_INFO("Initializing " << name() << "...");
    ATH_CHECK(ManagedMonitorToolBase::initialize());
    ATH_CHECK(m_evtInfoKey.initialize());
    
    return StatusCode::SUCCESS;
  }
StatusCode GeneratorPhysValMonitoringTool::book(PlotBase& plots)
{
  plots.initialize();
  std::vector<HistData> hists = plots.retrieveBookedHistograms();

  for (auto& hist : hists)
  {
    ATH_MSG_INFO("Initializing " << hist.first << " " << hist.first->GetName() << " " << hist.second << "...");
    ATH_CHECK(regHist(hist.first, hist.second, all));
  }
  return StatusCode::SUCCESS;
}
  StatusCode GeneratorPhysValMonitoringTool::bookHistograms()
  {
    ATH_MSG_INFO("Booking hists " << name() << "...");
    std::string dir_test("nplots/");
    ATH_CHECK(regHist(m_number = new TH1D("N_TruthParticle","Number of Truth Particles; n ;Events" ,m_binning_N_TruthParticle[0], m_binning_N_TruthParticle[1], m_binning_N_TruthParticle[2]), dir_test, all));
    ATH_CHECK(regHist(m_number_GeneratorLevel = new TH1D("N_GeneratorLevelParticle","Number of Generator Level Particles; n ;Events" ,m_binning_N_GeneratorLevelParticle[0], m_binning_N_GeneratorLevelParticle[1], m_binning_N_GeneratorLevelParticle[2]), dir_test, all));
    ATH_CHECK(regHist(m_number_SimulationLevel = new TH1D("N_SimulationLevelParticle","Number of Simulation Level Particles; n ;Events" ,m_binning_N_SimulationLevelParticle[0], m_binning_N_SimulationLevelParticle[1], m_binning_N_SimulationLevelParticle[2]), dir_test, all));

    ATH_CHECK(book(m_testPlots));
    ATH_CHECK(book(m_ChargedParticlePlots));
    ATH_CHECK(book(m_GeneratorLevelPlots));
    ATH_CHECK(book(m_SimulationLevelPlots));
    ATH_CHECK(book(m_ProductionVertexPlots));
    ATH_CHECK(book(m_EventInfoPlots));
    return StatusCode::SUCCESS;
  }

  StatusCode GeneratorPhysValMonitoringTool::fillHistograms()
  {

    const EventContext& ctx = Gaudi::Hive::currentContext();    
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_evtInfoKey, ctx);

    m_EventInfoPlots.check_eventNumber(eventInfo);
    m_ref_mcChannelNumber = m_EventInfoPlots.check_mcChannelNumber(eventInfo,m_ref_mcChannelNumber);

    const xAOD::TruthParticleContainer *TruthParticles(nullptr);
    if (!TruthParticles)
    {
      TruthParticles = evtStore()->tryConstRetrieve<xAOD::TruthParticleContainer>("TruthParticles");
    }
    
    const std::vector< TLorentzVector > SelectedGeneratorLevel = m_GeneratorSelector->GetGeneratorLevel(TruthParticles);
    const std::vector< TLorentzVector >  SelectedSimulationLevel = m_GeneratorSelector->GetSimulationLevel(TruthParticles);


    for (const auto particle : *TruthParticles)
    {

      if (particle->charge()){
         m_ChargedParticlePlots.fill(particle);
      }

      m_testPlots.fill(particle);

      const xAOD::TruthVertex* vtx = particle->prodVtx();
      if (vtx) {
      m_ProductionVertexPlots.fillProdVtx(vtx);
    }
    }

    m_number_GeneratorLevel->Fill((double)SelectedGeneratorLevel.size());
    m_number_SimulationLevel->Fill((double)SelectedSimulationLevel.size());
    m_GeneratorLevelPlots.fill(SelectedGeneratorLevel);
    m_SimulationLevelPlots.fill(SelectedSimulationLevel);

    m_number->Fill((double)TruthParticles->size());
    return StatusCode::SUCCESS;
  }

  StatusCode GeneratorPhysValMonitoringTool::procHistograms()
  {
    ATH_MSG_INFO("Finalising hists " << name() << "...");
    m_testPlots.finalize();
    m_ChargedParticlePlots.finalize();
    m_GeneratorLevelPlots.finalize();
    m_SimulationLevelPlots.finalize();
    m_ProductionVertexPlots.finalize();
    m_EventInfoPlots.finalize();
    return StatusCode::SUCCESS;
    
  }
}
