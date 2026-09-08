/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_FPGATRACKSIMPROTOTRACKFITTERALG_H
#define ACTSTRACKRECONSTRUCTION_FPGATRACKSIMPROTOTRACKFITTERALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"


#include "ActsToolInterfaces/IFitterTool.h"

#include "StoreGate/CondHandleKeyArray.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

#include "ActsEvent/ProtoTrackCollection.h"
#include "ActsEvent/ContextUtility.h"

namespace FPGATrackSim{
    class FPGATrackSimPrototrackFitterAlg: public ::AthReentrantAlgorithm { 
    public: 
    using ::AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~FPGATrackSimPrototrackFitterAlg() = default;

    ///uncomment and implement methods as required

                                            //IS EXECUTED:
    virtual StatusCode  initialize() override final;     //once, before any input is loaded
    virtual StatusCode  execute(const EventContext & ctx) const override final;
    
    private: 
      /** @brief Utility to fetch the geometry, magnetic field and calibration context in the event */
      ActsTrk::ContextUtility m_ctxProvider{this};
      // the track fitter to use for the refit 
      ToolHandle<ActsTrk::IFitterTool> m_actsFitter{this, "ActsFitter", "", "Choice of Acts Fitter (Kalman by default)"};
      // output location to write to 
      SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "", "Output track collection (ActsTrk variant)"};
      // acts helper for the output
      ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper{this};
      // prototrack collection from FPGAClusters or FPGATracks
      SG::ReadHandleKey<ActsTrk::ProtoTrackCollection> m_ProtoTrackCollectionFromFPGAKey{this, "FPGATrackSimActsProtoTracks","","FPGATrackSim PrototrackCollection"};
      // chrono service
      ServiceHandle<IChronoStatSvc> m_chrono{this,"ChronoStatSvc","ChronoStatSvc"};
    }; 

}

#endif //> !ACTSTRACKRECONSTRUCTION_PROTOTRACKCREATIONANDFITALG_H
