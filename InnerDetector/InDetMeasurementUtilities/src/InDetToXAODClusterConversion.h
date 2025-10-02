/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///
/// @file InDetToXAODClusterConversion.h
/// Algorithm to convert InDet::PixelClusters and InDet::SCT_Clusters
/// to xAOD::PixelClusters and xAOD::StripClusters.
/// This is a temporary solution before a proper
/// clustering algorithm is implemented.
///

#ifndef INDETRIOMAKER_INDETTOXAODCLUSTERCONVERSION_H
#define INDETRIOMAKER_INDETTOXAODCLUSTERCONVERSION_H
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
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "HGTD_PrepRawData/HGTD_ClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElementCollection.h"

class PixelID;
class SCT_ID;
class HGTD_ID;

namespace InDet {

class InDetToXAODClusterConversion : public AthReentrantAlgorithm {
public:

  /// Constructor with parameters:
  InDetToXAODClusterConversion(const std::string &name,ISvcLocator *pSvcLocator);

  //@name Usual algorithm methods
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  /**    @name Disallow default instantiation, copy, assignment */
  //@{
  //@}
  InDetToXAODClusterConversion() = delete;
  InDetToXAODClusterConversion(const InDetToXAODClusterConversion&) = delete;
  InDetToXAODClusterConversion &operator=(const InDetToXAODClusterConversion&) = delete;
   //@}

private:
  StatusCode convertPixelClusters(const EventContext& ctx) const;
  StatusCode convertStripClusters(const EventContext& ctx) const;
  StatusCode convertHgtdClusters(const EventContext& ctx) const;
  
private:
  const PixelID* m_pixelID {};
  const SCT_ID* m_stripID {};
  const HGTD_ID* m_hgtdID {};
  
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey{this, "StripDetEleCollKey", "ITkStripDetectorElementCollection", "Key of SiDetectorElementCollection for Strip"};
  SG::ReadCondHandleKey<InDetDD::HGTD_DetectorElementCollection> m_HGTDDetEleCollKey{this, "HGTDDetEleCollKey", "HGTD_DetectorElementCollection", "Key of HGTD_DetectorElementCollection for HGTD"};
  
  SG::ReadHandleKey<InDet::PixelClusterContainer> m_inputPixelClusterContainerKey {this, "InputPixelClustersName", "ITkPixelClusters", "name of the input InDet pixel cluster container"};
  SG::ReadHandleKey<InDet::SCT_ClusterContainer>  m_inputStripClusterContainerKey {this, "InputStripClustersName", "ITkStripClusters", "name of the input InDet strip cluster container"};

  SG::WriteHandleKey<xAOD::PixelClusterContainer> m_outputPixelClusterContainerKey {this, "OutputPixelClustersName", "ITkPixelClusters", "name of the output xAOD pixel cluster container"};
  SG::WriteHandleKey<xAOD::StripClusterContainer> m_outputStripClusterContainerKey {this, "OutputStripClustersName", "ITkStripClusters", "name of the output xAOD strip cluster container"};

  SG::ReadHandleKey<::HGTD_ClusterContainer> m_inputHgtdClusterContainerKey {this, "InputHgtdClustersName", "HGTD_Clusters", "name of the input hgtd cluster container"};
  SG::WriteHandleKey<xAOD::HGTDClusterContainer> m_outputHgtdClusterContainerKey {this, "OutputHgtdClustersName", "HGTD_Clusters", "name of the output xAOD hgtd cluster container"};
  
  Gaudi::Property<bool> m_processPixel {this, "ProcessPixel", false};
  Gaudi::Property<bool> m_processStrip {this, "ProcessStrip", false};
  Gaudi::Property<bool> m_processHgtd {this, "ProcessHgtd", false};
  Gaudi::Property<bool> m_isITk {this, "isITk", true, "True if running in ITk"};
};

}

#endif // INDETRIOMAKER_INDETTOXAODCLUSTERCONVERSION_H

