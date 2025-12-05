/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "AssociationUtils/BaseOverlapTool.h"

namespace
{
  /// Unit conversion constant
  const float invGeV = 1e-3;
}

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  BaseOverlapTool::BaseOverlapTool(const std::string& name)
    : asg::AsgTool(name)
  {
    declareProperty("InputLabel", m_inputLabel = "selected",
                    "Decoration which specifies input objects");
    declareProperty("OutputLabel", m_outputLabel = "overlaps",
                    "Decoration given to objects that fail OR");
    declareProperty("OutputPassValue", m_outputPassValue = false,
                    "Set the result assigned to objects that pass");
    declareProperty("LinkOverlapObjects", m_linkOverlapObjects = false,
                    "Turn on overlap object link decorations");
    declareProperty("EnableUserPriority", m_enableUserPrio = false,
                    "Turn on user priority score");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode BaseOverlapTool::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name());
    ATH_MSG_DEBUG("Base config options: InputLabel " << m_inputLabel <<
                  " OutputLabel " << m_outputLabel <<
                  " OutputPassValue " << m_outputPassValue <<
                  " UserPrio " << m_enableUserPrio);

    // Initialize the decoration helper
    m_decHelper1 = std::make_unique<OverlapDecorationHelper<columnar::ContainerId::particle1>>
      (m_inputLabel, m_outputLabel, m_outputPassValue);
    addSubtool(*m_decHelper1);
    m_decHelper2 = std::make_unique<OverlapDecorationHelper<columnar::ContainerId::particle2>>
      (m_inputLabel, m_outputLabel, m_outputPassValue);
    addSubtool(*m_decHelper2);

    // Initialize the obj-link helper
    if(m_linkOverlapObjects)
    {
      m_objLinkHelper1 = std::make_unique<OverlapLinkHelper<columnar::ContainerId::particle1>>("overlapObject");
      addSubtool(*m_objLinkHelper1);
      m_objLinkHelper2 = std::make_unique<OverlapLinkHelper<columnar::ContainerId::particle2>>("overlapObject");
      addSubtool(*m_objLinkHelper2);
    }

    // Initialize the derived tool
    ATH_CHECK( initializeDerived() );

    ATH_CHECK ( initializeColumns() );

    return StatusCode::SUCCESS;
  }

  void BaseOverlapTool::callEvents (columnar::EventContextRange events) const
  {
    auto& baseAcc = *m_baseAccessors;
    for (auto event : events)
    {
      ANA_CHECK_THROW (findOverlaps (baseAcc.m_particles1Acc (event), baseAcc.m_particles2Acc (event), event));
    }
  }
} // namespace ORUtils
