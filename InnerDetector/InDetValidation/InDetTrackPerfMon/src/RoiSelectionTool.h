/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_ROISELECTIONTOOL_H
#define INDETTRACKPERFMON_ROISELECTIONTOOL_H

/// Athena includes
#include "AsgTools/AsgTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

/// local includes
//#include "TrackRoiSelectionTool.h" // TODO - to be included in later MRs

/// STL includes
#include <string>
#include <vector>

/**
 * @file RoiSelectionTool.h
 * header file for class of same name
 * @author marco aparo
 * @date 16 February 2023
**/


typedef TrigCompositeUtils::LinkInfo< TrigRoiDescriptorCollection > roiCollection_t;

namespace IDTPM {

  class RoiSelectionTool :
      public virtual asg::IAsgTool, 
      public asg::AsgTool {

    ASG_TOOL_CLASS( RoiSelectionTool, IAsgTool );

  public:

    /// Constructor
    RoiSelectionTool( const std::string& name );

    /// Destructor
    virtual ~RoiSelectionTool() = default;

    /// RoiSelectionTool methods
    virtual StatusCode initialize() override;

    /// Main method to get selected RoIs
    std::vector< roiCollection_t > getRois(
        const std::string& chainName ) const;

  private:

    /// Retrieve RoIs
    std::vector< roiCollection_t > retrieveRois(
        const std::string& chainName,
        const std::string& roiKey,
        const int& chainLeg=-1 ) const;

    /// get selected RoIs (non-Tag&Probe selection)
    std::vector< roiCollection_t > getRoisStandard(
        const std::string& chainName ) const;

    /// get selected RoIs (Tag&Probe selection)
    std::vector< roiCollection_t > getRoisTnP(
        const std::string& chainName ) const;

    /// Properties to fine-tune the tool behaviour
    StringProperty m_roiKey{ this, "RoiKey", "", "RoI name to process" };

    IntegerProperty m_chainLeg{
        this, "ChainLeg", -1, "Restrict to a specific \"leg\" of a multi-object trigger chain (default = all)" };

    BooleanProperty m_doTnP{ this, "doTagNProbe", false, "Do Tag&Probe RoI selection" };

    StringProperty m_roiKeyTag{ this, "RoiKeyTag", "", "RoI name for the tag" };

    IntegerProperty m_chainLegTag{
        this, "ChainLegTag", 0, "Tag \"leg\" of a multi-object trigger chain (default = 0)" };

    StringProperty m_roiKeyProbe{ this, "RoiKeyProbe", "", "RoI name for the probe" };

    IntegerProperty m_chainLegProbe{
        this, "ChainLegProbe", 1, "Probe \"leg\" of a multi-object trigger chain (default = 1)" };

    PublicToolHandle<Trig::TrigDecisionTool> m_trigDecTool{
      this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool", "" };

  }; // class RoiSelectionTool

} // namespace IDTPM


#endif // > !INDETTRACKPERFMON_ROISELECTIONTOOL_H
