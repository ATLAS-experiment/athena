/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPATTERNRECOGNITIONALGS_SEGMENTACTSREFITALG_H
#define MUONPATTERNRECOGNITIONALGS_SEGMENTACTSREFITALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"


#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonPatternEvent/MuonPatternContainer.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/AuxiliaryMeasurementHandler.h"

#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"


#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"
#include "Acts/TrackFitting/GlobalChiSquareFitter.hpp"

#include "ActsEvent/ContextUtility.h"
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


          /// Type erased track fitter function.
          using Propagator_t = Acts::Propagator<Acts::StraightLineStepper, Acts::Navigator>;
          using Fitter_t = Acts::Experimental::Gx2Fitter<Propagator_t, ActsTrk::MutableTrackStateBackend>;

          /** @brief Abbrivation of the configuration to launch the fit  */
          using Gx2FitterOptions_t = Acts::Experimental::Gx2FitterOptions<ActsTrk::MutableTrackStateBackend>;
          /** @brief Abbrivation of the fitter extensions */
          using Gx2FitterExtension_t = Acts::Experimental::Gx2FitterExtensions<ActsTrk::MutableTrackStateBackend>;

        private:
            /** @brief Returns the entrance / exit portal surface of the tracking volume
             *         associated with the measurement surface
             * @param measurement: Reference to the measurement which portal is to be fetched
             * @param entrance: Flag toggling whether the entrance or exit portal shall be returned */
            const Acts::Surface* portalSurface(const xAOD::UncalibratedMeasurement* measurement,
                                               bool entrance) const;
            /** @brief Smear the segment's position and direction by one sigma defined by the
             *         segment's covariance. Returns a tuple of smeared position & direction.
             *  @param gctx: Geometry context to fetch the alignment of the segment
             *  @param segment: Reference to the segment to smear
             *  @param engine: Random engine to pass through the random number sequence */
            std::tuple<Amg::Vector3D, Amg::Vector3D> smearSegment(const Acts::GeometryContext& gctx,
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
            /** @brief Tracking geometry tool */
           ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
            /** @brief Auxiliary class to access the magnetic field, geometry and calibration context */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Segment selection tool to pick the good quality segments */
            ToolHandle<MuonR4::ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
            /** @brief Range service to smear the segment parameters */
            ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};
            /** @brief Smear interval in terms of standard deviations */
            Gaudi::Property<double> m_smearRange{this, "SmearRange", 1.};
            /// Handle to the space point calibrator
            ToolHandle<ISpacePointCalibrator> m_calibTool{this, "Calibrator", "" };
            /** @brief Dump the segment line in obj files */
            Gaudi::Property<bool> m_drawEvent{this , "drawEvent", false };
              
            ActsTrk::AuxiliaryMeasurementHandler m_auxMeasProv{this};
            /** @brief Detector manager to access the spectrometer sector surfaces */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            Gaudi::Property<bool> m_smearSegPars{this, "smearSegPars", true};
            /** @brief Maximum number of propagation steps */
            Gaudi::Property<unsigned> m_maxPropSteps{this,"maxPropagationSteps", 100000};
            /** @brief Maximum number of target surfaces */
            Gaudi::Property<unsigned> m_maxTargetSurfSkip{this, "maxTargetSurfSkip", 100000};
            /** @brief Maximum number of iterations */
            Gaudi::Property<unsigned> m_maxIter{this, "maxIter", 50};
            /** @brief Free to bound Jacobian correction */
            Gaudi::Property<bool> m_doJacobianCorr{this,"freeToBoundJacobian", true};

            /** @brief Surface accessor delegate for xAOD::UncalibratedMeasurement objects */
            ActsTrk::detail::xAODUncalibMeasSurfAcc m_surfAccessor{};
            /** @brief Fitter setup */
            Gx2FitterExtension_t m_fitExtension{};

            std::unique_ptr<Fitter_t> m_fitter{};



    };
}

#endif