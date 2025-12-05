/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "AssociationUtils/ObjLinkOverlapTool.h"

namespace
{
  /// Unit conversion constant
  const float invGeV = 0.001;
}

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  ObjLinkOverlapTool::ObjLinkOverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode ObjLinkOverlapTool::initializeDerived()
  {
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode ObjLinkOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId /*eventContext*/) const
  {
    ATH_MSG_DEBUG("Removing overlaps");

    // Initialize output decoration if necessary
    initializeDecorations(cont1);
    initializeDecorations(cont2);

    // Loop over surviving input objects in cont1
    for(const auto p1 : cont1){
      if(isSurvivingObject(p1)){

        // Check for existence of an object link
        auto linkParticle = m_objLinkHelper1->getObjectLink(p1, cont2);
        if(linkParticle){

          // See if the link matches a surviving input in cont2. the
          // above call will have already checked that it is in the
          // right container
          if(isSurvivingObject(*linkParticle)){
            if constexpr (columnar::ColumnarModeDefault::isXAOD) {
              ATH_MSG_DEBUG("  Found overlap " << p1.getXAODObject().type() <<
                          " pt " << p1.getXAODObject().pt()*invGeV);
            } else {
              ATH_MSG_DEBUG("  Found overlap " << p1);
            }
            setObjectFail(p1);
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }

} // namespace ORUtils
