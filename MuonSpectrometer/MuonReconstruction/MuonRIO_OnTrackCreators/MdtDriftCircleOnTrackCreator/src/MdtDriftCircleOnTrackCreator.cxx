/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtDriftCircleOnTrackCreator.h"

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "MdtCalibData/MdtRtRelation.h"
#include "MuonPrepRawData/MdtPrepData.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/MuonDriftCircleErrorStrategy.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/StraightLineSurface.h"

#include <optional>

namespace Muon {

using MdtRotPtr = MdtDriftCircleOnTrackCreator::MdtRotPtr;
using CalibrationOutput = MdtDriftCircleOnTrackCreator::CalibrationOutput;

MdtDriftCircleOnTrackCreator::MdtDriftCircleOnTrackCreator(const std::string& ty,
                                                           const std::string& na,
                                                           const IInterface* pa)
    : AthAlgTool(ty, na, pa) {
    // algtool interface - necessary!
    declareInterface<IMdtDriftCircleOnTrackCreator>(this);
    declareInterface<IRIO_OnTrackCreator>(this);
}

StatusCode MdtDriftCircleOnTrackCreator::initialize() {
    ATH_CHECK(m_mdtCalibrationTool.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    m_BME_idx = m_idHelperSvc->mdtIdHelper().stationNameIndex("BME");


    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::BroadError, m_createTubeHits);
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::ScaledError, m_scaleMdtCov);
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::FixedError, m_doFixedError);
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::ParameterisedErrors, m_useErrorParametrisation);
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::StationError, m_stationError);
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::ErrorAtPredictedPosition, m_errorAtPredictedPosition);
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::T0Refit, m_t0Refit);
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::Segment, m_doSegments);
    using ToolSettings = IMdtCalibrationTool::ToolSettings;
    using Property = ToolSettings::Property;
    ToolSettings calibSettings = m_mdtCalibrationTool->getSettings();
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::TofCorrection,
                                 calibSettings.isActive(Property::TofCorrection));
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::PropCorrection,
                                 calibSettings.isActive(Property::PropCorrection));
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::TempCorrection,
                                 calibSettings.isActive(Property::TempCorrection));
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::MagFieldCorrection,
                                 calibSettings.isActive(Property::MagFieldCorrection));
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::SlewCorrection,
                                 calibSettings.isActive(Property::SlewCorrection));
    m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::BackgroundCorrection,
                                 calibSettings.isActive(Property::BackgroundCorrection));
    m_errorStrategy.setCalibWindow(calibSettings.window);

    if ("Moore" == m_defaultStrategy) {
        m_errorStrategy.setStrategy(MuonDriftCircleErrorStrategy::Moore);
    } else if ("Muon" == m_defaultStrategy) {
        m_errorStrategy.setStrategy(MuonDriftCircleErrorStrategy::Muon);
    } else {
        // By default use one of the real strategies - don't default to unknown!
        m_errorStrategy.setStrategy(MuonDriftCircleErrorStrategy::Muon);
        ATH_MSG_FATAL("Unknown error strategy "<<m_errorStrategy);
        return StatusCode::FAILURE;
    }
    if (msgLevel(MSG::INFO)) {
        std::stringstream ss;
        ss << "Constructed default MuonDriftCircleErrorStrategy:";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError))
            ss << " Broad";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError))
            ss << " Scaled";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError))
            ss << " Fixed";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::ParameterisedErrors))
            ss << " Parm";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::StationError))
            ss << " Station";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::ErrorAtPredictedPosition))
            ss << " ErrAtPos";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::T0Refit))
            ss << " T0";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::TofCorrection))
            ss << " TOF";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::PropCorrection))
            ss << " Prop";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::TempCorrection))
            ss << " Temp";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::MagFieldCorrection))
            ss << " Mag";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::SlewCorrection))
            ss << " Slew";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::BackgroundCorrection))
            ss << " Back";
        if (m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::Segment))
            ss << " Seg";
        ss << ". ";
        if (!m_isMC && m_looseErrors)
            ss << "Using Data Loose error tuning";
        if (!m_isMC && !m_looseErrors)
            ss << "Using Data Tight error tuning";

        msg(MSG::INFO) << ss.str() << endmsg;
    }
    if (m_isMC)
        ATH_MSG_DEBUG("Using MC error tuning");
    ATH_MSG_VERBOSE("A correction is made if set to true: do_MDT = " << m_doMdt);

    if (m_timeCorrectionType == COSMICS_TOF) {
        if (!m_errorStrategy.creationParameter(MuonDriftCircleErrorStrategy::TofCorrection)) {
            ATH_MSG_ERROR("Detected bad default configuration, using Cosmic TOF without "
                        <<"time of flight corrections does not work");
            return StatusCode::FAILURE;
        }
    }
    ATH_MSG_DEBUG("Timing mode set to  " << m_timeCorrectionType);
    if (m_timeCorrectionType >= NumberOfTimingModes) {
        ATH_MSG_ERROR("Time Correction Type too large! Aborting.");
        return StatusCode::FAILURE;
    }

    if (!m_wasConfigured) {
        ATH_MSG_ERROR("This tool is too complicated to rely on defaults. Potential configuration issue.");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

MdtRotPtr MdtDriftCircleOnTrackCreator::createRIO_OnTrack(const MdtPrepData& mdtPrd,
                                                          const Amg::Vector3D& GP,
                                                          const Amg::Vector3D* GD,
                                                          const double t0Shift,
                                                          const MuonDriftCircleErrorStrategy* strategy,
                                                          const double beta,
                                                          const double tTrack) const {

    const EventContext& ctx{Gaudi::Hive::currentContext()};

    const MuonDriftCircleErrorStrategy& myStrategy{!strategy ? m_errorStrategy : *strategy};

    const Identifier iD = mdtPrd.identify();

    MdtCalibInput calibInput{mdtPrd};
    calibInput.setClosestApproach(GP);
    if (GD) calibInput.setTrackDirection((*GD).unit(), true);

    switch (m_timeCorrectionType) {
        case ATLTIME:
            // normal time of flight corrections assuming IP + light speed
            calibInput.setTimeOfFlight(calibInput.closestApproach().mag() * s_inverseSpeedOfLight);
            ATH_MSG_VERBOSE(" running in ATLTIME mode, tof: " << calibInput.timeOfFlight());
            break;
        case NO_CORRECTIONS:
            // special case for cosmics taken with scintilator trigger or
            // cosmic simulation without TOF
            calibInput.setTimeOfFlight(0);
            ATH_MSG_VERBOSE("running in NO_CORRECTIONS mode, tof: " << calibInput.timeOfFlight());
            break;
        case COSMICS_TRIGGERTIME:
            // special case for cosmics taken with scintilator trigger which
            // is read out so the offset with respect to the clock is known
            // getTriggerTime() NOT IMPLEMENETED YET!!!!!!
            calibInput.setTriggerTime(getTriggerTime());
            ATH_MSG_VERBOSE(" running in COSMICS_TRIGGERTIME mode, triggerOffset: "
                            << calibInput.triggerTime());
            break;
        case COSMICS_TOF:
            calibInput.setTimeOfFlight(timeOfFlight(calibInput.closestApproach(), beta, tTrack, t0Shift));
            ATH_MSG_VERBOSE(" running in COSMICS_TOF mode, tof: "
                            << calibInput.timeOfFlight() << " tTrack: " << tTrack
                            << " t0Shift: " << t0Shift
                            << " applyToF: " << m_applyToF);
            break;
        default:
            // default, no tof. Indicates wrong configuration
            ATH_MSG_WARNING("No valid mode selected, cannot apply tof correction");
            calibInput.setTimeOfFlight(0);
            break;
    }

    Amg::Vector2D posOnWire{Amg::Vector2D::Zero()};
    // if wire sag is taken into account, cast the surface to
    // StraightLineSurface so it can be added to the ROT
    const Trk::Surface& surf{calibInput.legacySurface()};
    std::optional<Amg::Vector2D> posOnIdealWire = surf.globalToLocal(GP, m_globalToLocalTolerance);
    if (!posOnIdealWire) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" globalToLocal() failed for ideal surface");
            return nullptr;
    }
    posOnWire = std::move(*posOnIdealWire);

    double positionAlongWire = posOnWire[Trk::locZ];
    // set driftcirclestatus, NODRIFTTIME if creating tube hits else UNDECIDED
    Trk::DriftCircleStatus dcstatus = m_doMdt ? Trk::UNDECIDED : Trk::NODRIFTTIME;

    CalibrationOutput calibOutput = getLocalMeasurement(ctx, mdtPrd, calibInput, myStrategy);
    // This basically determines the error etc and is where the bulk of the work
    // is done.

    // hack to determine whether we are before or after the spectrum, until we
    // sort this out properly
    if (!calibOutput.calibOk && calibOutput.driftTime > 500.) {
        ATH_MSG_WARNING("Unable to perform calibration ");
        return nullptr;
    }

    std::unique_ptr<MdtDriftCircleOnTrack> rot{};

    // we have to calculate sign, check whether direction is given
    if (m_doMdt && GD) {
        // calculate sign using surface
        const Trk::Surface& surf{calibInput.legacySurface()};
        std::optional<Amg::Vector2D> pos = surf.globalToLocal(GP, calibInput.trackDirection());

        // check this might still fail....
        if (!pos) {
            ATH_MSG_WARNING("Unexpected globalToLocal failure, cannot create MDT ROT ");
            return nullptr;
        }

        // calculate sign
        double sign = (*pos)[Trk::driftRadius] < 0 ? -1.0 : 1.0;
        calibOutput.locPars[Trk::driftRadius] *= sign;
        rot = std::make_unique<MdtDriftCircleOnTrack>(&mdtPrd,
                                                      std::move(calibOutput.locPars),
                                                      std::move(calibOutput.locErr),
                                                      calibOutput.driftTime,
                                                      Trk::DECIDED,
                                                      calibInput.trackDirection(),
                                                      positionAlongWire,
                                                      myStrategy);

    } else {
        // If the track direction is missing, the B-field correction was not
        // applied
        if (GD) {
            // do not have access to direction, so have to use partial
            // constructor:
            rot = std::make_unique<MdtDriftCircleOnTrack>(&mdtPrd,
                                                          std::move(calibOutput.locPars),
                                                          std::move(calibOutput.locErr),
                                                          calibOutput.driftTime,
                                                          dcstatus,
                                                          positionAlongWire,
                                                          myStrategy);
        } else {
            MuonDriftCircleErrorStrategy tmpStrategy(myStrategy.getBits());
            tmpStrategy.setParameter(MuonDriftCircleErrorStrategy::MagFieldCorrection, false);
            rot = std::make_unique<MdtDriftCircleOnTrack>(&mdtPrd,
                                                          std::move(calibOutput.locPars),
                                                          std::move(calibOutput.locErr),
                                                          calibOutput.driftTime,
                                                          dcstatus,
                                                          positionAlongWire,
                                                          tmpStrategy);
        }
    }
    ATH_MSG_DEBUG("MDT ROT: radius = "
                  << rot->localParameters().get(Trk::driftRadius) << " error = "
                  << Amg::error(rot->localCovariance(), Trk::locR)
                  <<" ,channel "
                  << m_idHelperSvc->toString(iD));

    return rot.release();
}

void MdtDriftCircleOnTrackCreator::updateSign(MdtDriftCircleOnTrack& caliDriftCircle,
                                              Trk::DriftCircleSide si) const {
    // ************************
    // Apply additional corrections to local position
    Trk::LocalParameters lpos(caliDriftCircle.localParameters());

    // set sign LocalPosition
    if (si == Trk::LEFT) {
        lpos[Trk::driftRadius] = -std::abs(lpos[Trk::driftRadius]);
    } else {
        lpos[Trk::driftRadius] = std::abs(lpos[Trk::driftRadius]);
    }

    caliDriftCircle.setLocalParameters(lpos);
    caliDriftCircle.m_status = Trk::DECIDED;
    ATH_MSG_VERBOSE("MDT DriftCircleOnTrack made with radius = " << lpos);
}

CalibrationOutput MdtDriftCircleOnTrackCreator::getLocalMeasurement(const EventContext& ctx,
                                                                    const MdtPrepData& DC,
                                                                    const MdtCalibInput& calibInput,
                                                                    const MuonDriftCircleErrorStrategy& myStrategy) const {


    ATH_MSG_VERBOSE("getLocalMeasurement "<<calibInput<<" with m_doMdt=" << m_doMdt << " and " << myStrategy);
    const Amg::Vector3D& gpos{calibInput.closestApproach()};
    const Amg::Vector3D& gdir{calibInput.trackDirection()};

    double sigmaR{1.}, driftTime{0.}, radius{0.}, errRadius{0.};
    MdtCalibOutput calibOutput{};
    if (m_doMdt) {
        // call the calibration service providing the time when the particle
        // passed the tube
        calibOutput = m_mdtCalibrationTool->calibrate(ctx, calibInput);
        ATH_MSG_VERBOSE("getLocalMeasurement() - Calibrated output "<<calibOutput);
        driftTime = calibOutput.driftTime();
        radius = calibOutput.driftRadius(); /// copy new values
        errRadius = radius;                 /// Use same value

        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ErrorAtPredictedPosition)) {
            const Trk::Surface& surf{calibInput.legacySurface()};
            std::optional<Amg::Vector2D> myLocalPosition = surf.globalToLocal(gpos, gdir);
            if (myLocalPosition) {
                errRadius = (*myLocalPosition)[Trk::driftRadius];
            } else {
                ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" ErrorAtPredictedPosition failed because local position "<<
                                    "transformation didn't succeed. Using measured radius instead.");
                errRadius = radius;
            }
        }
    } else {
        // use PRD values
        radius = DC.localPosition().x();
        errRadius = radius;
        // check consistency of error matrix
        if (DC.localCovariance().cols() > 1) {
            ATH_MSG_WARNING("Error matrix of DC doesn't have dimension 1 "<< DC.localCovariance());
            ATH_MSG_WARNING("Reducing size to 1 dim");
        }
    }

    // Handle the errors here.  Begin by getting the first part of the
    // resolution term
    if (!m_doMdt) {
        sigmaR = Amg::error(DC.localCovariance(), Trk::driftRadius);
    } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ParameterisedErrors)) {
        sigmaR = parametrisedSigma(errRadius);
    } else {
        sigmaR = calibOutput.driftRadiusUncert();
    }
    ATH_MSG_DEBUG("Tube : " << m_idHelperSvc->toString(calibInput.identify())
                            << " SigmaR = " << sigmaR);
    double sigmaR2 = 0.0;
    // Handle the errors scaling / addition of fixed terms
    if (myStrategy.strategy() == MuonDriftCircleErrorStrategy::Moore) {
        sigmaR2 = mooreErrorStrategy(myStrategy, sigmaR * sigmaR, calibInput.identify());
        ATH_MSG_DEBUG("After scaling etc:\t Moore sigmaR2 = " << sigmaR2);
    } else if (myStrategy.strategy() == MuonDriftCircleErrorStrategy::Muon) {
        sigmaR2 = muonErrorStrategy(myStrategy, sigmaR * sigmaR, calibInput.identify());
        ATH_MSG_DEBUG("After scaling etc:\t Muon ErrorStrategy sigmaR2 = " << sigmaR2);
    }
    //////////////////////////////////////////////////////////////
    // update or copy drift radius and error

    // new radius
    Trk::DefinedParameter radiusPar(radius, Trk::driftRadius);

    // create new error matrix
    Amg::MatrixX newLocalCov(1, 1);
    newLocalCov(0, 0) = sigmaR2;

    // return new values
    bool ok = true;
    return {Trk::LocalParameters(radiusPar), newLocalCov, driftTime,
            ok};
}

MdtRotPtr MdtDriftCircleOnTrackCreator::correct(const MdtPrepData& prd,
                                                const Trk::TrackParameters& tp,
                                                const MuonDriftCircleErrorStrategy* strategy,
                                                const double beta,
                                                const double tTrack) const {

    const Amg::Vector3D& momentum = tp.momentum();
    return createRIO_OnTrack(prd, tp.position(), &momentum, 0, strategy, beta, tTrack);
}

Trk::RIO_OnTrack* MdtDriftCircleOnTrackCreator::correct(const Trk::PrepRawData& prd,
                                                        const Trk::TrackParameters& tp,
                                                        const EventContext& /*ctx*/) const {
    if (!prd.type(Trk::PrepRawDataType::MdtPrepData)){
        ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Incorrect hit type: "
                      <<" Trk::PrepRawData not a MdtPrepData!! No rot created ");
        return nullptr;
    }
    Amg::Vector3D momentum = tp.momentum();

    return createRIO_OnTrack(static_cast<const MdtPrepData&>(prd), tp.position(), &momentum);
}
MdtRotPtr MdtDriftCircleOnTrackCreator::updateError(const MdtDriftCircleOnTrack& DCT,
                                                    const Trk::TrackParameters* /**pars*/,
                                                    const MuonDriftCircleErrorStrategy* strategy) const {

    const MuonDriftCircleErrorStrategy& myStrategy = (!strategy) ? m_errorStrategy : *strategy;

    // calculate error
    double sigmaR(1.);
    double t = DCT.driftTime();

    const MuonGM::MdtReadoutElement* detEl = DCT.detectorElement();
    if (!detEl) {
        ATH_MSG_WARNING("MdtDriftCircleOnTrack without MdtReadoutElement "
                        << m_idHelperSvc->toString(DCT.identify()));
        return nullptr;
    }

    double radius = DCT.driftRadius();

    if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError) &&
        !myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError) &&
        !myStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError)) {
        sigmaR = detEl->innerTubeRadius() / std::sqrt(3.0);  // Tube hit
    } else {
        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ErrorAtPredictedPosition))
            ATH_MSG_WARNING("updateError: ErrorAtPredictedPosition is not yet supported!");
        // get error
        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ParameterisedErrors)) {
            if (myStrategy.strategy() == MuonDriftCircleErrorStrategy::Moore) {
                sigmaR = parametrisedSigma(radius);
            } else if (myStrategy.strategy() == MuonDriftCircleErrorStrategy::Muon) {
                sigmaR = parametrisedSigma(radius);
                ATH_MSG_DEBUG("Muon drift errors:" << sigmaR
                                                   << " for radius=" << radius);
            }
        } else {
            const EventContext& ctx{Gaudi::Hive::currentContext()};
            sigmaR = m_mdtCalibrationTool->getResolutionFromRt(ctx,
                                                               DCT.identify(),
                                                               DCT.driftTime());
            if (sigmaR < 0.0001 || sigmaR * sigmaR < 0.0001) {
                ATH_MSG_WARNING("Bad obtained from calibration service: error "
                                << m_idHelperSvc->toString(DCT.identify())
                                << " reso " << sigmaR << " sigma2 "
                                << sigmaR * sigmaR << " drift time " << t
                                << " original " << DCT.driftTime());
                return nullptr;
            }
        }

    }  // end of tube hit check

    double sigmaR2 = 0.0;
    // Handle the errors scaling / addition of fixed terms
    if (myStrategy.strategy() == MuonDriftCircleErrorStrategy::Moore) {
        sigmaR2 = mooreErrorStrategy(myStrategy, sigmaR * sigmaR, DCT.identify());
    } else if (myStrategy.strategy() == MuonDriftCircleErrorStrategy::Muon) {
        sigmaR2 =  muonErrorStrategy(myStrategy, sigmaR * sigmaR, DCT.identify());
        ATH_MSG_DEBUG("After scaling etc:\t Muon ErrorStrategy sigmaR2 = " << sigmaR2);
    }
    std::unique_ptr<MdtDriftCircleOnTrack> rot{DCT.clone()};
    rot->m_localCovariance(0, 0) = sigmaR2;
    rot->setErrorStrategy(myStrategy);

    ATH_MSG_VERBOSE("updated error for "
                    << m_idHelperSvc->toString(DCT.identify()) << " new error "
                    << Amg::error(rot->localCovariance(), Trk::locR)
                    << " old error "
                    << Amg::error(DCT.localCovariance(), Trk::locR));
    return rot.release();
}

double MdtDriftCircleOnTrackCreator::timeOfFlight(const Amg::Vector3D& pos,
                                                  const double beta,
                                                  const double tTrack,
                                                  const double tShift) const {
    return m_applyToF * (pos.mag() * s_inverseSpeedOfLight / beta) +
           tTrack + tShift;
}

double MdtDriftCircleOnTrackCreator::parametrisedSigma(double r) {
    /// These are presumably fitted from data but no clue where and how
    return 0.23 * std::exp(-std::abs(r) / 6.06) + 0.0362;
}

double MdtDriftCircleOnTrackCreator::mooreErrorStrategy(const MuonDriftCircleErrorStrategy& myStrategy,
                                                        double sigmaR2,
                                                        const Identifier& id) const {
    if (m_isMC)
        return mooreErrorStrategyMC(myStrategy, sigmaR2, id);

    if (m_looseErrors)
        return mooreErrorStrategyLoose(myStrategy, sigmaR2, id);
    else
        return mooreErrorStrategyTight(myStrategy, sigmaR2, id);

}

double MdtDriftCircleOnTrackCreator::mooreErrorStrategyMC(const MuonDriftCircleErrorStrategy& myStrategy,
                                                          double sigmaR2,
                                                          const Identifier& id) const {
    ATH_MSG_DEBUG("mooreErrorStrategy sigmaR2=" << sigmaR2<<" "<<m_idHelperSvc->toString(id));

    // Moore error strategy.  Hard coding numbers for the time being - hope to
    // make them configurable some day
    if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::Segment)) {
        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError) &&
            myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError)) {
            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::T0Refit)) {
                ATH_MSG_VERBOSE(" segment error, t0fit ");
                return sigmaR2 + 0.005;  // Collisions with T0 refit (input)
            } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" segment error, broad ");
                return 4 * sigmaR2 + 0.16;  // Output segments - broad errors
            } else {
                ATH_MSG_VERBOSE(" segment error, precise ");
                return sigmaR2 + 0.005;  // Input segments , no T0 refit
            }
        }
        // Don't know how to handle other cases - error?
    } else {  // Track
        
        using namespace MuonStationIndex;
        const StIndex stIdx = m_idHelperSvc->stationIndex(id);
        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::StationError)) {
            if (stIdx == StIndex::BE) {
                ATH_MSG_VERBOSE(" track error BEE ");
                return 1.44 * sigmaR2 + 1.44;  // 1.2* + 1.2 mm
            } else if (stIdx == StIndex::EE) {
                ATH_MSG_VERBOSE(" track error EE ");
                if (!m_isMC && m_idHelperSvc->stationEta(id) < 0)
                    return 1.44 * sigmaR2 + 0.16;  // 1.2* + 0.4 mm
                return 1.44 * sigmaR2 + 1.;        // 1.2* + 1. mm
            } else if (m_idHelperSvc->chamberIndex(id) == ChIndex::BIS &&
                       std::abs(m_idHelperSvc->stationEta(id)) >= 7) {
                ATH_MSG_VERBOSE(" track error BIS78 ");
                if (std::abs(m_idHelperSvc->stationEta(id)) == 7)
                    return 1.44 * sigmaR2 + 1.;  // 1.2* + 1. mm
                else
                    return 4 * sigmaR2 + 25;  // 2* + 5. mm
            }
            ATH_MSG_VERBOSE(" track station error  ");
            return 1.44 * sigmaR2 + 1.;  // 1.2* + 1. mm

        } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError)) {

            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" track error Fixed/Broad ");
                return 4 * sigmaR2 + 49.;  // 2* + 7 mm -> barrel/endcap overlaps
            } else {
                ATH_MSG_VERBOSE(" track error Fixed ");
                return 4 * sigmaR2 + 4.;  // 2* + 2mm S/L overlaps
            }

        } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError)) {

            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" track error Scaled/Broad ");
                return 2.25 * sigmaR2 + 0.09;
            } else {
                // use slightly smaller errors for the barrel
                double fixedTerm = (stIdx == StIndex::BI || stIdx == StIndex::BM || stIdx == StIndex::BO)
                                 ? 0.014 : 0.04;
                if (m_doIndividualChamberReweights &&
                    m_idHelperSvc->chamberIndex(id) == ChIndex::BIL &&
                    m_idHelperSvc->stationEta(id) == 1 &&
                    m_idHelperSvc->sector(id) == 13 &&
                    m_idHelperSvc->mdtIdHelper().multilayer(id) == 1) {
                    fixedTerm = 1;
                    ATH_MSG_VERBOSE(" track error Scaled: BIL1A13, first multi layer ");
                } else {
                    ATH_MSG_VERBOSE(" track error Scaled ");
                }
                return 1.44 * sigmaR2 + fixedTerm;
            }
        }
    }  // End of segment or track
    return sigmaR2;
}

double MdtDriftCircleOnTrackCreator::mooreErrorStrategyLoose(const MuonDriftCircleErrorStrategy& myStrategy,
                                                             double sigmaR2,
                                                             const Identifier& id) const {
    ATH_MSG_DEBUG("mooreErrorStrategy sigmaR2=" << sigmaR2<<" "<<m_idHelperSvc->toString(id));

    // Moore error strategy.  Hard coding numbers for the time being - hope to
    // make them configurable some day
    if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::Segment)) {
        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError) &&
            myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError)) {
            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::T0Refit)) {
                ATH_MSG_VERBOSE(" segment error, t0fit ");
                return sigmaR2 + 0.005;  // Collisions with T0 refit (input)
            } else if (myStrategy.creationParameter(
                           MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" segment error, broad ");
                return 4 * sigmaR2 + 0.16;  // Output segments - broad errors
            } else {
                ATH_MSG_VERBOSE(" segment error, precise ");
                return sigmaR2 + 0.005;  // Input segments , no T0 refit
            }
        }
        // Don't know how to handle other cases - error?
    } else {  // Track
        using namespace MuonStationIndex;
        StIndex stIdx = m_idHelperSvc->stationIndex(id);
        if (myStrategy.creationParameter(
                MuonDriftCircleErrorStrategy::StationError)) {
            if (stIdx == StIndex::BE) {
                ATH_MSG_VERBOSE(" track error BEE ");
                return 1.44 * sigmaR2 + 4;  // 1.2* + 2 mm
            } else if (stIdx == StIndex::EE) {
                ATH_MSG_VERBOSE(" track error EE ");
                return 1.44 * sigmaR2 + 0.04;  // 1.2* + 0.2 mm
            } else if (m_idHelperSvc->chamberIndex(id) == ChIndex::BIS &&
                       std::abs(m_idHelperSvc->stationEta(id)) >= 7) {
                ATH_MSG_VERBOSE(" track error BIS78 ");
                if (std::abs(m_idHelperSvc->stationEta(id)) == 7)
                    return 1.44 * sigmaR2 + 1.;  // 1.2* + 1. mm
                return 4 * sigmaR2 + 25;  // 2* + 5. mm
            } else if (m_idHelperSvc->mdtIdHelper().stationName(id) == m_BME_idx &&
                       m_idHelperSvc->stationPhi(id) == 7) {
                ATH_MSG_VERBOSE(" track error BME ");
                return 1.44 * sigmaR2 + 0.25;  // 1.2* + 0.5 mm
            }
            /// Need to check whether this Identifier is still existent
            else if (m_idHelperSvc->chamberIndex(id) == ChIndex::BOL && std::abs(m_idHelperSvc->stationEta(id)) == 7 &&
                     m_idHelperSvc->stationPhi(id) == 7) {
                ATH_MSG_VERBOSE(" track error BOE ");
                return 1.44 * sigmaR2 + 0.25;  // 1.2* + 0.5 mm
            }
            ATH_MSG_VERBOSE(" track station error  ");
            return 1.44 * sigmaR2 + 0.04;  // 1.2* + 0.2 mm

        } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError)) {

            if (myStrategy.creationParameter(
                    MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" track error Fixed/Broad ");
                return 4 * sigmaR2 + 4.;  // 2* + 2 mm -> barrel/endcap overlaps
            } else {
                ATH_MSG_VERBOSE(" track error Fixed ");
                return 4 * sigmaR2 + 4.;  // 2* + 2mm S/L overlaps
            }

        } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError)) {

            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" track error Scaled/Broad ");
                return 2.25 * sigmaR2 + 0.09;
            } else {
                // use slightly smaller errors for the barrel
                double fixedTerm = (stIdx == StIndex::BI || stIdx == StIndex::BM || stIdx == StIndex::BO)
                                 ? 0.015 : 0.015;
                if (m_doIndividualChamberReweights &&
                    m_idHelperSvc->chamberIndex(id) == ChIndex::BIL && m_idHelperSvc->stationEta(id) == 1 &&
                    m_idHelperSvc->sector(id) == 13 && m_idHelperSvc->mdtIdHelper().multilayer(id) == 1) {
                    fixedTerm = 1;
                    ATH_MSG_VERBOSE(" track error Scaled: BIL1A13, first multi layer ");
                } else {
                    ATH_MSG_VERBOSE(" track error Scaled ");
                }

                return 1.44 * sigmaR2 + fixedTerm;
            }
        }
    }  // End of segment or track
    return sigmaR2;
}

double MdtDriftCircleOnTrackCreator::mooreErrorStrategyTight(const MuonDriftCircleErrorStrategy& myStrategy,
                                                             double sigmaR2,
                                                             const Identifier& id) const {
    ATH_MSG_DEBUG("mooreErrorStrategy sigmaR2=" << sigmaR2);

    // Moore error strategy.  Hard coding numbers for the time being - hope to
    // make them configurable some day
    if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::Segment)) {
        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError) &&
            myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError)) {
            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::T0Refit)) {
                ATH_MSG_VERBOSE(" segment error, t0fit ");
                return sigmaR2 + 0.005;  // Collisions with T0 refit (input)
            } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" segment error, broad ");
                return 4 * sigmaR2 + 0.16;  // Output segments - broad errors
            } else {
                ATH_MSG_VERBOSE(" segment error, precise ");
                return sigmaR2 + 0.005;  // Input segments , no T0 refit
            }
        }
        // Don't know how to handle other cases - error?
    } else {  // Track
        using namespace MuonStationIndex;
        StIndex stIdx = m_idHelperSvc->stationIndex(id);
        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::StationError)) {
            if (stIdx == StIndex::BE) {
                ATH_MSG_VERBOSE(" track error BEE ");
                return 1.44 * sigmaR2 + 0.04;  // 1.2* + 0.2 mm
            } else if (stIdx == StIndex::EE) {
                ATH_MSG_VERBOSE(" track error EE ");
                if (m_idHelperSvc->isSmallChamber(id))
                    return 1.21 * sigmaR2 + 0.01;  // 1.1* + 0.1 mm
                else
                    return 1.21 * sigmaR2 + 0.01;  // 1.1* + 0.1 mm
            } else if (m_idHelperSvc->chamberIndex(id) == ChIndex::BIS &&
                       std::abs(m_idHelperSvc->stationEta(id)) >= 7) {
                ATH_MSG_VERBOSE(" track error BIS78 ");
                if (std::abs(m_idHelperSvc->stationEta(id)) == 7)
                    return 1.44 * sigmaR2 + 1.;  // 1.2* + 1. mm

                return 4 * sigmaR2 + 1.;  // 2* + 1. mm
            } else if (stIdx == StIndex::BM &&
                       m_idHelperSvc->stationPhi(id) == 7 &&
                       (m_idHelperSvc->mdtIdHelper()).stationName(id) == m_BME_idx) {
                ATH_MSG_VERBOSE(" track error BME ");
                return 1.21 * sigmaR2 + 0.25;  // 1.1* + 0.5 mm
            }
            /// Need to check whether this Identifier is still valid?
            else if (m_idHelperSvc->chamberIndex(id) == ChIndex::BOL && std::abs(m_idHelperSvc->stationEta(id)) == 7 &&
                    m_idHelperSvc->stationPhi(id) == 7) {
                ATH_MSG_VERBOSE(" track error BOE ");
                return 1.21 * sigmaR2 + 0.25;  // 1.1* + 0.5 mm
            } else if (m_idHelperSvc->chamberIndex(id) == ChIndex::EEL &&  m_idHelperSvc->stationEta(id) < 0 &&
                       m_idHelperSvc->stationPhi(id) == 3) {
                ATH_MSG_VERBOSE(" track error EEL1C05 ");
                return 1.21 * sigmaR2 + 25.;  // 1.1* + 5 mm
            }
            ATH_MSG_VERBOSE(" track station error  ");
            return 1.21 * sigmaR2 + 0.04;  // 1.1* + 0.2 mm

        } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError)) {

            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" track error Fixed/Broad ");
                return 4 * sigmaR2 + 4.;  // 2* + 2 mm -> barrel/endcap overlaps
            } else {
                ATH_MSG_VERBOSE(" track error Fixed ");
                return 4 * sigmaR2 + 4.;  // 2* + 2mm S/L overlaps
            }

        } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError)) {

            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" track error Scaled/Broad ");
                return 2.25 * sigmaR2 + 0.09;
            } else {

                // use slightly smaller errors for the barrel
                //
                double fixedTerm = 0.01;
                if (m_doIndividualChamberReweights) {
                    if (m_idHelperSvc->chamberIndex(id) == ChIndex::BIL && m_idHelperSvc->stationEta(id) == 1 &&
                        m_idHelperSvc->sector(id) == 13 && m_idHelperSvc->mdtIdHelper().multilayer(id) == 1) {
                        fixedTerm = 1;
                        ATH_MSG_VERBOSE(" track error Scaled: BIL1A13, first multi layer ");
                    }
                } else {
                    ATH_MSG_VERBOSE(" track error Scaled ");
                }

                return 1.21 * sigmaR2 + fixedTerm;
            }
        }
    }  // End of segment or track
    return sigmaR2;
}

double MdtDriftCircleOnTrackCreator::muonErrorStrategy(const MuonDriftCircleErrorStrategy& myStrategy,
                                                       double sigmaR2,
                                                       const Identifier& /*id*/) const {

    //
    //   the new muonErrorStrategy is identical for Data and MC
    //   it assumes that for tracks the alignment uncertainties are added later
    //   this is done by the AignmentErrorTool where AlignmentEffectsOnTrack are
    //   used. (for segment errors the mooreStrategy is coded)
    //
    //   it is inspired by the mooreErrorStrategyTight but does not need a constant term
    //

    ATH_MSG_DEBUG("muonErrorStrategy sigmaR2=" << sigmaR2);

    // Muon error strategy.  Hard coding numbers for the time being - hope to
    // make them configurable some day
    if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::Segment)) {
        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError) &&
            myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError)) {
            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::T0Refit)) {
                ATH_MSG_VERBOSE(" segment error, t0fit ");
                return sigmaR2 + 0.005;  // Collisions with T0 refit (input)
            } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" segment error, broad ");
                return 4 * sigmaR2 + 0.16;  // Output segments - broad errors
            } else {
                ATH_MSG_VERBOSE(" segment error, precise ");
                return sigmaR2 + 0.005;  // Input segments , no T0 refit
            }
        }
    } else {  // Track
        if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::StationError)) {
            ATH_MSG_VERBOSE(" track station error  ");
            return 1.21 * sigmaR2;  // 1.1* mm
        } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::FixedError)) {
            ATH_MSG_VERBOSE(" track error Fixed ");
            return 4 * sigmaR2;  // 2*
        } else if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::ScaledError)) {
            if (myStrategy.creationParameter(MuonDriftCircleErrorStrategy::BroadError)) {
                ATH_MSG_VERBOSE(" track error Scaled/Broad ");
                return 2.25 * sigmaR2;
            } else {
                ATH_MSG_VERBOSE(" Track scaled error ");
                return 1.21 * sigmaR2;
            }
        }
    }  // End of segment or track
    return sigmaR2;
}
}
