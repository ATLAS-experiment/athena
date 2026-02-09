/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUDQA_GENERALTAUPLOTS_H
#define TAUDQA_GENERALTAUPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "TauKinematicPlots.h" //member
#include "xAODTau/TauJet.h" //typedef

class TH1;

namespace Tau{

class GeneralTauPlots: public PlotBase {
   public:
      GeneralTauPlots(PlotBase *pParent, const std::string& sDir, const std::string& sTauJetContainerName);
      virtual ~GeneralTauPlots();
      
      void fill(const xAOD::TauJet& tau, float weight);

      Tau::TauKinematicPlots m_oTauKinematicPlots;
      TH1* m_tauCharge{};
      TH1* m_tauNChargedTracks{};
      TH1* m_tauNIsolatedTracks{};
      TH1* m_tauNCoreTracks{};
      TH1* m_tauNWideTracks{};
      TH1* m_ptHighPt{};

      // RNN
      TH1* m_RNNEleScore{};
      TH1* m_RNNEleScoreSigTrans{};
      TH1* m_GNTauScore{};
      TH1* m_GNTauScoreSigTrans{};
      TH1* m_ptGNTauLoose{};
      TH1* m_ptGNTauMedium{};
      TH1* m_ptGNTauTight{};
      TH1* m_ptGNTauLooseHighPt{};
      TH1* m_ptGNTauMediumHighPt{};
      TH1* m_ptGNTauTightHighPt{};

   private:
      void initializePlots();
      std::string m_sTauJetContainerName;
};

}

#endif
