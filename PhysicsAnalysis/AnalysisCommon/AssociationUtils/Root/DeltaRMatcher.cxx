/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// Framework includes
#include "ColumnarCore/MomentumHelpers.h"

// Local includes
#include "AssociationUtils/DeltaRMatcher.h"

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // DeltaRMatcher constructor
  //---------------------------------------------------------------------------
  DeltaRMatcher::DeltaRMatcher(double dR, bool useRapidity)
    : m_dR(dR),
      m_useRapidity(useRapidity)
  {}

  StatusCode DeltaRMatcher::setObjectTypes (xAODType::ObjectType type1,
                                      xAODType::ObjectType type2)
  {
    columnar::resetObjectType (m_momAcc1, *this, type1);
    columnar::resetObjectType (m_momAcc2, *this, type2);
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Check if particles match in dR
  //---------------------------------------------------------------------------
  bool DeltaRMatcher::objectsMatch(columnar::Particle1Id p1,
                                   columnar::Particle2Id p2,
                                   bool /*swapArgs*/) const
  {
    return columnar::isInDeltaR(m_momAcc1, p1, m_momAcc2, p2, m_dR, m_useRapidity);
  }

  //---------------------------------------------------------------------------
  // SlidingDeltaRMatcher constructor
  //---------------------------------------------------------------------------
  SlidingDeltaRMatcher::SlidingDeltaRMatcher(double c1, double c2,
                                             double maxCone, bool useRapidity)
    : m_c1(c1), m_c2(c2), m_maxCone(maxCone), m_useRapidity(useRapidity)
  {}

  StatusCode SlidingDeltaRMatcher::setObjectTypes (xAODType::ObjectType type1,
                                                   xAODType::ObjectType type2)
  {
    columnar::resetObjectType (m_momAcc1, *this, type1);
    columnar::resetObjectType (m_momAcc2, *this, type2);
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Check if particles match in sliding dR
  //---------------------------------------------------------------------------
  bool SlidingDeltaRMatcher::objectsMatch(columnar::Particle1Id p1,
                                          columnar::Particle2Id p2,
                                          bool swapArgs) const
  {
    // Calculate the dR cone to match with
    double pt1 = swapArgs ? m_momAcc2.pt(p2) : m_momAcc1.pt(p1);
    double dR = m_c1 + (m_c2 / pt1);
    // Apply upper limit to the dR match cone
    dR = std::min(dR, m_maxCone);
    return columnar::isInDeltaR(m_momAcc1, p1, m_momAcc2, p2, dR, m_useRapidity);
  }

} // namespace ORUtils
