/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_PIXELMATERIALMAP_H
#define PIXELGEOMODEL_PIXELMATERIALMAP_H

// Class to interpret and query table PixelMaterialMap


#include "RDBAccessSvc/IRDBAccessSvc.h"
#include <string_view>
#include <string>
#include <map>

class PixelMaterialMap
{

public:
  PixelMaterialMap(const IRDBRecordset_ptr& mapTable);

  void addMaterial(int layerdisk, int typenum, std::string_view volumeName, std::string_view materialName);
  std::string getMaterial(int layerdisk, int typenum, std::string_view volumeName) const;

private:
  class Key 
  {
  public:
    Key(int layerdisk_in, int typenum_in, std::string_view volumeName_in);
    int layerdisk{};
    int typenum{};
    std::string volumeName;
    bool operator<(const Key &rhs) const;
  };

  typedef std::map<Key, std::string> mapType;
  mapType m_matmap;

};

#endif // PixelMaterialMap
