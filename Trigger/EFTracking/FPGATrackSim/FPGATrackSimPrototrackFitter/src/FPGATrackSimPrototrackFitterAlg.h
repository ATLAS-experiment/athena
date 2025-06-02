/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_FPGATRACKSIMPROTOTRACKFITTERALG_H
#define ACTSTRACKRECONSTRUCTION_FPGATRACKSIMPROTOTRACKFITTERALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"

#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

#include "ActsEvent/ProtoTrackCollection.h"

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
      // the track fitter to use for the refit 
      ToolHandle<ActsTrk::IFitterTool> m_actsFitter{this, "ActsFitter", "", "Choice of Acts Fitter (Kalman by default)"};
      // tracking geometry - used to translate ATLAS to ACTS geometry
      PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
      // ACTS extrapolation tool - provides the magnetic field 
      ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
      // output location to write to 
      SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "", "Output track collection (ActsTrk variant)"};
      // acts helper for the output
      ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper;
      // prototrack collection from FPGAClusters or FPGATracks
      SG::ReadHandleKey<ActsTrk::ProtoTrackCollection> m_ProtoTrackCollectionFromFPGAKey{this, "FPGATrackSimActsProtoTracks","","FPGATrackSim PrototrackCollection"};
      // chrono service
      ServiceHandle<IChronoStatSvc> m_chrono{this,"ChronoStatSvc","ChronoStatSvc"};
    }; 

}

#endif //> !ACTSTRACKRECONSTRUCTION_PROTOTRACKCREATIONANDFITALG_H
