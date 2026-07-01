/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimDETECTORTOOL_H
#define FPGATrackSimDETECTORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"


#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ServiceHandle.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"

#include <string>
#include <vector>
#include <list>
#include <set>
#include <fstream>

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"

class PixelID;
class SCT_ID;


/* This class interface the ID hits with the FPGATrackSim simulation
    implemented in Athena. Original code */

class FPGATrackSimDetectorTool : public ::AthAlgTool {
 public:

  FPGATrackSimDetectorTool(const std::string&, const std::string&, const IInterface*);
  virtual ~FPGATrackSimDetectorTool() = default;
  virtual StatusCode initialize() override;
  void dumpGlobalToLocalModuleMap();

private:
  ServiceHandle<IFPGATrackSimMappingSvc>   m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
  Gaudi::Property<std::string> m_global2local_path {this,"GlobalToLocalMapPath", "global-to-local-map.moduleidmap"};
  Gaudi::Property<std::string> m_sram_path_pix     {this, "SRAMPathPixel", "sram_lookup_pixel.txt"};
  Gaudi::Property<std::string> m_sram_path_sct     {this, "SRAMPathSCT", "sram_lookup_sct.txt"};
  Gaudi::Property<bool>        m_dumpAllModules    {this, "dumpAllModules", false};


  const InDetDD::SiDetectorManager*  m_PIX_mgr = nullptr;
  const PixelID*   m_pixelId = nullptr;
  const SCT_ID*    m_sctId = nullptr;

};

#endif // FPGATrackSimDETECTORTOOL_H
