/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAPHYSVALMONITORING_LRTELECTRONPLOTS_H
#define EGAMMAPHYSVALMONITORING_LRTELECTRONPLOTS_H

#include "GaudiKernel/ToolHandle.h"

#include "TrkValHistUtils/PlotBase.h"
#include "TrkValHistUtils/ParamPlots.h"
#include "KinematicsPlots.h"
#include "ShowerShapesPlots.h"
#include "IsolationPlots.h"
#include "TrackPlots.h"
#include "TrkValHistUtils/EfficiencyPlots.h"

#include "xAODEgamma/Electron.h"
#include "xAODTruth/TruthParticle.h"

namespace Egamma{

class LRTElectronPlots:public PlotBase {
    public:
      LRTElectronPlots(PlotBase* pParent, const std::string& sDir, const std::string& sParticleType);
      void fill(const xAOD::Electron& electron, const xAOD::EventInfo& eventInfo, bool isPrompt, bool pass_LHVeryLooseNoPix, bool pass_LHLooseNoPix, bool pass_LHMediumNoPix, bool pass_LHTightNoPix);
     // Reco only information

      Egamma::KinematicsPlots     m_oKinAllRecoPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesAllRecoPlots;
      Egamma::IsolationPlots      m_oIsolationAllRecoPlots;
      Egamma::TrackPlots          m_oTrackAllRecoPlots;
    
      Egamma::KinematicsPlots     m_oKinIsoRecoPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesIsoRecoPlots;
      Egamma::IsolationPlots      m_oIsolationIsoRecoPlots;
      Egamma::TrackPlots          m_oTrackIsoRecoPlots;
  
      Egamma::KinematicsPlots     m_oKinIsoLHLoosePlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesIsoLHLoosePlots;
      Egamma::IsolationPlots      m_oIsolationIsoLHLoosePlots;
      Egamma::TrackPlots          m_oTrackIsoLHLoosePlots;
    
      Egamma::KinematicsPlots     m_oKinIsoLHMediumPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesIsoLHMediumPlots;
      Egamma::IsolationPlots      m_oIsolationIsoLHMediumPlots;
      Egamma::TrackPlots          m_oTrackIsoLHMediumPlots;
    
      Egamma::KinematicsPlots     m_oKinIsoLHTightPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesIsoLHTightPlots;
      Egamma::IsolationPlots      m_oIsolationIsoLHTightPlots;
      Egamma::TrackPlots          m_oTrackIsoLHTightPlots;

      Egamma::KinematicsPlots     m_oKinPromptRecoPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesPromptRecoPlots;
      Egamma::IsolationPlots      m_oIsolationPromptRecoPlots;
      Egamma::TrackPlots          m_oTrackPromptRecoPlots;
    
      Egamma::KinematicsPlots     m_oKinPromptLHLoosePlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesPromptLHLoosePlots;
      Egamma::IsolationPlots      m_oIsolationPromptLHLoosePlots;
      Egamma::TrackPlots          m_oTrackPromptLHLoosePlots;
    
      Egamma::KinematicsPlots     m_oKinPromptLHMediumPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesPromptLHMediumPlots;
      Egamma::IsolationPlots      m_oIsolationPromptLHMediumPlots;
      Egamma::TrackPlots          m_oTrackPromptLHMediumPlots;
    
      Egamma::KinematicsPlots     m_oKinPromptLHTightPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesPromptLHTightPlots;
      Egamma::IsolationPlots      m_oIsolationPromptLHTightPlots;
      Egamma::TrackPlots          m_oTrackPromptLHTightPlots;
      
      // working point plots

      Egamma::KinematicsPlots     m_oKinLHVeryLooseNoPixPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesLHVeryLooseNoPixPlots;
      Egamma::IsolationPlots      m_oIsolationLHVeryLooseNoPixPlots;
      Egamma::TrackPlots          m_oTrackLHVeryLooseNoPixPlots;      

      Egamma::KinematicsPlots     m_oKinLHLooseNoPixPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesLHLooseNoPixPlots;
      Egamma::IsolationPlots      m_oIsolationLHLooseNoPixPlots;
      Egamma::TrackPlots          m_oTrackLHLooseNoPixPlots;   

      Egamma::KinematicsPlots     m_oKinLHMediumNoPixPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesLHMediumNoPixPlots;
      Egamma::IsolationPlots      m_oIsolationLHMediumNoPixPlots;
      Egamma::TrackPlots          m_oTrackLHMediumNoPixPlots; 

      Egamma::KinematicsPlots     m_oKinLHTightNoPixPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesLHTightNoPixPlots;
      Egamma::IsolationPlots      m_oIsolationLHTightNoPixPlots;
      Egamma::TrackPlots          m_oTrackLHTightNoPixPlots; 

      TH1* nParticles;
      TH1* nParticles_weighted;
      TH1* nTypeParticles;
      std::string m_sParticleType;

      void Set_d0_nBins(unsigned d0_nBins) 
      {
        m_oTrackAllRecoPlots.Set_d0_nBins(d0_nBins);
        m_oTrackIsoRecoPlots.Set_d0_nBins(d0_nBins);
        m_oTrackIsoLHLoosePlots.Set_d0_nBins(d0_nBins);
        m_oTrackIsoLHMediumPlots.Set_d0_nBins(d0_nBins);
        m_oTrackIsoLHTightPlots.Set_d0_nBins(d0_nBins);
        m_oTrackPromptRecoPlots.Set_d0_nBins(d0_nBins);
        m_oTrackPromptLHLoosePlots.Set_d0_nBins(d0_nBins);
        m_oTrackPromptLHMediumPlots.Set_d0_nBins(d0_nBins);
        m_oTrackPromptLHTightPlots.Set_d0_nBins(d0_nBins);
        m_oTrackLHVeryLooseNoPixPlots.Set_d0_nBins(d0_nBins);
        m_oTrackLHLooseNoPixPlots.Set_d0_nBins(d0_nBins);
        m_oTrackLHMediumNoPixPlots.Set_d0_nBins(d0_nBins);
        m_oTrackLHTightNoPixPlots.Set_d0_nBins(d0_nBins);
      };

      void Set_d0sig_nBins(unsigned d0_nBins) 
      {
        m_oTrackAllRecoPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackIsoRecoPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackIsoLHLoosePlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackIsoLHMediumPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackIsoLHTightPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackPromptRecoPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackPromptLHLoosePlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackPromptLHMediumPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackPromptLHTightPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackLHVeryLooseNoPixPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackLHLooseNoPixPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackLHMediumNoPixPlots.Set_d0sig_nBins(d0_nBins);
        m_oTrackLHTightNoPixPlots.Set_d0sig_nBins(d0_nBins);
      };
      void Set_z0_nBins(unsigned z0_nBins)
      {
        m_oTrackAllRecoPlots.Set_z0_nBins(z0_nBins);
        m_oTrackIsoRecoPlots.Set_z0_nBins(z0_nBins);
        m_oTrackIsoLHLoosePlots.Set_z0_nBins(z0_nBins);
        m_oTrackIsoLHMediumPlots.Set_z0_nBins(z0_nBins);
        m_oTrackIsoLHTightPlots.Set_z0_nBins(z0_nBins);
        m_oTrackPromptRecoPlots.Set_z0_nBins(z0_nBins);
        m_oTrackPromptLHLoosePlots.Set_z0_nBins(z0_nBins);
        m_oTrackPromptLHMediumPlots.Set_z0_nBins(z0_nBins);
        m_oTrackPromptLHTightPlots.Set_z0_nBins(z0_nBins);
        m_oTrackLHVeryLooseNoPixPlots.Set_z0_nBins(z0_nBins);
        m_oTrackLHLooseNoPixPlots.Set_z0_nBins(z0_nBins);
        m_oTrackLHMediumNoPixPlots.Set_z0_nBins(z0_nBins);
        m_oTrackLHTightNoPixPlots.Set_z0_nBins(z0_nBins);
      };
      void Set_d0_Bins(const std::vector<double> &d0Range) 
      {
        m_oTrackAllRecoPlots.Set_d0_Bins(d0Range);
        m_oTrackIsoRecoPlots.Set_d0_Bins(d0Range);
        m_oTrackIsoLHLoosePlots.Set_d0_Bins(d0Range);
        m_oTrackIsoLHMediumPlots.Set_d0_Bins(d0Range);
        m_oTrackIsoLHTightPlots.Set_d0_Bins(d0Range);
        m_oTrackPromptRecoPlots.Set_d0_Bins(d0Range);
        m_oTrackPromptLHLoosePlots.Set_d0_Bins(d0Range);
        m_oTrackPromptLHMediumPlots.Set_d0_Bins(d0Range);
        m_oTrackPromptLHTightPlots.Set_d0_Bins(d0Range);
        m_oTrackLHVeryLooseNoPixPlots.Set_d0_Bins(d0Range);
        m_oTrackLHLooseNoPixPlots.Set_d0_Bins(d0Range);
        m_oTrackLHMediumNoPixPlots.Set_d0_Bins(d0Range);
        m_oTrackLHTightNoPixPlots.Set_d0_Bins(d0Range);
      };

      void Set_d0sig_Bins(const std::vector<double> &d0Range) 
      {
        m_oTrackAllRecoPlots.Set_d0sig_Bins(d0Range);
        m_oTrackIsoRecoPlots.Set_d0sig_Bins(d0Range);
        m_oTrackIsoLHLoosePlots.Set_d0sig_Bins(d0Range);
        m_oTrackIsoLHMediumPlots.Set_d0sig_Bins(d0Range);
        m_oTrackIsoLHTightPlots.Set_d0sig_Bins(d0Range);
        m_oTrackPromptRecoPlots.Set_d0sig_Bins(d0Range);
        m_oTrackPromptLHLoosePlots.Set_d0sig_Bins(d0Range);
        m_oTrackPromptLHMediumPlots.Set_d0sig_Bins(d0Range);
        m_oTrackPromptLHTightPlots.Set_d0sig_Bins(d0Range);
        m_oTrackLHVeryLooseNoPixPlots.Set_d0sig_Bins(d0Range);
        m_oTrackLHLooseNoPixPlots.Set_d0sig_Bins(d0Range);
        m_oTrackLHMediumNoPixPlots.Set_d0sig_Bins(d0Range);
        m_oTrackLHTightNoPixPlots.Set_d0sig_Bins(d0Range);
      };
      void Set_z0_Bins(const std::vector<double> &z0Range) 
      {
        m_oTrackAllRecoPlots.Set_z0_Bins(z0Range);
        m_oTrackIsoRecoPlots.Set_z0_Bins(z0Range);
        m_oTrackIsoLHLoosePlots.Set_z0_Bins(z0Range);
        m_oTrackIsoLHMediumPlots.Set_z0_Bins(z0Range);
        m_oTrackIsoLHTightPlots.Set_z0_Bins(z0Range);
        m_oTrackPromptRecoPlots.Set_z0_Bins(z0Range);
        m_oTrackPromptLHLoosePlots.Set_z0_Bins(z0Range);
        m_oTrackPromptLHMediumPlots.Set_z0_Bins(z0Range);
        m_oTrackPromptLHTightPlots.Set_z0_Bins(z0Range);
        m_oTrackLHVeryLooseNoPixPlots.Set_z0_Bins(z0Range);
        m_oTrackLHLooseNoPixPlots.Set_z0_Bins(z0Range);
        m_oTrackLHMediumNoPixPlots.Set_z0_Bins(z0Range);
        m_oTrackLHTightNoPixPlots.Set_z0_Bins(z0Range);
      };


    private:
      virtual void initializePlots();
      bool Match(const xAOD::Egamma& particle);

};

}

#endif
