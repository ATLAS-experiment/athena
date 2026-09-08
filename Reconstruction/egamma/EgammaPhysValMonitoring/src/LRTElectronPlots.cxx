/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LRTElectronPlots.h"
#include "TruthUtils/TruthClasses.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include <iostream>
using namespace std;
using namespace MCTruthPartClassifier;

namespace Egamma{

LRTElectronPlots::LRTElectronPlots(PlotBase* pParent, const std::string& sDir,
    const std::string& sParticleType):PlotBase(pParent, sDir),
    m_oKinAllRecoPlots(this, "All/KinPlots/", "All Reco "+ sParticleType +" Electron"),
    m_oShowerShapesAllRecoPlots(this, "All/ShowerShapesPlots/","All Reco "+ sParticleType +" Electron"  ),
    m_oIsolationAllRecoPlots(this, "All/IsolationPlots/", "All Reco "+ sParticleType +"  Electron" ),
    m_oTrackAllRecoPlots(this, "All/TrackPlots/", "All Reco "+ sParticleType +"  Electron"  ),
    m_oKinIsoRecoPlots(this, "Iso/KinPlots/", "Iso Reco "+ sParticleType +"  Electron"),
    m_oShowerShapesIsoRecoPlots(this, "Iso/ShowerShapesPlots/","Iso Reco "+ sParticleType +"  Electron" ),
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

    m_oKinLHVeryLooseNoPixPlots(this, "LHVeryLooseNoPix/KinPlots/", "LHVeryLooseNoPix "+ sParticleType +"   Electron"),
    m_oShowerShapesLHVeryLooseNoPixPlots(this, "LHVeryLooseNoPix/ShowerShapesPlots/","LHVeryLooseNoPix "+ sParticleType +"  Electron" ),
    m_oIsolationLHVeryLooseNoPixPlots(this, "LHVeryLooseNoPix/IsolationPlots/", "LHVeryLooseNoPix "+ sParticleType +"  Electron" ),
    m_oTrackLHVeryLooseNoPixPlots(this, "LHVeryLooseNoPix/TrackPlots/", "LHVeryLooseNoPix "+ sParticleType +"  Electron" ),   

    m_oKinLHLooseNoPixPlots(this, "LHLooseNoPix/KinPlots/", "LHLooseNoPix "+ sParticleType +"   Electron"),
    m_oShowerShapesLHLooseNoPixPlots(this, "LHLooseNoPix/ShowerShapesPlots/","LHLooseNoPix "+ sParticleType +"  Electron" ),
    m_oIsolationLHLooseNoPixPlots(this, "LHLooseNoPix/IsolationPlots/", "LHLooseNoPix "+ sParticleType +"  Electron" ),
    m_oTrackLHLooseNoPixPlots(this, "LHLooseNoPix/TrackPlots/", "LHLooseNoPix "+ sParticleType +"  Electron" ),   

    m_oKinLHMediumNoPixPlots(this, "LHMediumNoPix/KinPlots/", "LHMediumNoPix "+ sParticleType +"   Electron"),
    m_oShowerShapesLHMediumNoPixPlots(this, "LHMediumNoPix/ShowerShapesPlots/","LHMediumNoPix "+ sParticleType +"  Electron" ),
    m_oIsolationLHMediumNoPixPlots(this, "LHMediumNoPix/IsolationPlots/", "LHMediumNoPix "+ sParticleType +"  Electron" ),
    m_oTrackLHMediumNoPixPlots(this, "LHMediumNoPix/TrackPlots/", "LHMediumNoPix "+ sParticleType +"  Electron" ),   

    m_oKinLHTightNoPixPlots(this, "LHTightNoPix/KinPlots/", "LHTightNoPix "+ sParticleType +"   Electron"),
    m_oShowerShapesLHTightNoPixPlots(this, "LHTightNoPix/ShowerShapesPlots/","LHTightNoPix "+ sParticleType +"  Electron" ),
    m_oIsolationLHTightNoPixPlots(this, "LHTightNoPix/IsolationPlots/", "LHTightNoPix "+ sParticleType +"  Electron" ),
    m_oTrackLHTightNoPixPlots(this, "LHTightNoPix/TrackPlots/", "LHTightNoPix "+ sParticleType +"  Electron" ),   

    nParticles(nullptr),
    nParticles_weighted(nullptr),
    nTypeParticles(nullptr),
    m_sParticleType(sParticleType)
{}

void LRTElectronPlots::initializePlots(){
  nParticles = Book1D("n", "Number of "+ m_sParticleType + "s;#" + m_sParticleType + " electrons;Events", 15, 0, 15.);
  nParticles_weighted = Book1D("n_weighted", "Number of "+ m_sParticleType + "s;#" + m_sParticleType + " electrons;Events", 15, 0, 15.);
 }

void LRTElectronPlots::fill(const xAOD::Electron& electron, const xAOD::EventInfo& eventInfo, bool isPrompt, bool pass_LHVeryLooseNoPix, bool pass_LHLooseNoPix, bool pass_LHMediumNoPix, bool pass_LHTightNoPix) {

  m_oKinAllRecoPlots.fill(electron,eventInfo);
  m_oShowerShapesAllRecoPlots.fill(electron,eventInfo);
  m_oIsolationAllRecoPlots.fill(electron,eventInfo);
  m_oTrackAllRecoPlots.fill(electron,eventInfo);
      
  m_oKinPromptRecoPlots.fill(electron,eventInfo);
  m_oShowerShapesPromptRecoPlots.fill(electron,eventInfo);
  m_oIsolationPromptRecoPlots.fill(electron,eventInfo);
  m_oTrackPromptRecoPlots.fill(electron,eventInfo);

  if (pass_LHVeryLooseNoPix){
    m_oKinLHVeryLooseNoPixPlots.fill(electron,eventInfo);
    m_oShowerShapesLHVeryLooseNoPixPlots.fill(electron,eventInfo);
    m_oIsolationLHVeryLooseNoPixPlots.fill(electron,eventInfo);
    m_oTrackLHVeryLooseNoPixPlots.fill(electron,eventInfo);
  }

  if (pass_LHLooseNoPix){
    m_oKinLHLooseNoPixPlots.fill(electron,eventInfo);
    m_oShowerShapesLHLooseNoPixPlots.fill(electron,eventInfo);
    m_oIsolationLHLooseNoPixPlots.fill(electron,eventInfo);
    m_oTrackLHLooseNoPixPlots.fill(electron,eventInfo);
  }

  if (pass_LHMediumNoPix){
    m_oKinLHMediumNoPixPlots.fill(electron,eventInfo);
    m_oShowerShapesLHMediumNoPixPlots.fill(electron,eventInfo);
    m_oIsolationLHMediumNoPixPlots.fill(electron,eventInfo);
    m_oTrackLHMediumNoPixPlots.fill(electron,eventInfo);
  }

  if (pass_LHTightNoPix){
    m_oKinLHTightNoPixPlots.fill(electron,eventInfo);
    m_oShowerShapesLHTightNoPixPlots.fill(electron,eventInfo);
    m_oIsolationLHTightNoPixPlots.fill(electron,eventInfo);
    m_oTrackLHTightNoPixPlots.fill(electron,eventInfo);
  }

  if(!isPrompt) return;

  m_oKinIsoRecoPlots.fill(electron,eventInfo);
  m_oShowerShapesIsoRecoPlots.fill(electron,eventInfo);
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
