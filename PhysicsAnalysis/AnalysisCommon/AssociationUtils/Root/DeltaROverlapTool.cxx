/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Primary include
#include "AssociationUtils/DeltaROverlapTool.h"

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  DeltaROverlapTool::DeltaROverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("DR", m_dR = 0.4, "Maximum dR for overlap match");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
    declareProperty("SwapContainerPrecedence", m_swapContainerPrecedence = false,
                    "Use second container for flagging overlaps");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode DeltaROverlapTool::initializeDerived()
  {
    ATH_MSG_DEBUG("Setting up dR matching with cone size " << m_dR);

    // Initialize the dR matcher
    m_dRMatcher = std::make_unique<DeltaRMatcher>(m_dR, m_useRapidity);
    if (m_objectType1.value() != 0 && m_objectType2.value() != 0) {
      ATH_CHECK (m_dRMatcher->setObjectTypes (static_cast<xAODType::ObjectType>(m_objectType1.value()),
                                              static_cast<xAODType::ObjectType>(m_objectType2.value())));
    } else if (!columnar::ColumnarModeDefault::isXAOD) {
      ATH_MSG_INFO("Object types not set, please set ObjectType1 and ObjectType2 properties");
    }
    addSubtool(*m_dRMatcher);
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI1,columnar::RegularContainerIdConcept CI2,typename CM>
  StatusCode DeltaROverlapTool::
  internalFindOverlaps(columnar::ObjectRange<CI1,CM> testCont, columnar::ObjectRange<CI2,CM> refCont) const
  {
    // Loop over surviving input objects
    for(const auto testPar : testCont){
      if(isSurvivingObject(testPar)){
        for(const auto refPar : refCont){
          if(isSurvivingObject(refPar)){
            // Check for duplicates and overlap
            if constexpr (CM::isXAOD) {
              if(&testPar.getXAODObject() == &refPar.getXAODObject()) continue;
            }
            if(m_dRMatcher->objectsMatch(testPar, refPar)){
              ATH_CHECK( handleOverlap(testPar, refPar) );
            }
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }


  StatusCode DeltaROverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId /*eventContext*/) const
  {
    ATH_MSG_DEBUG("Removing overlaps");

    // Initialize output decoration if necessary
    initializeDecorations(cont1);
    initializeDecorations(cont2);

    if (m_swapContainerPrecedence)
      ATH_CHECK ( internalFindOverlaps(cont2, cont1) );
    else
      ATH_CHECK ( internalFindOverlaps(cont1, cont2) );
    return StatusCode::SUCCESS;
  }

} // namespace ORUtils
