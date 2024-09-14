/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MAGFIELDCONDITIONS_ATLASMTFIELDCONDOBJ
#define MAGFIELDCONDITIONS_ATLASMTFIELDCONDOBJ

// MagField includes
#include "AthenaKernel/CondCont.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MagFieldElements/AtlasFieldCache.h"

// forward declarations
namespace MagField {
    class AtlasFieldMap;
}


class AtlasFieldCacheCondObj {

 public:
  AtlasFieldCacheCondObj() = default;
  // Rule of 5 for copy/move/dtor
  AtlasFieldCacheCondObj(AtlasFieldCacheCondObj&&) = default;
  AtlasFieldCacheCondObj& operator=(AtlasFieldCacheCondObj&&) = default;
  AtlasFieldCacheCondObj& operator=(const AtlasFieldCacheCondObj&) = default;
  AtlasFieldCacheCondObj(const AtlasFieldCacheCondObj&) = default;
  ~AtlasFieldCacheCondObj() = default;

  /** get B field cache for evaluation as a function of 2-d  or 3-d
     position. Resets cache to an initialized state */
  void getInitializedCache(MagField::AtlasFieldCache& cache) const {
    cache = MagField::AtlasFieldCache(m_solFieldScale, m_torFieldScale, m_fieldMap);
  }

  /** access to solenoid field scale factor */
  double solenoidFieldScaleFactor() const { return m_solFieldScale; }

  /** access to toroid field scale factor */
  double toriodFieldScaleFactor() const { return m_torFieldScale; }

  /** access to non-owning AtlasFieldMap*/
  const MagField::AtlasFieldMap* fieldMap() const { return m_fieldMap; }

  /** set values for field scale and service to be able to build the cache **/
  void initialize(double solFieldScale, double torFieldScale,
                  const MagField::AtlasFieldMap* fieldMap) {

    m_solFieldScale = solFieldScale;
    m_torFieldScale = torFieldScale;
    m_fieldMap = fieldMap;
  }

 private:
  double m_solFieldScale{1};
  double m_torFieldScale{1};
  //not owning ptr to Field Map
  const MagField::AtlasFieldMap* m_fieldMap{nullptr};
};

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( AtlasFieldCacheCondObj, 258146572, 1)
CONDCONT_DEF (AtlasFieldCacheCondObj, 3622068);


#endif // MAGFIELDCONDITIONS_ATLASMTFIELDCONDOBJ

