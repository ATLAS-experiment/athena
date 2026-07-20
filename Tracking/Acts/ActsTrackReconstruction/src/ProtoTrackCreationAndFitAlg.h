/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRACKRECONSTRUCTION_PROTOTRACKCREATIONANDFITALG_H
#define ACTSTRACKRECONSTRUCTION_PROTOTRACKCREATIONANDFITALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TrkParameters/TrackParameters.h"

#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsToolInterfaces/IProtoTrackCreatorTool.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEvent/ProtoTrackCollection.h"
#include "ActsEvent/ContextUtility.h"

namespace ActsTrk{
    class ProtoTrackCreationAndFitAlg: public ::AthReentrantAlgorithm { 
    public: 
    
    using ::AthReentrantAlgorithm::AthReentrantAlgorithm;
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
      ToolHandle<IProtoTrackCreatorTool> m_patternBuilder{this, "PatternBuilder", "", "the pattern builder to use"};
      // the track fitter to use for the refit 
      ToolHandle<IFitterTool> m_actsFitter{this, "ActsFitter", "", "Choice of Acts Fitter (Kalman by default)"};
      
      /** @brief Utility to fetch the geometry, magnetic field and calibration context in the event */
      ContextUtility m_ctxProvider{this};
      // output location to write to 
      SG::WriteHandleKey<TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "", "Output track collection (ActsTrk variant)"};

      // output location for proto tracks (can be empty)
      SG::WriteHandleKey<ProtoTrackCollection> m_protoTrackCollectionKey{this, "ProtoTracksLocation", "", "Output proto tracks as well"};
      Gaudi::Property<bool> m_copyParametersFromFit{this, "copyParametersFromFit", true, "If enabled proto tracks will have the same parameters as fitted tracks"};
      // acts helper for the output
      MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper{this};



    }; 

}

#endif //> !ACTSTRACKRECONSTRUCTION_PROTOTRACKCREATIONANDFITALG_H
