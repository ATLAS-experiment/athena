/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_MASS_SELECTION_TOOL_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_MASS_SELECTION_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AthContainers/AuxElement.h>
#include <PATCore/IAsgSelectionTool.h>
#include "AsgTools/PropertyWrapper.h"
#include <atomic>

namespace CP
{
  /// \brief an \ref IAsgSelectionTool that performs basic mass
  /// cut
  ///
  /// Mostly intended to be used for large-R jet selections but defined
  /// generically for any particle type here

  class AsgMassSelectionTool final
    : public asg::AsgTool, virtual public IAsgSelectionTool
  {
    //
    // public interface
    //

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( AsgMassSelectionTool, IAsgSelectionTool )


    /// \brief standard constructor
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    using asg::AsgTool::AsgTool;


    //
    // inherited interface
    //

    virtual StatusCode initialize () override;

    virtual const asg::AcceptInfo& getAcceptInfo( ) const override;

    virtual asg::AcceptData accept( const xAOD::IParticle *particle ) const override;

  private:
    Gaudi::Property<float> m_minM {this, "minM", 0, "minimum mass to require (or 0 for no mass cut)"};
    Gaudi::Property<float> m_maxM {this, "maxM", 0, "maximum mass to require (or 0 for no mass cut)"};

    /// Index for the minimum pT selection
    int m_minMassCutIndex{ -1 };
    /// Index for the maximum pT selection
    int m_maxMassCutIndex{ -1 };

    /// \brief the \ref asg::AcceptInfo we are using
  private:
    asg::AcceptInfo m_accept;

  };
}

#endif

