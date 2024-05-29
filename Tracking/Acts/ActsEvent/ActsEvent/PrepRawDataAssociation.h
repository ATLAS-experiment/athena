/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRD_ASSOCIATION_H
#define PRD_ASSOCIATION_H

#include "xAODMeasurementBase/MeasurementDefs.h"

#include <utility>
#include <unordered_set>

namespace ActsTrk
{

  class PrepRawDataAssociation {
  public:
    PrepRawDataAssociation() = default;
    PrepRawDataAssociation(const PrepRawDataAssociation&) = default;
    PrepRawDataAssociation(PrepRawDataAssociation&&) noexcept = delete;
    PrepRawDataAssociation& operator=(const PrepRawDataAssociation&) = default;
    PrepRawDataAssociation& operator=(PrepRawDataAssociation&&) noexcept = delete;
    ~PrepRawDataAssociation() = default;
    
    std::pair<typename std::unordered_set<xAOD::DetectorIdentType>::iterator, bool> markAsUsed(xAOD::DetectorIdentType id) { return m_prds.insert(id); }
    bool isUsed(xAOD::DetectorIdentType id) const { return m_prds.find(id) != m_prds.end(); }
    std::size_t size() const { return m_prds.size(); }
    
  private:
    std::unordered_set<xAOD::DetectorIdentType> m_prds {};
  };

}

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::PrepRawDataAssociation, 101405812, 1 )

#endif
