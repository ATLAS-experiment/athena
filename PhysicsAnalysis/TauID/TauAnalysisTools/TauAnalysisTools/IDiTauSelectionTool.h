// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_IDITAUONSELECTIONTOOL_H
#define TAUANALYSISTOOLS_IDITAUONSELECTIONTOOL_H

/*
  author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
  contact email: antonio.de.maria@cern.ch
  documentation in: https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/doc/README-DiTauSelectionTool.rst
*/

// Framework include(s):
#include "AsgTools/IAsgTool.h"
#include "PATCore/AcceptInfo.h"
#include "PATCore/AcceptData.h"

// EDM include(s):
#include "xAODTau/DiTauJet.h"

// ROOT include(s):
#include "TFile.h"

namespace TauAnalysisTools
{

/// Interface for tau selector tool
///
class IDiTauSelectionTool : public virtual asg::IAsgTool
{

  /// Declare the interface that the class provides
  ASG_TOOL_INTERFACE( TauAnalysisTools::IDiTauSelectionTool )

public:
  /// Function initialising the tool
  virtual StatusCode initialize() = 0;

  /// Get an object describing the "selection steps" of the tool
  virtual const asg::AcceptInfo& getAcceptInfo() const = 0;

  /// Get the decision using a generic IParticle pointer
  virtual asg::AcceptData accept( const xAOD::IParticle* p ) const = 0;

  /// Get the decision for a specific TauJet object
  virtual asg::AcceptData accept( const xAOD::DiTauJet& tau ) const = 0;

  /// Set output file for histograms
  virtual void setOutFile( TFile* fOutFile ) = 0;

  /// Write control histograms to output file
  virtual void writeControlHistograms() = 0;
}; // class IDiTauSelectionTool

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_IDITAUONSELECTIONTOOL_H
