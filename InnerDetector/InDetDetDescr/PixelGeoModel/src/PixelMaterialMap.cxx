/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelMaterialMap.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include <iostream>

PixelMaterialMap::Key::Key(int layerdisk_in, int typenum_in, const std::string & volumeName_in):
  layerdisk(layerdisk_in),
  typenum(typenum_in),
  volumeName(volumeName_in)
{}      

bool
PixelMaterialMap::Key::operator<(const PixelMaterialMap::Key &rhs) const
{
  return 
    ((volumeName < rhs.volumeName) ||
     (volumeName == rhs.volumeName && layerdisk < rhs.layerdisk) ||
     (volumeName == rhs.volumeName && layerdisk == rhs.layerdisk && typenum < rhs.typenum));
}


PixelMaterialMap::PixelMaterialMap(const IRDBRecordset_ptr& mapTable)
{
  for (const auto& rec : *mapTable) {
    int layerdisk = rec->getInt("LAYERDISK");
    int typenum    = rec->getInt("TYPENUM");
    std::string volumeName = rec->getString("VOLUMENAME");
    std::string material   = rec->getString("MATERIAL");
    addMaterial(layerdisk, typenum, volumeName, material);
  }
}

std::string
PixelMaterialMap::getMaterial(int layerdisk, int typenum, const std::string & volumeName) const
{
  // If not found try (layerdisk, 0) then (0, typenum), then (0,0)
  mapType::const_iterator iter;
  iter = m_matmap.find(Key(layerdisk, typenum, volumeName));
  if (iter == m_matmap.end() && typenum) {
    iter = m_matmap.find(Key(layerdisk, 0, volumeName));
  }
  if (iter == m_matmap.end() && layerdisk) {
    iter = m_matmap.find(Key(0, typenum, volumeName));
  }
  if (iter == m_matmap.end() && typenum && layerdisk) {
    iter = m_matmap.find(Key(0, 0, volumeName));
  }
  if (iter != m_matmap.end()) {
    return iter->second;
  } else {
    std::cout << "ERROR: PixelMaterialMap::getMaterial Cannot find material for volumeName: " << volumeName << std::endl;
    return "";
  }
}


void
PixelMaterialMap::addMaterial(int layerdisk, int typenum, const std::string & volumeName, const std::string & materialName) 
{
  m_matmap[Key(layerdisk, typenum, volumeName)] = materialName;
}
