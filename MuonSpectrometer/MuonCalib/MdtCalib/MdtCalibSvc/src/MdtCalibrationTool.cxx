/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
#include "MdtCalibData/IMdtWireSagCorFunc.h"
#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/TrRelation.h"
#include "MdtCalibData/RtScaleFunction.h"
#include "MagFieldElements/AtlasFieldCache.h"
#include "MuonCalibEvent/MdtCalibHit.h"

namespace {
  static double const twoBySqrt12 = 2/std::sqrt(12);
}

using SingleTubeCalib = MuonCalib::MdtTubeCalibContainer::SingleTubeCalib;
using MdtDriftCircleStatus = MdtCalibOutput::MdtDriftCircleStatus;
using ToolSettings = MdtCalibrationTool::ToolSettings;
MdtCalibrationTool::MdtCalibrationTool(const std::string& type, const std::string &name, const IInterface* parent) :
    base_class(type, name, parent) {}

ToolSettings MdtCalibrationTool::getSettings() const {
    ToolSettings settings{};
    using Property = ToolSettings::Property;
    settings.setBit(Property::TofCorrection, m_doTof);
    settings.setBit(Property::PropCorrection, m_doProp);
    settings.setBit(Property::TempCorrection, m_doTemp);
    settings.setBit(Property::MagFieldCorrection, m_doField);
    settings.setBit(Property::WireSagTimeCorrection, m_doWireSag);
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
                <<"   Correct wire sagging "<<(m_doWireSag ? "si" : "no")<<std::endl
                <<"   Correct time slew "<<(m_doSlew ? "si" : "no")<<std::endl
                <<"   Correct background "<<(m_doBkg ? "si" : "no"));
  return StatusCode::SUCCESS;
}  //end MdtCalibrationTool::initialize



MdtCalibOutput MdtCalibrationTool::calibrate(const EventContext& ctx, 
                                             const MdtCalibInput& calibIn,
                                             bool resolFromRtrack) const {

  const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
  /// Get the calibration constatns from the conditions store
  SG::ReadCondHandle<MuonCalib::MdtCalibDataContainer> readCondHandle{m_calibDbKey, ctx};
  if (!readCondHandle.isValid()){
       ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to retrieve the Mdt calibration constants "<<m_calibDbKey.fullKey());
       throw std::runtime_error("No calibration constants could be retrieved");
  }

  const Identifier& id{calibIn.identify()};

  const MuonCalib::MdtFullCalibData* calibConstants = readCondHandle->getCalibData(id, msgStream());
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
  
  MdtCalibOutput calibResult{};
  // correct for global t0 of rt-region
  
  calibResult.setTubeT0(singleTubeData->t0 + rtRelation->t0Global());
  calibResult.setMeanAdc(singleTubeData->adcCal);

  // set propagation delay
  if (m_doProp) {   
    const double propagationDistance = calibIn.signalPropagationDistance(); 
    calibResult.setPropagationTime(singleTubeData->inversePropSpeed * propagationDistance);
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
  const bool doCorrections = m_doField || m_doTemp || m_doBkg || m_doWireSag;
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

        SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, ctx};
        if (!readHandle.isValid()) {
          ATH_MSG_FATAL("calibrate: Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCacheCondObjInputKey.key());
          throw std::runtime_error("No magnetic field could be retrieved");
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
      if(m_doTemp && rt && rt->HasTmaxDiff()) {
        const int mL = id_helper.multilayer(id);
        const double tempTime = MuonCalib::RtScaleFunction(calibResult.driftTime(), 
                                                           mL == 2, 
                                                           *rt);
        calibResult.setTemperatureTime(tempTime);
        corrTime-=calibResult.temperatureTime();
      }
      // background corrections (I guess this is never active)
      if (m_doBkg && corrections->background()) {
        double bgLevel{0.};
        calibResult.setBackgroundTime(corrections->background()->correction(calibResult.driftTime(), bgLevel ));
        corrTime += calibResult.backgroundTime();
      }
      /// Wire sag corrections
      if (m_doWireSag && corrections->wireSag()) {
        /// Retrieve the center of the sagged surface in global coordinates
        const Amg::Vector3D& saggedSurfPos{calibIn.saggedSurfCenter()};
        const Amg::Vector3D& nominalSurfPos{calibIn.surfaceCenter()};
        /// Calculate the sagging as the difference of the point of closest approach to the
        /// sagged center surface
        const double deltaY = calibIn.closestApproach().y() - saggedSurfPos.y();
        
        // sign of drift radius (for sag calculation) is +/- of track passes
        // above/below wire
        const double signedDriftRadius = deltaY*(std::abs(calibResult.driftRadius()/deltaY));

        // calculate the magnitude of the wire sag
        double effectiveSag = nominalSurfPos.y()
                            - saggedSurfPos.y();

        calibResult.setSaggingTime(corrections->wireSag()->correction(signedDriftRadius, effectiveSag));
        // apply the correction
        corrTime += calibResult.saggingTime();
      }
  }

  calibResult.setDriftTime(calibResult.driftTime() + corrTime);

  // calculate drift radius + error
  double r{0.}, reso{0.};
  double t = calibResult.driftTime();
  double t_inrange = t;
  Muon::MdtDriftCircleStatus timeStatus = driftTimeStatus(t, *rtRelation);
  if(rtRelation->rt()) {
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
  } else {
    ATH_MSG_WARNING( "no rt found" );
    return calibResult;
  }

  if (rtRelation->rtRes()) {
    if (!resolFromRtrack) {
      reso = rtRelation->rtRes()->resolution( t_inrange );
    } else {
      bool boundFlag{false};
      const double tFromR = rtRelation->tr()->tFromR(std::abs(calibIn.distanceToTrack()),
                                                     boundFlag);
      reso = rtRelation->rtRes()->resolution(tFromR);
    }
  } else {
    ATH_MSG_WARNING( "no rtRes found" );
    return calibResult;
  }
  calibResult.setDriftRadius(r, reso);
  calibResult.setStatus(timeStatus);
  // summary
  ATH_MSG_VERBOSE( "Calibration for tube " << m_idHelperSvc->toString(id)
                   <<" passed. "<<std::endl<<"Input: "<<calibIn<<std::endl<<"Extracted calib constants: "<<calibResult<<std::endl);
  return calibResult;
}  //end MdtCalibrationTool::calibrate

MdtCalibTwinOutput MdtCalibrationTool::calibrateTwinTubes(const EventContext& ctx,
                                                          const MdtCalibInput& primHit, 
                                                          const MdtCalibInput& twinHit) const {
  
  MdtCalibOutput primResult = calibrate(ctx, primHit);
  MdtCalibOutput twinResult = calibrate(ctx, twinHit);

  // get Identifier and MdtReadOutElement for twin tubes
  const Identifier& primId = primHit.identify();
  const Identifier& twinId = twinHit.identify();
  // get 'raw' drifttimes of twin pair; we don't use timeofFlight or propagationTime cause they are irrelevant for twin coordinate
  double primdriftTime = primHit.tdc()*tdcBinSize - primResult.tubeT0();
  double twinDriftTime = twinHit.tdc()*tdcBinSize - twinResult.tubeT0();

  /// Get the calibration constatns from the conditions store
  SG::ReadCondHandle<MuonCalib::MdtCalibDataContainer> calibDataContainer{m_calibDbKey, ctx};
  if (!calibDataContainer.isValid()){
       ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to retrieve the Mdt calibration constants "<<m_calibDbKey.fullKey());
       throw std::runtime_error("No calibration constants could be retrieved");
  }
  // get calibration constants from DbTool
  const MuonCalib::MdtFullCalibData* data1st = calibDataContainer->getCalibData(primId, msgStream());
  const MuonCalib::MdtFullCalibData* data2nd = calibDataContainer->getCalibData(twinId, msgStream());
  if (!data1st || !data2nd) {
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Failed to access calibration constants for tubes "<<
                    m_idHelperSvc->toString(primId)<<" & "<<m_idHelperSvc->toString(twinId));
    return MdtCalibTwinOutput{};
  }
  const SingleTubeCalib* calibSingleTube1st = data1st->tubeCalib->getCalib(primId);
  const SingleTubeCalib* calibSingleTube2nd = data2nd->tubeCalib->getCalib(twinId);
  if (!calibSingleTube1st || !calibSingleTube2nd) {
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Failed to access calibration constants for tubes "<<
                  m_idHelperSvc->toString(primId)<<" & "<<m_idHelperSvc->toString(twinId));
    return MdtCalibTwinOutput{};
  }

  const double invPropSpeed1st{calibSingleTube1st->inversePropSpeed}; 
  const double invPropSpeed2nd{calibSingleTube2nd->inversePropSpeed};

  // define twin position and error
  double zTwin{0.}, errZTwin{0.}, twin_timedif{0.};
  // find out which tube was the prompt
  // (= actually hit by the muon; not hit by the muon = twinhit_)
  // in the formula for z_hit_from_twin we take as convention that
  // twindif = twin_time - prompt_time
  int prompthit_tdc{0}, twinhit_tdc{0};
  bool firstIsPrompt{true};
  if ( primdriftTime < twinDriftTime) {
    twin_timedif = twinDriftTime - primdriftTime;
  } else {
    twin_timedif = primdriftTime - twinDriftTime;
    firstIsPrompt = false;
  }

  // get tubelength and set HV-delay (~6ns)
  const MdtCalibInput& primaryHit{(firstIsPrompt ? primHit : twinHit)};
  const double tubelength = primaryHit.tubeLength();
  constexpr double HVdelay = 6.;

  // twin_timedif must be between min and max of possible time-difference
  // between prompt and twin signals
  // accounting for 5 std.dev. of twin time resolution
  if ( twin_timedif < (HVdelay - 5.*m_resTwin) || 
       twin_timedif > (tubelength*(invPropSpeed1st + invPropSpeed2nd)
                       + HVdelay + 5.*m_resTwin)){
      ATH_MSG_DEBUG( " TIME DIFFERENCE OF TWIN PAIR OUT OF RANGE("
             << (HVdelay - 5.*m_resTwin)<< "-"
             << (tubelength*(invPropSpeed1st + invPropSpeed2nd) + HVdelay + 5.*m_resTwin)
             << ")   time difference = " << twin_timedif );
    
  }

  // Make ONLY a twin PrepData if twin time difference is physical (within tubelength)
  if (twin_timedif < (tubelength* (invPropSpeed1st + invPropSpeed2nd)
                     + HVdelay + 10.*m_resTwin)){

    //calculate local(!) z of the hit from twin tubes information
    double z_hit_sign_from_twin = ( 1 / (invPropSpeed2nd *2.)) * 
                                  (tubelength*invPropSpeed2nd - 
                                   twin_timedif + HVdelay) ;
    /// Put twin hit always inside acceptance
    if (z_hit_sign_from_twin < -tubelength/2.) {
      ATH_MSG_DEBUG( " TWIN HIT outside acceptance with time difference "
                    <<  twin_timedif
                    << " Z local hit " <<  z_hit_sign_from_twin
                    << " Z local minimum " <<  -tubelength/2 );
      z_hit_sign_from_twin = - tubelength/2.;
    }
    // do sign management just like in MdtDigitizationTool.cxx
    const double z_hit_geo_from_twin = primaryHit.readOutSide() *z_hit_sign_from_twin;

    zTwin = z_hit_geo_from_twin;
    errZTwin = m_resTwin*invPropSpeed1st;

    ATH_MSG_VERBOSE( " TWIN TUBE "
                     << " tube: " << m_idHelperSvc->toString(primId)
                     << " twintube: " << m_idHelperSvc->toString(twinId)<<endmsg
                     << " prompthit tdc = " << prompthit_tdc//*TDCbinsize
                     << "  twinhit tdc = " << twinhit_tdc// *TDCbinsize
                     << "  tube driftTime = " << primResult
                     << "  second tube driftTime = " << twinResult
                     << " TWIN PAIR time difference = " << twin_timedif << endmsg
                     << " z_hit_sign_from_twin = " << z_hit_sign_from_twin
                     << " z_hit_geo_from_twin = " << z_hit_geo_from_twin);    

  } // end  if(twin_timedif < (tubelength*inversePropSpeed + tubelength*inversePropSpeedSecond + HVdelay + 10.*m_resTwin)){
  else {
    ATH_MSG_VERBOSE( " TIME DIFFERENCE OF TWIN PAIR UNPHYSICAL OUT OF RANGE("
                  << (HVdelay - 5*m_resTwin) << "-"
                  << (2*tubelength*invPropSpeed1st + HVdelay + 5*m_resTwin)
                  << ")   time difference = "
                  << twin_timedif );
    zTwin = 0.;
    errZTwin = tubelength/2.;
  }

  MdtCalibTwinOutput calibResult{(firstIsPrompt ? primHit : twinHit),
                                 (firstIsPrompt ? twinHit : primHit),
                                 (firstIsPrompt ? primResult : twinResult),
                                 (firstIsPrompt ? twinResult : primResult)};    

 
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
  
  SG::ReadCondHandle<MuonCalib::MdtCalibDataContainer> calibConstants{m_calibDbKey, ctx};
  if (!calibConstants.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve the calibration constants "<<m_calibDbKey.fullKey());
      throw std::runtime_error("Where are my Mdt calibration constants");
  }
  const MuonCalib::MdtFullCalibData* moduleConstants = calibConstants->getCalibData(moduleID, msgStream());
  if (!moduleConstants){
      ATH_MSG_FATAL("Failed to retrieve set of calibration constants for "<<m_idHelperSvc->toString(moduleID));
      throw std::runtime_error("No constants for calib container");
  }
  const RtRelationPtr& rtRel{moduleConstants->rtRelation};
  if (!rtRel) {
    ATH_MSG_FATAL("No rt-relation found for "<<m_idHelperSvc->toString(moduleID));
    throw std::runtime_error("No rt relation ");
  }
  const double t = std::min(std::max(time, rtRel->rt()->tLower()), rtRel->rt()->tUpper());
  return rtRel->rtRes()->resolution(t);
}