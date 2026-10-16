/**
 * @file DiTauTruthMatchingTool.h
 * @author David Kirchmeier
 * @author Guillermo Hamity (ghamity@cern.ch)
 * @brief Tau, lepton and jet truth matching for ditau jets
 * 
 * @copyright Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 * 
 */
// Dear emacs, this is -*- c++ -*-


#ifndef  TAUANALYSISTOOLS_DITAUTRUTHMATCHINGTOOL_H
#define  TAUANALYSISTOOLS_DITAUTRUTHMATCHINGTOOL_H


// Local include(s):
#include "TauAnalysisTools/IDiTauTruthMatchingTool.h"
#include "TauAnalysisTools/BuildTruthTaus.h"
#include "xAODBase/IParticle.h"
#include "CxxUtils/CachedValue.h"
#include "AsgTools/PropertyWrapper.h"

namespace ADD_CMODE(TauAnalysisTools)
{

class DiTauTruthMatchingTool
  : public virtual BuildTruthTaus
  , public virtual IDiTauTruthMatchingTool
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS2( DiTauTruthMatchingTool,
                   IBuildTruthTaus,
                   IDiTauTruthMatchingTool )

public:                         // Interface functions

  DiTauTruthMatchingTool( const std::string& name );

  virtual ~DiTauTruthMatchingTool();

  // initialize the tool
  virtual StatusCode initialize();

  /** get pointer to the truth matched particle.
   *  if no truth particle was found a null pointer is returned */
  virtual void getTruth(const xAOD::DiTauJet& xDiTau);

  virtual void getTruth(const std::vector<const xAOD::DiTauJet*>& vDiTaus);

private:                        // private helper functions

  StatusCode findTruthTau(const xAOD::DiTauJet& xDiTau);
  StatusCode checkTruthMatch (const xAOD::DiTauJet& xDiTau, const xAOD::TruthParticleContainer& xTauContainer) const;
  StatusCode truthMatch (const TLorentzVector& vSubjetTLV,
                         const xAOD::TruthParticleContainer& xTruthTauContainer, 
                         const xAOD::TruthParticle* &xTruthMatch,
			 const xAOD::Jet* &xTruthJetMatch,
                         TauAnalysisTools::TruthMatchedParticleType &eTruthMatchedParticleType) const;

private:                        // steering variables

  Gaudi::Property<double> m_dMaxDeltaR{this, "MaxDeltaR", 0.2}; 

  CxxUtils::CachedValue<bool> m_bIsTruthMatchedAvailable;
  CxxUtils::CachedValue<bool> m_bIsTruthParticleLinkAvailable;

private:                        // private helper variables

  SG::ConstAccessor<double> m_accPtVis;
  SG::ConstAccessor<double> m_accEtaVis;
  SG::ConstAccessor<double> m_accPhiVis;
  SG::ConstAccessor<double> m_accMVis;
}; // class DiTauTruthMatchingTool

}
#endif // TAUANALYSISTOOLS_DITAUTRUTHMATCHINGTOOL_H
