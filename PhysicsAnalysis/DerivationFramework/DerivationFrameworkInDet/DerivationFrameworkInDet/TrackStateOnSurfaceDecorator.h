/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackToVertexWrapper.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_TRACKSTATEONSURFACEDECORATOR_H
#define DERIVATIONFRAMEWORK_TRACKSTATEONSURFACEDECORATOR_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthLinks/ElementLink.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkToolInterfaces/IPRD_AssociationTool.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkToolInterfaces/ITrackHoleSearchTool.h"
#include "TRT_ConditionsServices/ITRT_CalDbTool.h"
#include "TRT_ElectronPidTools/ITRT_ToT_dEdx.h"
#include "TrkToolInterfaces/IUpdator.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "CommissionEvent/ComTime.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"

#include "TrkEventUtils/PRDtoTrackMap.h"

class AtlasDetectorID;
class PixelID;
class SCT_ID;
class TRT_ID;

namespace Trk {
  class IUpdator;
  class PrepRawData;
}

namespace DerivationFramework {

  class TrackStateOnSurfaceDecorator : public extends<ExpressionParserUser<AthAlgTool>, IAugmentationTool> {
    public: 
      using base_class::base_class;

      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches() const;

    private:
      
      
      ElementLink< xAOD::TrackMeasurementValidationContainer > buildElementLink( const Trk::PrepRawData*, 
                                                                  const std::vector<unsigned int>*, 
                                                                  const xAOD::TrackMeasurementValidationContainer* ) const;

      // --- Steering and configuration flags
      Gaudi::Property<bool> m_storeHoles{this, "StoreHoles", true};
      Gaudi::Property<bool> m_storeOutliers{this, "StoreOutliers", true};
      Gaudi::Property<bool> m_storeTRT{this, "StoreTRT", false};
      Gaudi::Property<bool> m_storeSCT{this, "StoreSCT", true};
      Gaudi::Property<bool> m_storePixel{this, "StorePixel", true};
      Gaudi::Property<bool> m_addPulls{this, "AddPulls", true};
      Gaudi::Property<bool> m_addSurfaceInfo{this, "AddSurfaceInfo", true};
      Gaudi::Property<bool> m_addPRD{this, "AddPRD", true};
      Gaudi::Property<bool> m_addExtraEventInfo{this, "AddExtraEventInfo", true};

      // --- Configuration keys
      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
        { this, "EventInfoKey", "EventInfo", "" };
      Gaudi::Property<std::string> m_sgName
         { this, "DecorationPrefix", "IDDET1_",""};
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_containerName
         { this, "ContainerName", "InDetTrackParticles", "" };
      SG::ReadHandleKey<ComTime> m_trtPhaseKey
         { this,"TRTPhaseKey","TRT_Phase", ""};
      StringProperty m_selectionString
	 { this, "SelectionString", "", "track selections"};

      SG::ReadHandleKey<std::vector<unsigned int> > m_pixelMapName
         { this, "PixelMapName", "PixelClustersOffsets" , ""};
      SG::ReadHandleKey<std::vector<unsigned int> >  m_sctMapName
         { this, "SctMapName",   "SCT_ClustersOffsets" , ""};
      SG::ReadHandleKey<std::vector<unsigned int> >  m_trtMapName
         { this, "TrtMapName",   "TRT_DriftCirclesOffsets" , ""};

      SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer > m_pixelClustersName
         {this, "PixelClustersName", "PixelClusters" ,"" };
      SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer > m_sctClustersName
         {this, "SctClustersName", "SCT_Clusters" ,"" };
      SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer> m_trtDCName
         {this, "TrtDriftCirclesName", "TRT_DriftCircles" ,"" };

      SG::ReadHandleKey<Trk::PRDtoTrackMap> m_prdToTrackMap
         { this,"PRDtoTrackMap","","option PRD-to-track association"};

      SG::WriteHandleKey<xAOD::TrackStateValidationContainer> m_pixelMsosName
         { this, "PixelMsosName", "PixelMSOSs", "" };
      SG::WriteHandleKey<xAOD::TrackStateValidationContainer> m_sctMsosName
         { this, "SctMsosName", "SCT_MSOSs", "" };
      SG::WriteHandleKey<xAOD::TrackStateValidationContainer> m_trtMsosName
         { this, "TrtMsosName",  "TRT_MSOSs", ""};


      // --- Read Cond Handle Key
      // For P->T converter of SCT_Clusters
      SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

     
      // --- Services and tools
      const AtlasDetectorID* m_idHelper = nullptr;
      const PixelID*         m_pixId = nullptr;
      const SCT_ID*          m_sctId = nullptr;
      const TRT_ID*          m_trtId = nullptr;

      ToolHandle<Trk::IUpdator> m_updator {this, "Updator", "Trk::KalmanUpdator"};
      ToolHandle<Trk::IResidualPullCalculator> m_residualPullCalculator
	{this, "ResidualPullCalculator",
	 "Trk::ResidualPullCalculator/ResidualPullCalculator"};
      ToolHandle<Trk::ITrackHoleSearchTool> m_holeSearchTool
	{this, "HoleSearch", "InDet::InDetTrackHoleSearchTool/InDetHoleSearchTool"};
      ToolHandle<Trk::IExtrapolator> m_extrapolator
	{this, "TrackExtrapolator", "Trk::Extrapolator/AtlasExtrapolator"};
      ToolHandle<ITRT_CalDbTool> m_trtcaldbTool {this, "TRT_CalDbTool", "TRT_CalDbTool"};
      ToolHandle<ITRT_ToT_dEdx> m_TRTdEdxTool
	{this, "TRT_ToT_dEdx", "InDet::TRT_ElectronPidTools/TRT_ToT_dEdx"};

      // --- Private other members
      std::vector<SG::WriteDecorHandleKey<xAOD::EventInfo> > m_trtPhaseDecorKey;
      enum ETRTFloatDecor {kTRTdEdxDecor,
                           kTRTusedHitsDecor,
                           kTRTdEdx_noHT_divByLDecor,
                           kTRTusedHits_noHT_divByLDecor,
                           kNTRTFloatDecor};
     std::vector<SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> > m_trackTRTFloatDecorKeys;
     enum EPixFloatDecorKeys {kTrkIBLXDecor, kTrkIBLYDecor, kTrkIBLZDecor,
                              kTrkBLXDecor,  kTrkBLYDecor,  kTrkBLZDecor,
                              kTrkL1XDecor,  kTrkL1YDecor,  kTrkL1ZDecor,
                              kTrkL2XDecor,  kTrkL2YDecor,  kTrkL2ZDecor,
                              kNPixFloatDecor };
     std::vector<SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> > m_trackPixFloatDecorKeys;
     SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>               m_trackTSOSMOSLinkDecorKey;
     Gaudi::Property< std::vector<float> > m_pixelLayerRadii {this, "PixelLayerRadii", {29.5,50.5,88.5,122.5}, "Radii to extrapolate to for estimating track position on layers" };


  }; 
}

#endif // DERIVATIONFRAMEWORK_TRACKSTATEONSURFACEDECORATOR_H
