/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_PIXELSTAVETYPES_H
#define PIXELGEOMODEL_PIXELSTAVETYPES_H

// Class to interpret and query PixelStaveType table

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include <map>

class PixelStaveTypes {

public :
  PixelStaveTypes(const IRDBRecordset_ptr& table);
  int getFluidType(int layer, int phiModule) const;
  int getBiStaveType(int layer, int phiModule) const;

private :
  class Key 
  {
  public:
    Key(int layer_in, int phiModule_in);
    int layer;
    int phiModule;
    bool operator<(const Key &rhs) const;
  };

  class Datum
  {
  public:
    Datum(int fluidType_in = 0, int biStaveType_in = 0);
    int fluidType;
    int biStaveType;
  };

  const Datum & getData(int layer, int phiModule) const;
  
  typedef std::map<Key, Datum> MapType;
  MapType m_dataLookup;

  std::map<int,int> m_maxSector;
  
  static const Datum s_defaultDatum;

};

#endif // PixelStaveTypes_H
