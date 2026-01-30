/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELINTERFACES_IGEOSUBDETTOOL_H
#define GEOMODELINTERFACES_IGEOSUBDETTOOL_H

#include "GaudiKernel/IAlgTool.h"

class StoreGateSvc;
class GeoVPhysVol;

class IGeoSubDetTool : public virtual IAlgTool {
public:
   DeclareInterfaceID( IGeoSubDetTool, 1, 0 );

    // Build subdetector in parent
    virtual StatusCode build( GeoVPhysVol* parent ) = 0;
};

#endif // GEOMODELINTERFACES_IGEOSUBDETTOOL_H
