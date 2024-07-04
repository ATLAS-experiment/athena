/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <Gaudi/Property.h>
#ifndef ACTSTRACKRECONSTRUCTION_PROTOTRACKCREATIONANDFITALG_H
#define ACTSTRACKRECONSTRUCTION_PROTOTRACKCREATIONANDFITALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TrkParameters/TrackParameters.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"

#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsToolInterfaces/IProtoTrackCreatorTool.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEvent/ProtoTrackCollection.h"

namespace
{
  // Forward-declare internal classes defined in TrackFindingData.h and used only in TrackFindingAlg.cxx.
  // Define in the anonymous namespace to prevent unnecessary external linkage.
  class TrackFindingMeasurements;
  class DuplicateSeedDetector;
}



namespace ActsTrk{
    class ProtoTrackCreationAndFitAlg: public ::AthReentrantAlgorithm { 
    public: 
    ProtoTrackCreationAndFitAlg( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~ProtoTrackCreationAndFitAlg() = default;

    ///uncomment and implement methods as required

                                            //IS EXECUTED:
    virtual StatusCode  initialize() override final;     //once, before any input is loaded
    virtual StatusCode  execute(const EventContext & ctx) const override final;
    
    private: 

      // the pixel clusters to read as input 
      SG::ReadHandleKey<xAOD::PixelClusterContainer> m_PixelClusters{this, "PixelClusterContainer","","the pix clusters"};
      // the strip clusters to read as input 
      SG::ReadHandleKey<xAOD::StripClusterContainer> m_StripClusters{this, "StripClusterContainer","","the strip clusters"};
      // the user-provided pattern recognition tool to test 
      ToolHandle<ActsTrk::IProtoTrackCreatorTool> m_patternBuilder{this, "PatternBuilder", "", "the pattern builder to use"};
      // the track fitter to use for the refit 
      ToolHandle<ActsTrk::IFitterTool> m_actsFitter{this, "ActsFitter", "", "Choice of Acts Fitter (Kalman by default)"};
      // tracking geometry - used to translate ATLAS to ACTS geometry
      ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
      // more conversion helpers
      ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", ""};
      // detector element collections - again needed for geometry translation 
      SG::ReadCondHandleKeyArray<InDetDD::SiDetectorElementCollection> m_detEleCollKeys{this, "DetectorElementCollectionKeys", {}, "input SiDetectorElementCollection"};
      // ACTS extrapolation tool - provides the magnetic field 
      ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
      // output location to write to 
      SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "", "Output track collection (ActsTrk variant)"};

      // output location for proto tracks (can be empty)
      SG::WriteHandleKey<ActsTrk::ProtoTrackCollection> m_protoTrackCollectionKey{this, "ProtoTracksLocation", "", "Output proto tracks as well"};
      Gaudi::Property<bool> m_copyParametersFromFit{this, "copyParametersFromFit", true, "If enabled proto tracks will have the same parameters as fitted tracks"};
      // acts helper for the output
      ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper;



    }; 

}

#endif //> !ACTSTRACKRECONSTRUCTION_PROTOTRACKCREATIONANDFITALG_H
