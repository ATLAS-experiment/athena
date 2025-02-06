/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibrationTool.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "MdtCalibData/MdtRtRelation.h"
#include "MdtCalibData/MdtTubeCalibContainer.h"
#include "MdtCalibData/MdtCorFuncSet.h"
#include "MdtCalibData/IMdtBFieldCorFunc.h"
#include "MdtCalibData/IMdtSlewCorFunc.h"
#include "MdtCalibData/IMdtTempCorFunc.h"
#include "MdtCalibData/IMdtBackgroundCorFunc.h"
#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/RtScaleFunction.h"
#include "MagFieldElements/AtlasFieldCache.h"
#include "MuonCalibEvent/MdtCalibHit.h"

#include "GeoModelKernel/throwExcept.h"
namespace {
  static double const twoBySqrt12 = 2/std::sqrt(12);
}

using SingleTubeCalib = MuonCalib::MdtTubeCalibContainer::SingleTubeCalib;
using MdtDriftCircleStatus = MdtCalibOutput::MdtDriftCircleStatus;
using ToolSettings = MdtCalibrationTool::ToolSettings;


ToolSettings MdtCalibrationTool::getSettings() const {
    ToolSettings settings{};
    using Property = ToolSettings::Property;
    settings.setBit(Property::TofCorrection, m_doTof);
    settings.setBit(Property::PropCorrection, m_doProp);
    settings.setBit(Property::TempCorrection, m_doTemp);
    settings.setBit(Property::MagFieldCorrection, m_doField);
    settings.setBit(Property::SlewCorrection, m_doSlew);
    settings.setBit(Property::BackgroundCorrection, m_doBkg);
    settings.window = static_cast<timeWindowMode>(m_windowSetting.value()); 
    return settings;
}
StatusCode MdtCalibrationTool::initialize() {  
  ATH_MSG_DEBUG( "Initializing" );

  switch(m_windowSetting.value()) {
    case timeWindowMode::UserDefined:
       ATH_MSG_DEBUG("Use predefined user values of "<<m_timeWindowLowerBound<<" & "<<m_timeWindowUpperBound);
       break;
    case timeWindowMode::Default:
      ATH_MSG_DEBUG("Use 1000. & 2000. as the lower and upper time window values ");
      m_timeWindowLowerBound = 1000.;
      m_timeWindowUpperBound = 2000.;
      break;
    case timeWindowMode::CollisionG4:
       ATH_MSG_DEBUG("Use Geant4 collision time window of 20-30");
       m_timeWindowLowerBound = 20.;
       m_timeWindowUpperBound = 30.;
       break;
    case timeWindowMode::CollisionData:
        ATH_MSG_DEBUG("Use collision data time window of 10 to 30");
        m_timeWindowLowerBound = 10.;
        m_timeWindowUpperBound = 30.;
        break;
    case timeWindowMode::CollisionFitT0:
        ATH_MSG_DEBUG("Use collision data time window of 50 to 100 to fit T0 in the end");
        m_timeWindowLowerBound = 50.;
        m_timeWindowUpperBound = 100.;
        break;
    default:
       ATH_MSG_FATAL("Unknown time window setting "<<m_windowSetting<<" provided.");
       return StatusCode::FAILURE;
  };

  ATH_CHECK(m_idHelperSvc.retrieve());
  /// Ensure that the conditions dependency is properly declared  
  ATH_CHECK(m_fieldCacheCondObjInputKey.initialize());
  ATH_CHECK(m_calibDbKey.initialize());
  /// Shifting tools to evaluate systematic uncertainties on the T0 timing
  ATH_CHECK(m_t0ShiftTool.retrieve(EnableTool{m_doT0Shift}));
  ATH_CHECK(m_tMaxShiftTool.retrieve(EnableTool{m_doTMaxShift}));
  ATH_MSG_DEBUG("Initialization finalized "<<std::endl
                <<"  TimeWindow: ["<< m_timeWindowLowerBound.value()<<";"<<m_timeWindowUpperBound.value()<<"]"<<std::endl
                <<"   Correct time of flight "<<(m_doTof ? "yay" : "nay")<<std::endl
                <<"   Correct propagation time "<<(m_doProp ? "si" : "no")<<std::endl
                <<"   Correct temperature "<<(m_doTemp ? "si" : "no")<<std::endl
                <<"   Correct magnetic field "<<(m_doField ? "si" : "no")<<std::endl
                <<"   Correct time slew "<<(m_doSlew ? "si" : "no")<<std::endl
                <<"   Correct background "<<(m_doBkg ? "si" : "no"));
  return StatusCode::SUCCESS;
}  //end MdtCalibrationTool::initialize


const MuonCalib::MdtFullCalibData* MdtCalibrationTool::getCalibConstants(const EventContext& ctx,
                                                                         const Identifier& channelId) const {
    SG::ReadCondHandle constantHandle{m_calibDbKey, ctx};
    if (!constantHandle.isValid()){
         THROW_EXCEPTION("Failed to retrieve the Mdt calibration constants "<<m_calibDbKey.fullKey());
    }
    return constantHandle->getCalibData(channelId, msgStream());
}
MdtCalibOutput MdtCalibrationTool::calibrate(const EventContext& ctx, 
                                             const MdtCalibInput& calibIn,
                                             bool resolFromRtrack) const {
  
  const Identifier& id{calibIn.identify()};
  
  SG::ReadCondHandle constantHandle{m_calibDbKey, ctx};
  if (!constantHandle.isValid()){
      THROW_EXCEPTION("Failed to retrieve the Mdt calibration constants "<<m_calibDbKey.fullKey());
  }

  const MuonCalib::MdtFullCalibData* calibConstants = constantHandle->getCalibData(id, msgStream());
  if (!calibConstants) {
     ATH_MSG_WARNING("Could not find calibration data for channel "<<m_idHelperSvc->toString(id));
     return MdtCalibOutput{};
  }
 
  // require at least the MdtRtRelation to be available
  const RtRelationPtr& rtRelation{calibConstants->rtRelation};
  // Hardcoded MDT tube radius 14.6mm here - not correct for sMDT
  // on the other hand it should be rare that a tube does not have an RT
  if(!rtRelation) {
    ATH_MSG_WARNING("No rtRelation found, cannot calibrate tube "<<m_idHelperSvc->toString(id));
    return MdtCalibOutput{};
  }
  if (!calibConstants->tubeCalib) {
     ATH_MSG_WARNING("Cannot extract the single tube calibrations for tube "<<m_idHelperSvc->toString(id));
     return MdtCalibOutput{};
  }
  /// Retrieve the constants for the specific tube
  const SingleTubeCalib* singleTubeData = calibConstants->tubeCalib->getCalib(id);
  if (!singleTubeData) {
     ATH_MSG_WARNING("Failed to access tubedata for " << m_idHelperSvc->toString(id));
     return MdtCalibOutput{};
  }
  const float invPropSpeed = constantHandle->inversePropSpeed();
  
  MdtCalibOutput calibResult{};
  // correct for global t0 of rt-region
  
  calibResult.setTubeT0(singleTubeData->t0);
  calibResult.setMeanAdc(singleTubeData->adcCal);

  // set propagation delay
  if (m_doProp) {   
    const double propagationDistance = calibIn.signalPropagationDistance(); 
    ATH_MSG_VERBOSE("Calibration of "<<m_idHelperSvc->toString(id)<<", propagation distance: "<<propagationDistance<<" -> "
                  <<(invPropSpeed * propagationDistance));
    calibResult.setPropagationTime(invPropSpeed * propagationDistance);
  }
  
  /// calculate drift time
  const double driftTime = calibIn.tdc() * tdcBinSize 
                         - (m_doTof ? calibIn.timeOfFlight() : 0.)
                         - calibIn.triggerTime()
                         - calibResult.tubeT0() 
                         - calibResult.signalPropagationTime();

  calibResult.setDriftTime(driftTime);
  // apply corrections
  double corrTime{0.};
  const bool doCorrections = m_doField || m_doTemp || m_doBkg;
  if (doCorrections) {
    const CorrectionPtr& corrections{calibConstants->corrections};
    const RtRelationPtr& rtRelation{calibConstants->rtRelation};
    const MuonCalib::IRtRelation* rt = rtRelation->rt();
    ATH_MSG_VERBOSE("There are correction functions.");
    /// slewing corrections
      if (m_doSlew && corrections->slewing()) {
        double slewTime=corrections->slewing()->correction(calibResult.driftTime(), calibIn.adc());
        corrTime -= slewTime;
        calibResult.setSlewingTime(slewTime);
      }

   
      if (m_doField && corrections->bField()) {
        MagField::AtlasFieldCache fieldCache{};

        SG::ReadCondHandle readHandle{m_fieldCacheCondObjInputKey, ctx};
        if (!readHandle.isValid()) {
          THROW_EXCEPTION("calibrate: Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCacheCondObjInputKey.key());
        }
        readHandle->getInitializedCache(fieldCache);
 
        Amg::Vector3D  globalB{Amg::Vector3D::Zero()};
        fieldCache.getField(calibIn.closestApproach().data(), globalB.data());
        const Amg::Vector2D locBField = calibIn.projectMagneticField(globalB);
        using BFieldComp = MdtCalibInput::BFieldComp;
        calibResult.setLorentzTime(corrections->bField()->correction(calibResult.driftTime(), 
                                                                     locBField[static_cast<int>(BFieldComp::alongWire)], 
                                                                     locBField[static_cast<int>(BFieldComp::alongTrack)]));
        corrTime -= calibResult.lorentzTime();
      }
      if(m_doTemp && rt && rt->hasTmaxDiff()) {
        const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
        const int mL = id_helper.multilayer(id);
        const double tempTime = MuonCalib::RtScaleFunction(calibResult.driftTime(), mL == 2, *rt);
        calibResult.setTemperatureTime(tempTime);
        corrTime-=calibResult.temperatureTime();
      }
      // background corrections (I guess this is never active)
      if (m_doBkg && corrections->background()) {
        double bgLevel{0.};
        calibResult.setBackgroundTime(corrections->background()->correction(calibResult.driftTime(), bgLevel ));
        corrTime += calibResult.backgroundTime();
      }
  }

  calibResult.setDriftTime(calibResult.driftTime() + corrTime);

  // calculate drift radius + error
  double r{0.}, reso{0.};
  double t = calibResult.driftTime();
  double t_inrange = t;
  Muon::MdtDriftCircleStatus timeStatus = driftTimeStatus(t, *rtRelation);
  
  assert(rtRelation->rt() != nullptr);
  r = rtRelation->rt()->radius(t);
  // apply tUpper gshift
  if (m_doTMaxShift) {
    float tShift = m_tMaxShiftTool->getValue(id);
    r = rtRelation->rt()->radius( t * (1 + tShift) );
  }
  // check whether drift times are within range, if not fix them to the min/max range
  if ( t < rtRelation->rt()->tLower() ) {
    t_inrange = rtRelation->rt()->tLower();
    double rmin = rtRelation->rt()->radius( t_inrange );
    double drdt = (rtRelation->rt()->radius( t_inrange + 30. ) - rmin)/30.;
    /// now check whether we are outside the time window
    if (timeStatus == Muon::MdtStatusBeforeSpectrum) {
      t = rtRelation->rt()->tLower() - m_timeWindowLowerBound;
    }
    // if we get here we are outside the rt range but inside the window.
    r = std::max(rmin + drdt*(t-t_inrange), m_unphysicalHitRadiusLowerBound.value());
  } else if( t > rtRelation->rt()->tUpper() ) {
    t_inrange = rtRelation->rt()->tUpper();
    double rmax = rtRelation->rt()->radius( t_inrange );
    double drdt = (rmax - rtRelation->rt()->radius( t_inrange - 30. ))/30.;
    // now check whether we are outside the time window
    if ( timeStatus == Muon::MdtStatusAfterSpectrum ) {
      t = rtRelation->rt()->tUpper() + m_timeWindowUpperBound;
    }
    // if we get here we are outside the rt range but inside the window.
    r = rmax + drdt*(t-t_inrange);
  }

  assert(rtRelation->rtRes() != nullptr);
  if (!resolFromRtrack) {
    reso = rtRelation->rtRes()->resolution( t_inrange );
  } else {
    const std::optional<double> tFromR = rtRelation->tr()->driftTime(std::abs(calibIn.distanceToTrack()));
    reso = rtRelation->rtRes()->resolution(tFromR.value_or(0.));
  }
  

  if (m_doPropUncert) {
      assert(rtRelation->rt() != nullptr);
      const double driftTimeUp = std::min(rtRelation->rt()->tUpper(),
                                          calibIn.tdc() * tdcBinSize 
                                        - (m_doTof ? calibIn.timeOfFlight() : 0.)
                                        - calibIn.triggerTime()
                                        - calibResult.tubeT0());

      const double driftTimeDn = std::max(rtRelation->rt()->tLower(),
                                          calibIn.tdc() * tdcBinSize 
                                        - (m_doTof ? calibIn.timeOfFlight() : 0.)
                                        - calibIn.triggerTime()
                                        - calibResult.tubeT0()
                                        - calibIn.tubeLength() * invPropSpeed);

      const double radiusUp = rtRelation->rt()->radius(driftTimeUp);
      const double radiusDn = rtRelation->rt()->radius(driftTimeDn);
      ATH_MSG_VERBOSE("Measurement "<<m_idHelperSvc->toString(calibIn.identify())
          <<" nominal drift time "<<driftTime<<", down: "<<driftTimeDn<<", up: "<<driftTimeUp
          <<" --> driftRadius: "<<r<<" pm "<<reso<<", prop-up: "<<radiusUp<<", prop-dn: "<<radiusDn
          <<" delta: "<<(radiusUp-radiusDn));
      calibResult.setDriftUncertSigProp(0.5*std::abs(radiusUp - radiusDn));
      reso = std::hypot(reso, calibResult.driftUncertSigProp());
  }

  calibResult.setDriftRadius(r, reso);
  calibResult.setStatus(timeStatus);
  // summary
  ATH_MSG_VERBOSE( "Calibration for tube " << m_idHelperSvc->toString(id)
                   <<" passed. "<<std::endl<<"Input: "<<calibIn<<std::endl<<"Extracted calib constants: "<<calibResult<<std::endl);
  return calibResult;
}  //end MdtCalibrationTool::calibrate

MdtCalibTwinOutput MdtCalibrationTool::calibrateTwinTubes(const EventContext& ctx,
                                                          MdtCalibInput&& primHit, 
                                                          MdtCalibInput&& twinHit) const {
  
  MdtCalibOutput primResult = calibrate(ctx, primHit);
  MdtCalibOutput twinResult = calibrate(ctx, twinHit);

  // get Identifier and MdtReadOutElement for twin tubes
  // get 'raw' drifttimes of twin pair; we don't use timeofFlight or propagationTime cause they are irrelevant for twin coordinate
  double primdriftTime = primHit.tdc()*tdcBinSize - primResult.tubeT0();
  double twinDriftTime = twinHit.tdc()*tdcBinSize - twinResult.tubeT0();
  /// The primary hit has a smaller tdc...
  if (primdriftTime >= twinDriftTime) {
     ATH_MSG_VERBOSE("Swap "<<m_idHelperSvc->toString(primHit.identify())<<" & "<<m_idHelperSvc->toString(twinHit.identify())
                    <<" primDriftTime: "<<primdriftTime<<", secondTime: "<<twinDriftTime);
     std::swap(primdriftTime, twinDriftTime);
     std::swap(primResult, twinResult);
     std::swap(primHit, twinHit);
  }
  const Identifier& primId = primHit.identify();
  const Identifier& twinId = twinHit.identify();

  // get calibration constants from DbTool
  SG::ReadCondHandle constantHandle{m_calibDbKey, ctx};
  if (!constantHandle.isValid()){
    THROW_EXCEPTION("Failed to retrieve the Mdt calibration constants "<<m_calibDbKey.fullKey());
  }


  const MuonCalib::MdtFullCalibData* data1st = constantHandle->getCalibData(primId, msgStream());
  const MuonCalib::MdtFullCalibData* data2nd = constantHandle->getCalibData(twinId, msgStream());
  if (!data1st || !data2nd) {
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Failed to access calibration constants for tubes "<<
                    m_idHelperSvc->toString(primId)<<" & "<<m_idHelperSvc->toString(twinId));
    return MdtCalibTwinOutput{};
  }
  const SingleTubeCalib* calibSingleTube1st = data1st->tubeCalib->getCalib(primId);
  const SingleTubeCalib* calibSingleTube2nd = data2nd->tubeCalib->getCalib(twinId);
  const double invPropSpeed = constantHandle->inversePropSpeed();
  if (!calibSingleTube1st || !calibSingleTube2nd) {
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Failed to access calibration constants for tubes "<<
                  m_idHelperSvc->toString(primId)<<" & "<<m_idHelperSvc->toString(twinId));
    return MdtCalibTwinOutput{};
  }

  // get tubelength and set HV-delay (~6ns)
  constexpr double HVdelay = 6.;

  /// Propagation time difference inside the primary tube
  double twin_timedif =  twinDriftTime - primdriftTime - invPropSpeed * twinHit.tubeLength() - HVdelay;
  ///  HVPropTime - ROPropTime =  invPropSpeed*(twinZ - HVPos) - (ROPos - twinZ) 
  ///                          =  2*invPropSpeed*twinZ - (HVPos + ROPos)
  
  const double tubeHalfLength = 0.5*primHit.tubeLength();
  const double zTwin = std::clamp(0.5* primHit.readOutSide()* twin_timedif / invPropSpeed,  -tubeHalfLength, tubeHalfLength);
  const double errZTwin = m_resTwin / invPropSpeed;

  ATH_MSG_VERBOSE( "Twin calibration -  tube: " << m_idHelperSvc->toString(primId)<< " twintube: " << m_idHelperSvc->toString(twinId)<<endmsg
               << " prompthit tdc = " << primHit.tdc() << "  twinhit tdc = " << twinHit.tdc()
               << " tube driftTime = " << primResult<< "  second tube driftTime = " << twinResult<<endmsg
               << " Time difference =" << twin_timedif  << " zTwin=" << zTwin<<", errorZ="<<errZTwin);
  MdtCalibTwinOutput calibResult{primHit,twinHit,primResult, twinResult};     

 
  calibResult.setLocZ(zTwin, errZTwin);
  return calibResult;
} 

Muon::MdtDriftCircleStatus MdtCalibrationTool::driftTimeStatus(double driftTime, 
                                                               const MuonCalib::MdtRtRelation& rtRelation ) const {  
  if (rtRelation.rt()) {
    if(driftTime < rtRelation.rt()->tLower() - m_timeWindowLowerBound) {
        ATH_MSG_VERBOSE( " drift time outside time window "
                      << driftTime << ". Mininum time = "
                      << rtRelation.rt()->tLower() - m_timeWindowLowerBound );
        return Muon::MdtStatusBeforeSpectrum;
    } else if (driftTime > rtRelation.rt()->tUpper() + m_timeWindowUpperBound) {
        ATH_MSG_VERBOSE( " drift time outside time window "
                      << driftTime << ". Maximum time  = "
                      << rtRelation.rt()->tUpper() + m_timeWindowUpperBound);
        return Muon::MdtStatusAfterSpectrum;
    }
  } else {
    ATH_MSG_WARNING( "No valid rt relation supplied for driftTimeStatus method" );
    return Muon::MdtStatusUnDefined;
  }
  return Muon::MdtStatusDriftTime;
}
double MdtCalibrationTool::getResolutionFromRt(const EventContext& ctx, const Identifier& moduleID, const double time) const  {
  
  const MuonCalib::MdtFullCalibData* moduleConstants = getCalibConstants(ctx, moduleID);
  if (!moduleConstants){
      THROW_EXCEPTION("Failed to retrieve set of calibration constants for "<<m_idHelperSvc->toString(moduleID));
  }
  const RtRelationPtr& rtRel{moduleConstants->rtRelation};
  if (!rtRel) {
    THROW_EXCEPTION("No rt-relation found for "<<m_idHelperSvc->toString(moduleID));
  }
  const double t = std::min(std::max(time, rtRel->rt()->tLower()), rtRel->rt()->tUpper());
  return rtRel->rtRes()->resolution(t);
}