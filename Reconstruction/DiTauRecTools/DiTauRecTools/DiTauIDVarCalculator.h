// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  DITAURECTOOLS_DITAUIDVARCALCULATOR_H
#define  DITAURECTOOLS_DITAUIDVARCALCULATOR_H

/**
 * @brief Tool to calculate and decorate ID variables for boosted di-tau candidates.
 * 
 * @author David Kirchmeier (david.kirchmeier@cern.ch)
 *                                                                              
 */


// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/AnaToolHandle.h"

// Core include(s):
// #include "TruthUtils/PIDHelpers.h"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequenceArea.hh"

// EDM include(s):
#include "xAODTau/TauxAODHelpers.h"
#include "xAODTau/DiTauJetContainer.h"

//#include "tauRecTools/TauEventData.h"

// Selection Tools include:

// Local include(s):
#include "DiTauRecTools/IDiTauToolBase.h"

namespace DiTauRecTools{

class DiTauIDVarCalculator
  : public DiTauRecTools::IDiTauToolBase
  , public asg::AsgTool
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS( DiTauIDVarCalculator,
                  DiTauRecTools::IDiTauToolBase )

public:

  DiTauIDVarCalculator( const std::string& name );

  virtual ~DiTauIDVarCalculator();

  // initialize the tool
  virtual StatusCode initialize() override;

  // calculate ID variables depricated
  virtual StatusCode calculateIDVariables(const xAOD::DiTauJet& xDiTau);

  // calculate ID variables
  virtual StatusCode execute(const xAOD::DiTauJet& xDiTau) override;
  
private:

  static float n_subjets(const xAOD::DiTauJet& xDiTau) ;
  float ditau_pt(const xAOD::DiTauJet& xDiTau) const;
  float f_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float f_subjet(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float f_subjets(const xAOD::DiTauJet& xDiTau) const;
  float f_track(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float R_max(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  static int n_track(const xAOD::DiTauJet& xDiTau) ;
  int n_tracks(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  static int n_isotrack(const xAOD::DiTauJet& xDiTau) ;
  int n_othertrack(const xAOD::DiTauJet& xDiTau) const;
  float R_track(const xAOD::DiTauJet& xDiTau) const;
  float R_track_all(const xAOD::DiTauJet& xDiTau) const;
  float R_track_core(const xAOD::DiTauJet& xDiTau) const;
  float R_isotrack(const xAOD::DiTauJet& xDiTau) const;
  float R_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float R_tracks(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float mass_track(const xAOD::DiTauJet& xDiTau) const;
  float mass_track_core(const xAOD::DiTauJet& xDiTau) const;
  float mass_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float mass_track_all(const xAOD::DiTauJet& xDiTau) const;
  float mass_tracks(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float E_frac(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float R_subjets(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float d0_leadtrack(const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float f_isotracks(const xAOD::DiTauJet& xDiTau) const;

  // steering variables
  // float m_dMaxDeltaR;
  float m_dDefault;
  
  static StatusCode decorNtracks (const xAOD::DiTauJet& xDiTau);
  //TauEventData m_data;
}; // class DiTauIDVarCalculator

}

#endif // TAURECTOOLS_DITAUIDVARCALCULATOR_H
