/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MAGFIELDCONDITIONS_ATLASMTFIELDMAPCONDOBJ
#define MAGFIELDCONDITIONS_ATLASMTFIELDMAPCONDOBJ

// MagField includes
#include "AthenaKernel/CondCont.h"
#include "MagFieldElements/AtlasFieldMap.h"

class AtlasFieldMapCondObj {
 public:
  AtlasFieldMapCondObj() = default;
  // Rule of 5 for copy/move/dtor
  AtlasFieldMapCondObj(AtlasFieldMapCondObj&&) = default;
  AtlasFieldMapCondObj& operator=(AtlasFieldMapCondObj&&) = default;
  ~AtlasFieldMapCondObj() = default;
  // delete copy as we hold a unique ptr. Moveable not copyable
  AtlasFieldMapCondObj& operator=(const AtlasFieldMapCondObj&) = delete;
  AtlasFieldMapCondObj(const AtlasFieldMapCondObj&) = delete;

  // access to field map
  const MagField::AtlasFieldMap* fieldMap() const { return m_fieldMap.get(); }

  // setter
  void setFieldMap(std::unique_ptr<MagField::AtlasFieldMap> fieldMap);

 private:
  // field map
  std::unique_ptr<MagField::AtlasFieldMap> m_fieldMap{nullptr};
};

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( AtlasFieldMapCondObj, 122915602, 1)
CONDCONT_DEF (AtlasFieldMapCondObj, 249359246);


#endif // MAGFIELDCONDITIONS_ATLASMTFIELDMAPCONDOBJ

