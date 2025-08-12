// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include "AsgTools/PropertyWrapper.h"

// EDM include(s):
#include "xAODTau/TauxAODHelpers.h"

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
  virtual StatusCode calculateIDVariables(const xAOD::DiTauJet& xDiTau) const;

  // calculate ID variables
  virtual StatusCode execute(const xAOD::DiTauJet& xDiTau) const override;
  
private:

  Gaudi::Property<float> m_dDefault{this, "DefaultValue", -1234};

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
  
  static StatusCode decorNtracks (const xAOD::DiTauJet& xDiTau);
}; // class DiTauIDVarCalculator

}

#endif // TAURECTOOLS_DITAUIDVARCALCULATOR_H


