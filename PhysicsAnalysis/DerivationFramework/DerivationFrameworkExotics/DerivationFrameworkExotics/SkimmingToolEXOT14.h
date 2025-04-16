/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SkimmingToolEXOT14.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_SKIMMINGTOOLEXOT14_H
#define DERIVATIONFRAMEWORK_SKIMMINGTOOLEXOT14_H 1

#include <array>
#include <optional>
#include <string>
#include <vector>
#include <algorithm>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"

// DerivationFramework includes
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"

// #include "xAODRootAccess/TStore.h"

// xAOD header files
#include "xAODJet/JetContainer.h"
// #include "xAODEgamma/ElectronContainer.h"
// #include "xAODEgamma/PhotonContainer.h"
// #include "xAODMuon/MuonContainer.h"

#include "TrigDecisionTool/TrigDecisionTool.h"

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "JetCalibTools/IJetCalibrationTool.h"

class JetCalibrationTool;

namespace DerivationFramework {

  /** @class SkimmingToolEXOT14
      @author jsaxon@cern.ch (adapted from Susumu Oda)
     */
  class SkimmingToolEXOT14 : public extends<AthAlgTool, ISkimmingTool> {

    public: 
      /** Constructor with parameters */
      SkimmingToolEXOT14( const std::string& t, const std::string& n, const IInterface* p );

      /** Destructor */
      ~SkimmingToolEXOT14();

      // Athena algtool's Hooks
      virtual StatusCode  initialize() override;
      virtual StatusCode  finalize() override;

      /** Check that the current event passes this filter */
      virtual bool eventPassesFilter() const override;

    private:

      ///////////////
      ///// TOOLS 

      ToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool;
      ToolHandle<IJetCalibrationTool>    m_JESTool;

      ///////////////
      ///// SETTINGS

      std::string m_jetSGKey;

      // CUTS TO APPLY OR NOT
      bool m_reqGRL;
      bool m_reqLArError;
      bool m_reqTrigger;
      bool m_reqPreselection;
      bool m_reqJetPts;
      bool m_reqJetsDEta;
      bool m_reqDiJetMass;
      bool m_reqJetsDPhi;

      // CUT VALUES/SETTINGS

      std::string m_goodRunList;

      std::string m_defaultTrigger;
      std::vector<std::string> m_triggers;

      double m_minJetPt;
      double m_maxEta;
      double m_leadingJetPt   ;
      double m_subleadingJetPt;
      double m_etaSeparation;
      double m_dijetMass;
      double m_jetDPhi;

      ////////////////
      ///// FUNCTIONS

      // Cuts
      using LeadingJets_t = std::array<TLorentzVector, 2>;
      std::optional<LeadingJets_t> SubcutPreselect() const;
      bool   SubcutGoodRunList() const;
      bool   SubcutLArError() const;
      bool   SubcutTrigger() const;
      bool   SubcutJetPts(const LeadingJets_t& jets) const;
      bool   SubcutJetDEta(const LeadingJets_t& jets) const;
      bool   SubcutDijetMass(const LeadingJets_t& jets) const;
      bool   SubcutJetDPhi(const LeadingJets_t& jets) const;

      // Helpers
      std::string TriggerVarName(std::string s) const;


      ///////////////
      ///// COUNTERS
      mutable std::atomic<unsigned int> m_n_tot;
      mutable std::atomic<unsigned int> m_n_passGRL;
      mutable std::atomic<unsigned int> m_n_passLArError;
      mutable std::atomic<unsigned int> m_n_passTrigger;
      mutable std::atomic<unsigned int> m_n_passPreselect;
      mutable std::atomic<unsigned int> m_n_passJetPts;
      mutable std::atomic<unsigned int> m_n_passJetsDEta;
      mutable std::atomic<unsigned int> m_n_passDiJetMass;
      mutable std::atomic<unsigned int> m_n_passJetsDPhi;
      mutable std::atomic<unsigned int> m_n_pass;

      static const double s_MZ;

  }; 

}

#endif // DERIVATIONFRAMEWORK_SKIMMINGTOOLEXAMPLE_H
