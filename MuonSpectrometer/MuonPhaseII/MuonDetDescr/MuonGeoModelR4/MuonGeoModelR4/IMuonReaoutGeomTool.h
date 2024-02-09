/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_IMUONREAOUDGEOMTOOL_H
#define MUONGEOMODELR4_IMUONREAOUDGEOMTOOL_H

#include "GaudiKernel/IAlgTool.h"
/***
 *   The IMuonReadoutGeomTool provides an Inteface for a class of tools that
 * create the ReadoutElements for each Muon Detector and append then to the
 * MuonDetectorManager
 * **/
#include <map>

class GeoFullPhysVol;

namespace MuonGMR4 {
class MuonDetectorManager;
class IMuonReadoutGeomTool : virtual public IAlgTool {
   public:
    /// Gaudi interface ID
    DeclareInterfaceID(IMuonReadoutGeomTool, 1, 0);
    
    using physNodeMap = std::map<std::string, GeoFullPhysVol*>;
    /// Retrieves the GeoModel from the GeoModelSvc and append the
    /// ReadoutElements of the Given MuonDetectorTechnology to the
    /// MuonDetectorManager
    virtual StatusCode buildReadOutElements(MuonDetectorManager& mgr) = 0;
};
}  // namespace MuonGMR4
#endif
