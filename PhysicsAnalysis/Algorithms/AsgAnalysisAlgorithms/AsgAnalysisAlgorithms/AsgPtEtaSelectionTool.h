/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_PT_ETA_SELECTION_TOOL_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_PT_ETA_SELECTION_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AthContainers/AuxElement.h>
#include <PATCore/IAsgSelectionTool.h>
#include "AsgTools/PropertyWrapper.h"
#include <atomic>

namespace CP
{
  /// \brief an \ref IAsgSelectionTool that performs basic pt and eta
  /// cut (with an optional eta gap)
  ///
  /// This is a very basic selection that needs to happen on all
  /// object types to some degree.  Instead of doing this separately
  /// for each type, it is just one basic tool for all \ref IParticle
  /// implementations.  Also, this is a tool, not an algorithm,
  /// because we already have an algorithm wrapping generic selection
  /// tools and there is no benefit to making it an algorithm.
  ///
  /// There may be some overlap with the individual selectors for the
  /// given object types, but having a tool for this allows to apply
  /// it at any point in the algorithm sequence.

  class AsgPtEtaSelectionTool final
    : public asg::AsgTool, virtual public IAsgSelectionTool
  {
    //
    // public interface
    //

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( AsgPtEtaSelectionTool, IAsgSelectionTool )


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



    //
    // private interface
    //

    /// tool properties
    /// \{

  private:
    Gaudi::Property<float> m_minPt {this, "minPt", 0, "minimum pt to require (or 0 for no pt cut)"};
    Gaudi::Property<float> m_maxPt {this, "maxPt", 0, "maximum pt to require (or 0 for no pt cut)"};
    Gaudi::Property<float> m_maxEta {this, "maxEta", 0, "maximum abs(eta) to allow (or 0 for no eta cut)"};
    Gaudi::Property<float> m_etaGapLow {this, "etaGapLow", 0, "low end of the eta gap"};
    Gaudi::Property<float> m_etaGapHigh {this, "etaGapHigh", 0, "high end of the eta gap (or 0 for no eta gap)"};
    Gaudi::Property<bool> m_useClusterEta {this, "useClusterEta", false, "whether to use the cluster eta (for electrons only)"};
    Gaudi::Property<bool> m_useDressedProperties {this, "useDressedProperties", false, "whether to use the dressed kinematic properties (for truth particles only)"};
    Gaudi::Property<bool> m_printCastWarning {this, "printCastWarning", true, "whether to print a warning/error when the cast fails"};
    Gaudi::Property<bool> m_printClusterWarning {this, "printClusterWarning", true, "whether to print a warning/error when the cluster is missing"};

    /// \}

    /// Index for the minimum pT selection
    int m_minPtCutIndex{ -1 };
    /// Index for the maximum pT selection
    int m_maxPtCutIndex{ -1 };
    /// Index for the maximum eta selection
    int m_maxEtaCutIndex{ -1 };
    /// Index for the eta gap selection
    int m_etaGapCutIndex{ -1 };
    /// Index for the e/gamma casting
    int m_egammaCastCutIndex{ -1 };
    /// Index for the e/gamma calo-cluster
    int m_egammaClusterCutIndex{ -1 };
    /// Index for the existence of dressed properties
    int m_dressedPropertiesIndex{ -1 };

    /// \brief a version of \ref m_printCastWarning that we modify
    /// once we printed the warning
    ///
    /// I don't like modifying property values in the tool itself, so
    /// I copy it over here and then modify once I print out.
    ///
    /// Technically this tool isn't thread-safe due to the use of
    /// TAccept, but once we move to master this will be fixed, so
    /// this member is already made thread-safe so that we don't trip
    /// up on that later.
  private:
    mutable std::atomic<bool> m_shouldPrintCastWarning {true};

    /// \brief a version of \ref m_printClusterWarning that we modify
    /// once we printed the warning
    ///
    /// I don't like modifying property values in the tool itself, so
    /// I copy it over here and then modify once I print out.
    ///
    /// Technically this tool isn't thread-safe due to the use of
    /// TAccept, but once we move to master this will be fixed, so
    /// this member is already made thread-safe so that we don't trip
    /// up on that later.
  private:
    mutable std::atomic<bool> m_shouldPrintClusterWarning {true};

    /// \brief the \ref asg::AcceptInfo we are using
  private:
    asg::AcceptInfo m_accept;

    /// \brief dressed pt and eta accessors
  private:
    std::unique_ptr<SG::AuxElement::ConstAccessor<float>> m_dressedPtAccessor{};
    std::unique_ptr<SG::AuxElement::ConstAccessor<float>> m_dressedEtaAccessor{};
  };
}

#endif
