/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SkimmingToolHIGG1.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_SKIMMINGTOOLHSG1_H
#define DERIVATIONFRAMEWORK_SKIMMINGTOOLHSG1_H

#include <array>
#include <optional>
#include <string>
#include <vector>
#include <algorithm>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"

// DerivationFramework includes
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"

// xAOD header files
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEventInfo/EventInfo.h"


#include "TrigDecisionTool/TrigDecisionTool.h"

class IAsgElectronIsEMSelector;

namespace DerivationFramework {

  /** @class SkimmingToolHIGG1
      @author jsaxon@cern.ch 
      @author magdac@cern.ch
     */
 
  
 
 
  class SkimmingToolHIGG1 : public extends<AthAlgTool, ISkimmingTool> {
   


    public: 
      /** Constructor with parameters */
      SkimmingToolHIGG1( const std::string& t, const std::string& n, const IInterface* p );

      /** Destructor */
      ~SkimmingToolHIGG1();

      // Athena algtool's Hooks
      virtual StatusCode  initialize() override;
      virtual StatusCode  finalize() override;

      /** Check that the current event passes this filter */
      virtual bool eventPassesFilter() const override;

    private:

      ///////////////
      ///// TOOLS 

      ToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool;
      


      // CUTS TO APPLY OR NOT
      bool m_reqGRL;
      bool m_reqLArError;
      bool m_reqTrigger;
      bool m_reqPreselection;
      bool m_incMergedElectron;
      bool m_incSingleElectron;
      bool m_incDoubleElectron;
      bool m_incSingleMuon;
      bool m_incDoubleMuon;          
      bool m_incDoubleElectronPhoton;
      bool m_incMergedElectronPhoton;
      bool m_incHighPtElectronPhoton;
      bool m_incTwoPhotons;
      bool m_reqKinematic;
      bool m_reqQuality;
      bool m_reqIsolation;
      bool m_reqInvariantMass;


      // CUT VALUES/SETTINGS

      std::string m_goodRunList;

      std::string m_defaultTrigger;
      std::vector<std::string> m_triggers;
      std::vector<std::string> m_mergedtriggers;

      double m_minPhotonPt;
      bool   m_removeCrack;
      double m_maxEta;

      bool   m_relativePtCuts;
      double m_leadingPhotonPt;
      double m_subleadingPhotonPt;

      double m_minInvariantMass;
      double m_maxInvariantMass;

      double m_minElectronPt;
      double m_minMergedElectronPt;
      double m_minMuonPt;
      double m_maxMuonEta;
     

      ////////////////
      ///// FUNCTIONS

      // Cuts
      bool   SubcutOneMergedElectron() const;
      bool   SubcutGoodRunList() const;
      bool   SubcutLArError(const xAOD::EventInfo& eventInfo) const;
      bool   SubcutTrigger() const;
      /// Leading and sub-leading photon (in that order)
      using LeadingPhotons_t = std::array<const xAOD::Photon*, 2>;
      std::optional<LeadingPhotons_t> SubcutPreselect() const;
      bool   SubcutOnePhotonOneElectron() const;
      bool   SubcutTwoElectrons() const;
      bool   SubcutOnePhotonOneMuon() const;
      bool   SubcutOnePhotonTwoElectrons() const;
      bool   SubcutOnePhotonTwoMuons() const;
      bool   SubcutOnePhotonMergedElectrons(const xAOD::EventInfo& eventInfo) const;
      bool   SubcutHighPtOnePhotonOneElectron() const;

      bool   SubcutKinematic(const LeadingPhotons_t& leadingPhotons, double invariantMass) const;
      bool   SubcutQuality(const LeadingPhotons_t& leadingPhotons) const;
      bool   SubcutIsolation() const;
      bool   SubcutInvariantMass(double invariantMass) const;

      // Calculators
      bool   PhotonPreselect(const xAOD::Photon *ph) const;
      bool   ElectronPreselect(const xAOD::Electron *el) const;
      bool   MergedElectronPreselect(const xAOD::Electron *el) const;
      bool   MuonPreselect(const xAOD::Muon *mu) const;
      double CalculateInvariantMass(const LeadingPhotons_t& leadingPhotons) const;
      double GetDiphotonVertex() const;
      static double CorrectedEnergy(const xAOD::Photon *ph) ;
      double CorrectedEta(const xAOD::Photon *ph) const;
      static double ReturnRZ_1stSampling_cscopt2(double eta1) ;
    

      ///////////////
      ///// COUNTERS

      mutable std::atomic<unsigned int> m_n_tot{0};
      mutable std::atomic<unsigned int> m_n_passGRL{0};
      mutable std::atomic<unsigned int> m_n_passLArError{0};
      mutable std::atomic<unsigned int> m_n_passTrigger{0};
      mutable std::atomic<unsigned int> m_n_passPreselect{0};
      mutable std::atomic<unsigned int> m_n_passSingleElectronPreselect{0};
      mutable std::atomic<unsigned int> m_n_passDoubleElectronPreselect{0};
      mutable std::atomic<unsigned int> m_n_passSingleMuonPreselect{0};
      mutable std::atomic<unsigned int> m_n_passSinglePhotonDoubleMuonPreselect{0};
      mutable std::atomic<unsigned int> m_n_passSinglePhotonDoubleElectronPreselect{0};
      mutable std::atomic<unsigned int> m_n_passSinglePhotonMergedElectronPreselect{0};
      mutable std::atomic<unsigned int> m_n_passHighPtPhotonMergedElectronPreselect{0};
      mutable std::atomic<unsigned int> m_n_passSingleMergedElectronPreselect{0};
      mutable std::atomic<unsigned int> m_n_passKinematic{0};
      mutable std::atomic<unsigned int> m_n_passQuality{0};
      mutable std::atomic<unsigned int> m_n_passIsolation{0};
      mutable std::atomic<unsigned int> m_n_passInvariantMass{0};
      mutable std::atomic<unsigned int> m_n_pass{0};


      /////////////////////////////
      ///// FUNCTIONS

      static const double s_MZ;

      ////////////////////////////
      ///// TOOLS

      ToolHandle<IAsgElectronIsEMSelector> m_mergedCutTools;

      SG::ReadHandleKey<xAOD::EventInfo> 
        m_eventInfoKey { this, "EventInfoKey", "EventInfo", "" };

      SG::ReadHandleKey<xAOD::PhotonContainer >
        m_photonKey { this, "PhotonKey", "Photons", "" };

      SG::ReadHandleKey<xAOD::ElectronContainer >
        m_electronKey { this, "ElectronKey", "Electrons", "" };

      SG::ReadHandleKey<xAOD::MuonContainer >
        m_muonKey { this, "MuonKey", "Muons", "" };

  }; 

}

#endif // DERIVATIONFRAMEWORK_SKIMMINGTOOLEXAMPLE_H
