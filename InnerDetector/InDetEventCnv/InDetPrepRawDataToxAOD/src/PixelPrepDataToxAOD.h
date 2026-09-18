/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file InDetPrepRawDataToxAOD/PixelPrepDataToxAOD.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date November, 2019
 * @brief Store pixel data in xAOD.
 */

///////////////////////////////////////////////////////////////////
// PixelPrepDataToxAOD.h
//   Header file for class PixelPrepDataToxAOD
///////////////////////////////////////////////////////////////////

#ifndef PIXELPREPDATATOXAOD_H
#define PIXELPREPDATATOXAOD_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

#include "GeneratorObjects/xAODTruthParticleLink.h"

#include "PixelConditionsData/PixelDCSStateData.h"
#include "PixelConditionsData/PixelDCSStatusData.h"
#include "PixelConditionsData/PixelDCSHVData.h"
#include "PixelConditionsData/PixelDCSTempData.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "InDetConditionsSummaryService/IInDetConditionsTool.h"

#include "StoreGate/ReadCondHandleKey.h"

#include "TrkEventUtils/ClusterSplitProbabilityContainer.h"

#include <string>

class PixelID;
class SiHit;
class InDetSimDataCollection;

class IdentifierHash;

namespace InDet
{
  class PixelCluster;
}

namespace InDetDD
{
  class SiCellId; 
}



class PixelPrepDataToxAOD : public AthAlgorithm  {

public:
  // Constructor with parameters:
  using AthAlgorithm::AthAlgorithm;

  // Basic algorithm methods:
  virtual StatusCode initialize();
  virtual StatusCode execute(const EventContext& ctx);
  virtual StatusCode finalize();

private:

  std::vector< std::vector< int > >  addSDOInformation( xAOD::TrackMeasurementValidation* xprd,
							const InDet::PixelCluster* prd,
							const InDetSimDataCollection& sdoCollection ) const;


  void  addSiHitInformation( xAOD::TrackMeasurementValidation* xprd, 
                             const InDet::PixelCluster* prd,
                             const std::vector<SiHit> & matchingHits ) const;
  
  std::vector<SiHit>  findAllHitsCompatibleWithCluster(const InDet::PixelCluster* prd,
                                                       const std::vector<const SiHit*>* sihits,
						       std::vector< std::vector< int > > & trkBCs) const;


  void  addNNTruthInfo( xAOD::TrackMeasurementValidation* xprd,
                        const InDet::PixelCluster* prd, 
                        const std::vector<SiHit> & matchingHits ) const;

  void  addNNInformation( xAOD::TrackMeasurementValidation* xprd, 
                         const InDet::PixelCluster* pixelCluster, 
                         const unsigned int SizeX, 
                         const unsigned int SizeY ) const;

  void  addRdoInformation( xAOD::TrackMeasurementValidation* xprd,
                           const InDet::PixelCluster* pixelCluster,
                           const PixelChargeCalibCondData *calibData) const;



   InDetDD::SiCellId getCellIdWeightedPosition(  const InDet::PixelCluster* pixelCluster,
                                                 int *rrowMin = 0,
                                                 int *rrowMax = 0,
                                                 int *rcolMin = 0,
                                                 int *rcolMax = 0 ) const;

  const PixelID *m_PixelHelper = nullptr;

  Gaudi::Property<bool> m_useTruthInfo{this, "UseTruthInfo", false};
  Gaudi::Property<bool> m_writeSDOs{this, "WriteSDOs", false};
  Gaudi::Property<bool> m_writeSiHits{this, "WriteSiHits", false};
  Gaudi::Property<bool> m_writeNNinformation{this, "WriteNNinformation", true};
  Gaudi::Property<bool> m_writeRDOinformation{this, "WriteRDOinformation", true};
  Gaudi::Property<bool> m_writeExtendedPRDinformation
    {this, "WriteExtendedPRDinformation", false};
  // hasBSError and DCSState on their own, without the per-RDO information
  // (and its charge calibration) that WriteRDOinformation also writes.
  Gaudi::Property<bool> m_writeModuleStatus
    {this, "WriteModuleStatus", false};
  Gaudi::Property<bool> m_useSiHitsGeometryMatching
    {this, "UseSiHitsGeometryMatching", true};

  ServiceHandle<InDetDD::IPixelReadoutManager> m_pixelReadout
  {this, "PixelReadoutManager", "PixelReadoutManager", "Pixel readout manager" };

  SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey
  {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "Pixel charge calibration data"};

  SG::ReadCondHandleKey<PixelDCSStateData> m_condDCSStateKey
  {this, "PixelDCSStateCondData", "PixelDCSStateCondData", "Pixel FSM state key"};

  SG::ReadCondHandleKey<PixelDCSStatusData> m_condDCSStatusKey
  {this, "PixelDCSStatusCondData", "PixelDCSStatusCondData", "Pixel FSM status key"};

  SG::ReadCondHandleKey<PixelDCSTempData> m_readKeyTemp
  {this, "ReadKeyTemp", "PixelDCSTempCondData", "Key of input sensor temperature conditions folder"};

  SG::ReadCondHandleKey<PixelDCSHVData> m_readKeyHV
  {this, "ReadKeyHV",    "PixelDCSHVCondData", "Key of input bias voltage conditions folder"};

  ToolHandle<IInDetConditionsTool> m_pixelSummary
  {this, "PixelConditionsSummaryTool", "PixelConditionsSummaryTool", "Tool for PixelConditionsSummaryTool"};

  ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool
  {this, "LorentzAngleTool", "SiLorentzAngleTool", "Tool to retreive Lorentz angle"};

  SG::ReadHandleKey<Trk::ClusterSplitProbabilityContainer>   m_clusterSplitProbContainer
  {this, "ClusterSplitProbabilityName", "",""};

  // -- Private members   
  mutable std::atomic<unsigned int> m_haveTruthLink {};
  mutable std::atomic<unsigned int> m_missingTruthParticle {};
  mutable std::atomic<unsigned int> m_missingParentParticle {};
  bool m_firstEventWarnings = true;
  bool m_need_sihits = false;

  SG::ReadHandleKey<InDet::PixelClusterContainer> m_clustercontainer_key
    {this, "SiClusterContainer", "PixelClusters"};
  SG::ReadHandleKey<SiHitCollection> m_sihitContainer_key
    {this, "MC_Hits", "PixelHits"};
  SG::ReadHandleKey<InDetSimDataCollection> m_SDOcontainer_key
    {this, "MC_SDOs", "PixelSDO_Map"};
  SG::ReadHandleKey<PRD_MultiTruthCollection> m_multiTruth_key
    {this, "PRD_MultiTruth", "PRD_MultiTruthPixel"};
  SG::ReadHandleKey<xAODTruthParticleLinkVector> m_truthParticleLinks
     {this,"InputTruthParticleLinks","","The key for the truth particle link collection."};

  SG::WriteHandleKey<xAOD::TrackMeasurementValidationContainer> m_write_xaod_key
    {this, "OutputClusterContainer", "PixelClusters"};
  SG::WriteHandleKey<std::vector<unsigned int>> m_write_offsets
    {this, "PixelxAodOffset", "PixelClustersOffsets"};
};


#endif 
