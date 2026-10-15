/* 
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS Collaboration
*/

#include "AssociationUtils/FEOverlapTool.h"

namespace ADD_CMODE(ORUtils) {

  //------------------------------------------------------
  // Constructor
  //------------------------------------------------------
  FEOverlapTool::FEOverlapTool(const std::string& name) : BaseOverlapTool(name) {
  }

  //------------------------------------------------------
  // Initialize
  //------------------------------------------------------
  StatusCode FEOverlapTool::initializeDerived() {
    ATH_CHECK(m_feMapKey.initialize());

    // Initialize the dR matcher
    m_FEMatcher = std::make_unique<FEMatcher>();

    return StatusCode::SUCCESS;
  }

  //------------------------------------------------------
  // Identify overlaps
  //------------------------------------------------------  
  template<columnar::ContainerIdConcept CI1,columnar::RegularContainerIdConcept CI2>
  StatusCode FEOverlapTool::
  internalFindOverlaps(columnar::ObjectRange<CI1> testCont, columnar::ObjectRange<CI2> refCont) const
  {
    SG::ReadHandle<xAOD::MissingETAssociationMap> fe_map(m_feMapKey);
    // Loop over surviving input objects
    for (const auto testPar : testCont) {
      if (isSurvivingObject(testPar)) {
        for (const auto refPar : refCont) {
          if (isSurvivingObject(refPar)) {
            if (m_FEMatcher->objectsMatch(&testPar.getXAODObject(), &refPar.getXAODObject(), *fe_map)) {
              ATH_CHECK(handleOverlap(testPar, refPar));
            }
            
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode FEOverlapTool::findOverlaps(columnar::Particle1Range cont1, 
                                        columnar::Particle2Range cont2, 
                                        columnar::EventContextId) const
  {
    ATH_MSG_DEBUG("Removing overlaps");

    // Initialize output decorations if necessary
    initializeDecorations(cont1);
    initializeDecorations(cont2);

    ATH_CHECK(internalFindOverlaps(cont1,cont2));
  
    return StatusCode::SUCCESS;
  }
} // namespace ADD_CMODE(ORUtils)

