/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPATTERNRECOGNITIONALGS_SEGMENTACTSREFITALG_H
#define MUONPATTERNRECOGNITIONALGS_SEGMENTACTSREFITALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "MuonPatternEvent/MuonPatternContainer.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "ActsToolInterfaces/IFitterTool.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "ActsEvent/AuxiliaryMeasurementHandler.h"


namespace CLHEP{
    class HepRandomEngine;
}
/** @brief The SegmentActsRefitAlg is designed to test the Acts tracking geometry  & the global chi2
 *         fitter at chamber level. Uncalibrated xAOD measurements are collected from previously
 *         fitted segments and then passed through the global chi2 fitter. If the fit succeeded,
 *         a new xAOD::MuonSegment is created and the parameters as well as the hit summary are saved.  */
namespace MuonR4{
    class SegmentActsRefitAlg: public AthReentrantAlgorithm {
        public:
          using AthReentrantAlgorithm::AthReentrantAlgorithm;
          virtual StatusCode initialize() override final;
          virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Smear the segment's position and direction by one sigma defined by the
             *         segment's covariance. Returns a tuple of smeared position & direction.
             *  @param gctx: Geometry context to fetch the alignment of the segment
             *  @param segment: Reference to the segment to smear
             *  @param engine: Random engine to pass through the random number sequence */
            std::tuple<Amg::Vector3D, Amg::Vector3D> smearSegment(const ActsTrk::GeometryContext& gctx,
                                                                  const MuonR4::Segment& segment,
                                                                  CLHEP::HepRandomEngine* engine) const;
            /** @brief Declare the data dependency on the standard Mdt+Rpc+Tgc segment container */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_readKey{this, "SegmentContainer", "MuonSegmentsFromR4"};
            /** @brief Declare the key for the refitted segment container */
            SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_writeKey{this, "OutContainer", "ActsRefitSegments"};
            /** @brief  Construct a link from the refitted segment to the input segment. */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_linkKey{this, "LinkKey", m_writeKey, "prefitSegmentLink"};
            /** @brief  Decorate directly the local segment parameters on to the object. */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_localParsKey{this, "LocalParsKey", m_writeKey, "localSegPars"};
            /** @brief Decorate the seed parameters entering the fit */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_seedParsKey{this, "SeedParsKey", m_readKey, "seedSegPars"};
            
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Track fitting tool */
            ToolHandle<ActsTrk::IFitterTool> m_trackFitTool{this, "FittingTool", ""};
            /** @brief Tracking geometry tool */
            PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            /** @brief Segment selection tool to pick the good quality segments */
            ToolHandle<MuonR4::ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
            /** @brief Range service to smear the segment parameters */
            ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};
            /** @brief Smear interval in terms of standard deviations */
            Gaudi::Property<double> m_smearRange{this, "SmearRange", 1.};
            /** @brief Key to setup a surface container for the external constraints */
            SG::WriteHandleKey<xAOD::TrackSurfaceContainer> m_surfKey{this, "SurfaceKey", "RefitSegmentSurf"};
            /** @brief Dump the segment line in obj files */
            Gaudi::Property<bool> m_drawEvent{this , "drawEvent", false };
              
            ActsTrk::AuxiliaryMeasurementHandler m_auxMeasProv{this};
            /** @brief Detector manager to access the spectrometer sector surfaces */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};


    };
}

#endif