/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETRIOMAKER_XAODTOINDETCLUSTERCONVERSION_H
#define INDETRIOMAKER_XAODTOINDETCLUSTERCONVERSION_H

//STL
#include <string>

//Gaudi
#include "GaudiKernel/ToolHandle.h"

// Base class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

//InDet
//can't fwd declare this, needed for typedef to Pixel_RDO_Container
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"

#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

#include "HGTD_PrepRawData/HGTD_ClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h" 

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetPrepRawData/SiClusterContainer.h"

#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElementCollection.h"

class PixelID;
class SCT_ID;
class HGTD_ID;

namespace InDet {

class XAODToInDetClusterConversion 
  : public AthReentrantAlgorithm {
 public:
  
  /// Constructor with parameters:
  XAODToInDetClusterConversion(const std::string &name,ISvcLocator *pSvcLocator);

  //@name Usual algorithm methods
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  /**    @name Disallow default instantiation, copy, assignment */
  //@{
  //@}
  XAODToInDetClusterConversion() = delete;
  XAODToInDetClusterConversion(const XAODToInDetClusterConversion&) = delete;
  XAODToInDetClusterConversion &operator=(const XAODToInDetClusterConversion&) = delete;
   //@}

 private:
  StatusCode convertPixelClusters(const EventContext& ctx) const;
  StatusCode convertStripClusters(const EventContext& ctx) const;
  StatusCode convertHgtdClusters(const EventContext& ctx) const;
  
 private:
  const PixelID* m_pixelID {}; 
  const SCT_ID* m_stripID {};
  const HGTD_ID* m_hgtdID {};
  
  ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool {this, "LorentzAngleTool", "", "Tool to retrieve Lorentz angle of SCT"};

  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey {this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_inputPixelClusterContainerKey {this, "InputPixelClustersName", "ITkPixelClusters", "name of the input xAOD pixel cluster container"};

  SG::WriteHandleKey<InDet::PixelClusterContainer> m_outputPixelClusterContainerKey {this, "OutputPixelClustersName", "ITkPixelClusters", "name of the output InDet pixel cluster container"};
  SG::WriteHandleKey< InDet::SiClusterContainer > m_pixelClusterContainerLinkKey {this, "PixelClustersLinkName", "ITkPixelClusters"};

  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey {this, "StripDetEleCollKey", "ITkStripDetectorElementCollection", "Key of SiDetectorElementCollection for Strip"};
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_inputStripClusterContainerKey {this, "InputStripClustersName", "ITkStripClusters", "name of the input xAOD strip cluster container"};

  SG::WriteHandleKey<InDet::SCT_ClusterContainer> m_outputStripClusterContainerKey {this, "OutputStripClustersName", "ITkStripClusters", "name of the output InDet pixel cluster container"};
  SG::WriteHandleKey< InDet::SiClusterContainer > m_stripClusterContainerLinkKey {this, "StripClustersLinkName", "ITkStripClusters"};

  SG::ReadCondHandleKey<InDetDD::HGTD_DetectorElementCollection> m_HGTDDetEleCollKey{this, "HGTDDetEleCollKey", "HGTD_DetectorElementCollection", "Key of HGTD_DetectorElementCollection for HGTD"};
  
  SG::ReadHandleKey<xAOD::HGTDClusterContainer> m_inputHgtdClusterContainerKey {this, "InputHGTDClustersName", "HGTD_Clusters", "name of the input xAOD hgtd cluster container"};
  SG::WriteHandleKey<::HGTD_ClusterContainer> m_outputHgtdClusterContainerKey {this, "OutputHGTDClustersName", "HGTD_Clusters", "name of the output InDet hgtd cluster container"};
  
  Gaudi::Property<bool> m_processPixel {this, "ProcessPixel", false};
  Gaudi::Property<bool> m_processStrip {this, "ProcessStrip", false};
  Gaudi::Property<bool> m_processHgtd {this, "ProcessHgtd", false};
};

}

#endif // INDETRIOMAKER_XAODTOINDETCLUSTERCONVERSION_H
