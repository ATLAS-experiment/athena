/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ElectronPlots.h"
#include "MCTruthClassifier/MCTruthClassifierDefs.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include <iostream>
using namespace std;
using namespace MCTruthPartClassifier;

namespace Egamma{

ElectronPlots::ElectronPlots(PlotBase* pParent, const std::string& sDir,
			     const std::string& sParticleType):PlotBase(pParent, sDir),
							m_oKinAllRecoPlots(this, "All/KinPlots/", "All Reco "+ sParticleType +" Electron"),
							m_oShowerShapesAllRecoPlots(this, "All/ShowerShapesPlots/","All Reco "+ sParticleType +" Electron"  ),
							m_oShowerShapesAll10GeVRecoPlots(this, "All/ShowerShapes10GeVPlots/","All Reco "+ sParticleType +" Electron"  ),
							m_oIsolationAllRecoPlots(this, "All/IsolationPlots/", "All Reco "+ sParticleType +"  Electron" ),
							m_oTrackAllRecoPlots(this, "All/TrackPlots/", "All Reco "+ sParticleType +"  Electron"  ),
							m_oKinIsoRecoPlots(this, "Iso/KinPlots/", "Iso Reco "+ sParticleType +"  Electron"),
							m_oShowerShapesIsoRecoPlots(this, "Iso/ShowerShapesPlots/","Iso Reco "+ sParticleType +"  Electron" ),
							m_oShowerShapesIso10GeVRecoPlots(this, "Iso/ShowerShapes10GeVPlots/","Iso Reco "+ sParticleType +"  Electron" ),
							m_oIsolationIsoRecoPlots(this, "Iso/IsolationPlots/", "Iso Reco "+ sParticleType +"  Electron" ),
							m_oTrackIsoRecoPlots(this, "Iso/TrackPlots/", "Iso Reco "+ sParticleType +"  Electron" ),
							
							m_oKinIsoLHLoosePlots(this, "IsoLHLoose/KinPlots/", "LHLoose "+ sParticleType +"   Electron"),
							m_oShowerShapesIsoLHLoosePlots(this, "IsoLHLoose/ShowerShapesPlots/","LHLoose "+ sParticleType +"  Electron" ),
							m_oIsolationIsoLHLoosePlots(this, "IsoLHLoose/IsolationPlots/", "LHLoose "+ sParticleType +"  Electron" ),
							m_oTrackIsoLHLoosePlots(this, "IsoLHLoose/TrackPlots/", "LHLoose "+ sParticleType +"  Electron" ),
                            m_oKinIsoLHMediumPlots(this, "IsoLHMedium/KinPlots/", "LHMedium "+ sParticleType +"   Electron"),
							m_oShowerShapesIsoLHMediumPlots(this, "IsoLHMedium/ShowerShapesPlots/","LHMedium "+ sParticleType +"  Electron" ),
							m_oIsolationIsoLHMediumPlots(this, "IsoLHMedium/IsolationPlots/", "LHMedium "+ sParticleType +"  Electron" ),
							m_oTrackIsoLHMediumPlots(this, "IsoLHMedium/TrackPlots/", "LHMedium "+ sParticleType +"  Electron" ),
                            m_oKinIsoLHTightPlots(this, "IsoLHTight/KinPlots/", "LHTight "+ sParticleType +"   Electron"),
							m_oShowerShapesIsoLHTightPlots(this, "IsoLHTight/ShowerShapesPlots/","LHTight "+ sParticleType +"  Electron" ),
							m_oIsolationIsoLHTightPlots(this, "IsoLHTight/IsolationPlots/", "LHTight "+ sParticleType +"  Electron" ),
							m_oTrackIsoLHTightPlots(this, "IsoLHTight/TrackPlots/", "LHTight "+ sParticleType +"  Electron" ),
                            
                            m_oKinPromptRecoPlots(this, "Truth_matched/KinPlots/", "Truth matched Reco "+ sParticleType +"   Electron"),
                            m_oShowerShapesPromptRecoPlots(this, "Truth_matched/ShowerShapesPlots/","Truth matched Reco "+ sParticleType +"  Electron" ),
                            m_oIsolationPromptRecoPlots(this, "Truth_matched/IsolationPlots/", "Truth matched Reco "+ sParticleType +"  Electron" ),
                            m_oTrackPromptRecoPlots(this, "Truth_matched/TrackPlots/", "Truth matched Reco "+ sParticleType +"  Electron" ),
                            m_oKinPromptLHLoosePlots(this, "Truth_matched_LHLoose/KinPlots/", "Truth matched LHLoose "+ sParticleType +"   Electron"),
                            m_oShowerShapesPromptLHLoosePlots(this, "Truth_matched_LHLoose/ShowerShapesPlots/","Truth matched LHLoose "+ sParticleType +"  Electron" ),
                            m_oIsolationPromptLHLoosePlots(this, "Truth_matched_LHLoose/IsolationPlots/", "Truth matched LHLoose "+ sParticleType +"  Electron" ),
                            m_oTrackPromptLHLoosePlots(this, "Truth_matched_LHLoose/TrackPlots/", "Truth matched LHLoose "+ sParticleType +"  Electron" ),
                            m_oKinPromptLHMediumPlots(this, "Truth_matched_LHMedium/KinPlots/", "Truth matched LHMedium "+ sParticleType +"   Electron"),
                            m_oShowerShapesPromptLHMediumPlots(this, "Truth_matched_LHMedium/ShowerShapesPlots/","Truth matched LHMedium "+ sParticleType +"  Electron" ),
                            m_oIsolationPromptLHMediumPlots(this, "Truth_matched_LHMedium/IsolationPlots/", "Truth matched LHMedium "+ sParticleType +"  Electron" ),
                            m_oTrackPromptLHMediumPlots(this, "Truth_matched_LHMedium/TrackPlots/", "Truth matched LHMedium "+ sParticleType +"  Electron" ),
                            m_oKinPromptLHTightPlots(this, "Truth_matched_LHTight/KinPlots/", "Truth matched LHTight "+ sParticleType +"   Electron"),
                            m_oShowerShapesPromptLHTightPlots(this, "Truth_matched_LHTight/ShowerShapesPlots/","Truth matched LHTight "+ sParticleType +"  Electron" ),
                            m_oIsolationPromptLHTightPlots(this, "Truth_matched_LHTight/IsolationPlots/", "Truth matched LHTight "+ sParticleType +"  Electron" ),
                            m_oTrackPromptLHTightPlots(this, "Truth_matched_LHTight/TrackPlots/", "Truth matched LHTight "+ sParticleType +"  Electron" ),
                            nParticles(nullptr),
							nParticles_weighted(nullptr),
							nTypeParticles(nullptr),
							m_sParticleType(sParticleType)
{}

void ElectronPlots::initializePlots(){
  nParticles = Book1D("n", "Number of "+ m_sParticleType + "s;#" + m_sParticleType + " electrons;Events", 15, 0, 15.);
  nParticles_weighted = Book1D("n_weighted", "Number of "+ m_sParticleType + "s;#" + m_sParticleType + " electrons;Events", 15, 0, 15.);
 }

  void ElectronPlots::fill(const xAOD::Electron& electron, const xAOD::EventInfo& eventInfo, bool isPrompt) {
    
  m_oKinAllRecoPlots.fill(electron,eventInfo);
  m_oShowerShapesAllRecoPlots.fill(electron,eventInfo);
  if (electron.pt() > 10 * Gaudi::Units::GeV) {
    m_oShowerShapesAll10GeVRecoPlots.fill(electron,eventInfo);
  }
  m_oIsolationAllRecoPlots.fill(electron,eventInfo);
  m_oTrackAllRecoPlots.fill(electron,eventInfo);
 
  if(!isPrompt) return;
      
  m_oKinPromptRecoPlots.fill(electron,eventInfo);
  m_oShowerShapesPromptRecoPlots.fill(electron,eventInfo);
  m_oIsolationPromptRecoPlots.fill(electron,eventInfo);
  m_oTrackPromptRecoPlots.fill(electron,eventInfo);

  m_oKinIsoRecoPlots.fill(electron,eventInfo);
  m_oShowerShapesIsoRecoPlots.fill(electron,eventInfo);
  if (electron.pt() > 10 * Gaudi::Units::GeV) {
    m_oShowerShapesIso10GeVRecoPlots.fill(electron,eventInfo);
  }
  m_oIsolationIsoRecoPlots.fill(electron,eventInfo);
  m_oTrackIsoRecoPlots.fill(electron,eventInfo);

  bool val_LHloose=false;
  electron.passSelection(val_LHloose, "LHLoose");
  if(val_LHloose) {
    m_oKinIsoLHLoosePlots.fill(electron,eventInfo);
    m_oShowerShapesIsoLHLoosePlots.fill(electron,eventInfo);
    m_oIsolationIsoLHLoosePlots.fill(electron,eventInfo);
    m_oTrackIsoLHLoosePlots.fill(electron,eventInfo);
      
    m_oKinPromptLHLoosePlots.fill(electron,eventInfo);
    m_oShowerShapesPromptLHLoosePlots.fill(electron,eventInfo);
    m_oIsolationPromptLHLoosePlots.fill(electron,eventInfo);
    m_oTrackPromptLHLoosePlots.fill(electron,eventInfo);
  }
  
  bool val_LHmed=false;
  electron.passSelection(val_LHmed, "LHMedium");
  if(val_LHmed) {
    m_oKinIsoLHMediumPlots.fill(electron,eventInfo);
    m_oShowerShapesIsoLHMediumPlots.fill(electron,eventInfo);
    m_oIsolationIsoLHMediumPlots.fill(electron,eventInfo);
    m_oTrackIsoLHMediumPlots.fill(electron,eventInfo);
      
    m_oKinPromptLHMediumPlots.fill(electron,eventInfo);
    m_oShowerShapesPromptLHMediumPlots.fill(electron,eventInfo);
    m_oIsolationPromptLHMediumPlots.fill(electron,eventInfo);
    m_oTrackPromptLHMediumPlots.fill(electron,eventInfo);
  }
  
  bool val_LHtight=false;
  electron.passSelection(val_LHtight, "LHTight");
  if(val_LHtight) {
    m_oKinIsoLHTightPlots.fill(electron,eventInfo);
    m_oShowerShapesIsoLHTightPlots.fill(electron,eventInfo);
    m_oIsolationIsoLHTightPlots.fill(electron,eventInfo);
    m_oTrackIsoLHTightPlots.fill(electron,eventInfo);
      
    m_oKinPromptLHTightPlots.fill(electron,eventInfo);
    m_oShowerShapesPromptLHTightPlots.fill(electron,eventInfo);
    m_oIsolationPromptLHTightPlots.fill(electron,eventInfo);
    m_oTrackPromptLHTightPlots.fill(electron,eventInfo);
  }
  }
  
  
}
