/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCALIBALGS_PIXELCHARGETOTCONVERSION_H
#define PIXELCALIBALGS_PIXELCHARGETOTCONVERSION_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelGeoModel/IIBLParameterSvc.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "GaudiKernel/ServiceHandle.h"

#include <string>
#include <vector>

// FIXME: Modifies data in SG!
class ATLAS_NOT_THREAD_SAFE PixelChargeToTConversion : public AthAlgorithm{
  
 public:
  PixelChargeToTConversion(const std::string& name, ISvcLocator* pSvcLocator);
  ~PixelChargeToTConversion();
  
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  
 private:
  ServiceHandle<IIBLParameterSvc> m_IBLParameterSvc
  {this, "IBLParameterSvc", "IBLParameterSvc"};

  SG::ReadHandleKey<InDet::PixelClusterContainer> m_pixelsClustersKey
  {this, "PixelClusterContainer",  "PixelClusters", ""};

  ServiceHandle<InDetDD::IPixelReadoutManager> m_pixelReadout
  {this, "PixelReadoutManager", "PixelReadoutManager", "Pixel readout manager" };


  SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey
  {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "Charge calibration"};

  // For P->T converter of PixelClusters
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection>
    m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "PixelDetectorElementCollection",
      "Key of SiDetectorElementCollection for Pixel"};

  bool m_doIBL = true; // Properly set in initialize()

};

#endif
