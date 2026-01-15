/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "NSWCalibTool.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#include "xAODMuonPrepData/MMCluster.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include <cfloat>
namespace {
  constexpr double toRad = M_PI/180;
  constexpr double pitchErr = 0.425 * 0.425 / 12;
  constexpr double reciprocalSpeedOfLight = 1. / Gaudi::Units::c_light; // mm/ns
  
  // since the final operation gas is not yet fixed different, different mixtures are added for studies 
  static const std::map<std::string, float> map_transDiff {{"ArCo2_937", 0.036},
                              {"ArCo2_8020", 0.019}, {"ArCo2iC4H10_9352", 0.035}};
  static const std::map<std::string, float> map_longDiff {{"ArCo2_937", 0.019},
                              {"ArCo2_8020", 0.022 }, {"ArCo2iC4H10_9352", 0.0195}};
  static const std::map<std::string, float> map_vDrift {{"ArCo2_937", 0.047},
                              {"ArCo2_8020", 0.040}, {"ArCo2iC4H10_9352", 0.045}};


  // unit correction factors
  constexpr double const MM_electronsPerfC = 6241.;
  constexpr double const sTGC_pCPerfC = 1000.;


  //Functional form fit to agree with Garfield simulations. Fit and parameters from G. Iakovidis
  // For now only the parametrisation for 93:7 is available
  using angleFunction = NSWCalib::MicroMegaGas::angleFunction;
  static const std::map<std::string, angleFunction> map_lorentzAngleFunctionPars {
                              {"ArCo2_937",  [](double x)  {return 0.f + 58.87f*x -2.983f*x*x -10.62f*x*x*x + 2.818f*x*x*x*x;}},
                              {"ArCo2_8020", [](double x)  {return 0.f + 58.87f*x -2.983f*x*x -10.62f*x*x*x + 2.818f*x*x*x*x;}},
                              {"ArCo2iC4H10_9352", [](double x)  {return 0.f + 58.87f*x -2.983f*x*x -10.62f*x*x*x + 2.818f*x*x*x*x;}}};

  // For now only the parametrisation for 93:7 is available
  static const std::map<std::string, float> map_interactionDensitySigma {{"ArCo2_937", 4.04 / 5.},
                                   {"ArCo2_8020", 4.04 / 5.}, {"ArCo2iC4H10_9352", 4.04 / 5.}};
  static  const std::map<std::string, float> map_interactionDensityMean {{"ArCo2_937", 16.15 / 5.},
                                   {"ArCo2_8020", 16.15 / 5.}, {"ArCo2iC4H10_9352", 16.15 / 5.}};

}

StatusCode Muon::NSWCalibTool::initialize()
{
  ATH_MSG_DEBUG("In initialize()");
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_condTdoPdoKey.initialize());
  ATH_CHECK(m_condT0Key.initialize(m_applyMmT0Calib || m_applysTgcT0Calib));
  ATH_CHECK(m_fieldCondObjInputKey.initialize());
  ATH_CHECK(m_muDetMgrKey.initialize());
  ATH_CHECK(initializeGasProperties());
  ATH_CHECK(m_ctpClusterCalibKey.initialize(m_CalibDriftVelocityFromData));  
  ATH_CHECK(initializeGasProperties());
  return StatusCode::SUCCESS;
}

StatusCode Muon::NSWCalibTool::initializeGasProperties() {
  if (!map_vDrift.count(m_gasMixture)) {
    ATH_MSG_FATAL("Configured Micromegas with unkown gas mixture: " << m_gasMixture);
    return StatusCode::FAILURE;
  }

  m_vDrift = map_vDrift.find(m_gasMixture)->second;
  m_transDiff = map_transDiff.find(m_gasMixture)->second;
  m_longDiff = map_longDiff.find(m_gasMixture)->second;
  m_interactionDensitySigma = map_interactionDensitySigma.find(m_gasMixture)->second;
  m_interactionDensityMean =  map_interactionDensityMean.find(m_gasMixture)->second;
  m_lorentzAngleFunction =  map_lorentzAngleFunctionPars.find(m_gasMixture)->second;
  return StatusCode::SUCCESS;
}
StatusCode Muon::NSWCalibTool::calibrateClus(const EventContext& ctx, const Muon::MMPrepData* prepData, const Amg::Vector3D& globalPos, std::vector<NSWCalib::CalibratedStrip>& calibClus) const {

  double lorentzAngle {0.};
  if(m_applyMmBFieldCalib){
    /// magnetic field
    MagField::AtlasFieldCache fieldCache;
    ATH_CHECK(loadMagneticField(ctx, fieldCache));
    Amg::Vector3D magneticField{Amg::Vector3D::Zero()};
    fieldCache.getField(globalPos.data(), magneticField.data());

    /// get the component parallel to to the eta strips (same used in digitization)
    double phi    = globalPos.phi();
    double bfield = (magneticField.x()*std::sin(phi)-magneticField.y()*std::cos(phi))*1000.;

    /// swap sign depending on the readout side
    int gasGap = m_idHelperSvc->mmIdHelper().gasGap(prepData->identify());
    bool changeSign = ( globalPos.z() < 0. ? (gasGap==1 || gasGap==3) : (gasGap==2 || gasGap==4) );
    if (changeSign) bfield = -bfield;

    //// sign of the lorentz angle matches digitization - angle is in radians
    lorentzAngle = (bfield>0. ? 1. : -1.)*m_lorentzAngleFunction(std::abs(bfield)) * toRad;
  }

  /// loop over prepData strips
  for (unsigned int i = 0; i < prepData->stripNumbers().size(); ++i){
    Identifier id = prepData->rdoList().at(i);
    double time = prepData->stripTimes().at(i);
    double charge = prepData->stripCharges().at(i);
    //Retrieve pointing constraint
    const Amg::Vector3D& globPos{prepData->globalPosition()};
    NSWCalib::CalibratedStrip calibStrip;
    ATH_CHECK(calibrateStrip(ctx, id, time, charge, (globPos.theta() / toRad) , lorentzAngle, calibStrip));

    calibClus.push_back(std::move(calibStrip));
  }
  return StatusCode::SUCCESS;
}


StatusCode Muon::NSWCalibTool::calibrateClus(const EventContext& ctx, 
                                             const ActsTrk::GeometryContext& gctx, 
                                             const xAOD::MMCluster& prepData, 
                                             const Amg::Vector3D& globalPos, 
                                             std::vector<NSWCalib::CalibratedStrip>& calibClus) const {

  double lorentzAngle {0.};
  if(m_applyMmBFieldCalib){
    /// magnetic field
    MagField::AtlasFieldCache fieldCache;
    ATH_CHECK(loadMagneticField(ctx, fieldCache));
    Amg::Vector3D magneticField{Amg::Vector3D::Zero()};
    fieldCache.getField(globalPos.data(), magneticField.data());

    /// get the component parallel to to the eta strips (same used in digitization)
    double phi    = globalPos.phi();
    double bfield = (magneticField.x()*std::sin(phi)-magneticField.y()*std::cos(phi))*1000.;

    /// swap sign depending on the readout side
    int gasGap = m_idHelperSvc->mmIdHelper().gasGap(prepData.identify());
    bool changeSign = ( globalPos.z() < 0. ? (gasGap==1 || gasGap==3) : (gasGap==2 || gasGap==4) );
    if (changeSign) bfield = -bfield;

    //// sign of the lorentz angle matches digitization - angle is in radians
    lorentzAngle = (bfield>0. ? 1. : -1.)*m_lorentzAngleFunction(std::abs(bfield)) * toRad;
  }

  /// loop over prepData strips
  for (unsigned int i = 0; i < prepData.stripNumbers().size(); ++i){
    Identifier id =  m_idHelperSvc->mmIdHelper().channelID(prepData.identify(), m_idHelperSvc->mmIdHelper().multilayer(prepData.identify()), m_idHelperSvc->mmIdHelper().gasGap(prepData.identify()),prepData.stripNumbers().at(i));
    double time = prepData.stripTimes().at(i);
    double charge = prepData.stripCharges().at(i);
    //Retrieve pointing constraint
    const Amg::Vector3D& globPos{prepData.readoutElement()->localToGlobalTransform(gctx, prepData.layerHash()) * (prepData.localPosition<1>()[0]*Amg::Vector3D::UnitX())};
    NSWCalib::CalibratedStrip calibStrip;
    ATH_CHECK(calibrateStrip(ctx, id, time, charge, (globPos.theta() / toRad) , lorentzAngle, calibStrip));

    calibClus.push_back(std::move(calibStrip));
  }
  return StatusCode::SUCCESS;
}



StatusCode Muon::NSWCalibTool::calibrateStrip(const EventContext& ctx, 
                                              const Identifier& id, 
                                              const double time, 
                                              const double charge, 
                                              const double theta, 
                                              const double lorentzAngle, 
                                              NSWCalib::CalibratedStrip& calibStrip) const {
  //get local positon
  auto locPos = localStripPosition(ctx, id);
  if(!locPos) {
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Failed to retrieve local strip position "<<m_idHelperSvc->toString(id));
    return StatusCode::FAILURE;
  }
  
  calibStrip.identifier = id;
  calibStrip.charge = charge;
  calibStrip.time = time;

  const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};

  //retrieve identifier for the gas gap, not necessarily for the channel
  //There is no pcb segmentation for these corrections, stored with pcb = 1 as default 
  Identifier gasGapId = idHelper.channelID(id,
                                          idHelper.multilayer(id),
                                          idHelper.gasGap(id), 1);

  double vDrift = m_vDrift; //nominal value from Garfield simulation  

  if(m_CalibDriftVelocityFromData) {
     const MmCTPClusterCalibData* ctpCalibData{nullptr};
     ATH_CHECK(SG::get(ctpCalibData, m_ctpClusterCalibKey, ctx));
     vDrift = ctpCalibData->getCTPCorrectedDriftVelocity(gasGapId, theta);

     //Calculate the new half max possible time based on the new drift velocity
     float max_half_drifttime = (vDrift != 0 ) ? 2.5/vDrift : 50.;

     //Shift the mean of the time to account for different values of drift velocities
     calibStrip.time = time + (max_half_drifttime - m_mmT0TargetValue);
     ATH_MSG_VERBOSE( "Original drift time: " << time << " new max half drift time: " << max_half_drifttime 
                  <<  " new time: " << calibStrip.time << " targett0 " << m_mmT0TargetValue );
  }


  double vDriftCorrected = vDrift * std::cos(lorentzAngle);

  calibStrip.distDrift = vDriftCorrected * calibStrip.time;

  /// transversal and longitudinal components of the resolution
  calibStrip.resTransDistDrift = pitchErr + std::pow(m_transDiff * calibStrip.distDrift, 2);
  calibStrip.resLongDistDrift = std::pow(m_ionUncertainty * vDriftCorrected, 2)
                              + std::pow(m_longDiff * calibStrip.distDrift, 2);
  calibStrip.dx = std::sin(lorentzAngle) * calibStrip.time * vDrift;  
  calibStrip.locPos = Amg::Vector2D{locPos->x() + calibStrip.dx, locPos->y()}; 
  return StatusCode::SUCCESS;
}


StatusCode Muon::NSWCalibTool::calibrateStrip(const EventContext& ctx, 
                                              const Muon::MM_RawData* mmRawData, 
                                              NSWCalib::CalibratedStrip& calibStrip) const {  
 
  const Identifier& rdoId = mmRawData->identify();

  const MuonGM::MuonDetectorManager* muDetMgr{nullptr};
  ATH_CHECK(SG::get(muDetMgr, m_muDetMgrKey, ctx));
 
  //get globalPos
  Amg::Vector3D globalPos{Amg::Vector3D::Zero()};
  const MuonGM::MMReadoutElement* detEl = muDetMgr->getMMReadoutElement(rdoId);
  if (!detEl->stripGlobalPosition(rdoId, globalPos)) {
      ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to fetch global position for "
                    <<m_idHelperSvc->toString(rdoId));
      return StatusCode::FAILURE;
  }

  // RDO has values in counts for both simulation and data
  float time{-std::numeric_limits<float>::max()}, 
        charge{-std::numeric_limits<float>::max()};
  tdoToTime  (ctx, mmRawData->timeAndChargeInCounts(), mmRawData->time(), rdoId, time, mmRawData->relBcid()); 
  pdoToCharge(ctx, mmRawData->timeAndChargeInCounts(), mmRawData->charge(), rdoId, charge);

  calibStrip.charge = charge;
  // historically the peak time is included in the time determined by the MM digitization and therefore added back in the tdoToTime function
  // in order to not break the RDO to digit conversion needed for the trigger and the overlay
  calibStrip.time = time - globalPos.norm() * reciprocalSpeedOfLight - mmPeakTime();
  // applying T0 calibration, cannot be done inside the the tdo to time function since the tof correction was included when deriving the calibration constants
  if(m_applyMmT0Calib){
    calibStrip.time = applyT0Calibration(ctx, rdoId, calibStrip.time);
  }

  calibStrip.identifier = rdoId;

  ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Calibrating RDO " << m_idHelperSvc->toString(rdoId) 
             << " with pdo: " << mmRawData->charge() << ", tdo: "<< mmRawData->time() 
             << ", relBCID "<< mmRawData->relBcid() << ", charge and time in counts: "
             << mmRawData->timeAndChargeInCounts() <<", "<< m_isData << " to charge: " 
             << calibStrip.charge << " electrons  time after corrections " << calibStrip.time 
             << " ns  time before corrections "<< time << "ns");
  //get stripWidth
  detEl->getDesign(rdoId)->channelWidth(); // positon is not used for strip width 

  calibStrip.distDrift = m_vDrift * calibStrip.time;
  calibStrip.resTransDistDrift = pitchErr + std::pow(m_transDiff * calibStrip.distDrift, 2);
  calibStrip.resLongDistDrift = std::pow(m_ionUncertainty * m_vDrift, 2)
                              + std::pow(m_longDiff * calibStrip.distDrift, 2);

  /// Hopefully this does not trigger the T0 reference test....
  detEl->surface(rdoId).globalToLocal(globalPos, Amg::Vector3D::Zero(), calibStrip.locPos);

  return StatusCode::SUCCESS;
}

StatusCode Muon::NSWCalibTool::calibrateStrip(const EventContext& ctx, 
                                              const Muon::STGC_RawData* sTGCRawData, 
                                              NSWCalib::CalibratedStrip& calibStrip) const {

  const Identifier& rdoId = sTGCRawData->identify();
  const MuonGM::MuonDetectorManager* muDetMgr{nullptr};
  ATH_CHECK(SG::get(muDetMgr, m_muDetMgrKey, ctx));

  //get globalPos
  Amg::Vector3D globalPos{Amg::Vector3D::Zero()};
  const MuonGM::sTgcReadoutElement* detEl = muDetMgr->getsTgcReadoutElement(rdoId);
  if (!detEl->stripGlobalPosition(rdoId,globalPos)) {
      ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to retrieve a valid global position for "
                  <<m_idHelperSvc->toString(rdoId));
      return StatusCode::FAILURE;
  }
  // RDO has values in counts for both simulation and data
  float time{-std::numeric_limits<float>::max()}, charge{-std::numeric_limits<float>::max()};
  tdoToTime(ctx, sTGCRawData->timeAndChargeInCounts(), sTGCRawData->time(), rdoId, time, sTGCRawData->bcTag()); 
  pdoToCharge(ctx, sTGCRawData->timeAndChargeInCounts(), sTGCRawData->charge(), rdoId, charge);
  if(sTGCRawData->timeAndChargeInCounts()){
    calibStrip.charge = charge  * sTGC_pCPerfC;
  } else {
    calibStrip.charge = charge;
  }
  calibStrip.time = time - stgcPeakTime();

  if(m_applysTgcT0Calib){
    calibStrip.time = applyT0Calibration(ctx, rdoId, calibStrip.time);
  }

  calibStrip.identifier = rdoId;
  /// Hopefully this does not trigger the T0 reference test....
  detEl->surface(rdoId).globalToLocal(globalPos, Amg::Vector3D::Zero(), calibStrip.locPos);

  return StatusCode::SUCCESS;
  
}
StatusCode Muon::NSWCalibTool::loadMagneticField(const EventContext& ctx, MagField::AtlasFieldCache& fieldCache) const {
  const AtlasFieldCacheCondObj* condObj{};
  ATH_CHECK(SG::get(condObj, m_fieldCondObjInputKey, ctx));
  condObj->getInitializedCache(fieldCache);
  return StatusCode::SUCCESS;
}
StatusCode Muon::NSWCalibTool::distToTime(const EventContext& ctx, 
                                          const Muon::MMPrepData* prepData, 
                                          const Amg::Vector3D& globalPos, 
                                          const std::vector<double>& driftDistances, 
                                          std::vector<double>& driftTimes) const {
  /// retrieve the magnetic field
  MagField::AtlasFieldCache fieldCache;
  ATH_CHECK(loadMagneticField(ctx, fieldCache));
  Amg::Vector3D magneticField{Amg::Vector3D::Zero()};
  fieldCache.getField(globalPos.data(), magneticField.data());

  /// get the component parallel to to the eta strips (same used in digitization)
  const double phi = globalPos.phi();
  double bfield = (magneticField.x()*std::sin(phi)-magneticField.y()*std::cos(phi))*1000.;

  /// swap sign depending on the readout side  
  int gasGap = m_idHelperSvc->mmIdHelper().gasGap(prepData->identify());
  bool changeSign = ( globalPos.z() < 0. ? (gasGap==1 || gasGap==3) : (gasGap==2 || gasGap==4) );
  if (changeSign) bfield = -bfield;
  double cos2_lorentzAngle = std::pow(std::cos ( (bfield>0. ? 1. : -1.)*m_lorentzAngleFunction(std::abs(bfield)) * toRad), 2);
  /// loop over drift distances                                                                                             
  for (const double dist : driftDistances){
    double time = dist / (m_vDrift*cos2_lorentzAngle);
    driftTimes.push_back(time);
  }
  return StatusCode::SUCCESS;

}

bool
Muon::NSWCalibTool::chargeToPdo(const EventContext& ctx, const float charge, const Identifier& chnlId, int& pdo) const {
  const NswCalibDbTimeChargeData* tdoPdoData{nullptr};
  if (!SG::get(tdoPdoData, m_condTdoPdoKey, ctx)) {
    pdo = 0;
    return false;  
  }
  const TimeCalibConst* calib_ptr = tdoPdoData->getCalibForChannel(TimeCalibType::PDO, chnlId);
  if (!calib_ptr) {
    pdo = 0;
    return false; 
  }
  const TimeCalibConst& calib{*calib_ptr};
  float c = charge;
  if     (m_idHelperSvc->isMM  (chnlId)) c /= MM_electronsPerfC;
  else if(m_idHelperSvc->issTgc(chnlId)) c *= sTGC_pCPerfC;
  else {
    pdo = 0;
    return false;
  }
  pdo = c * calib.slope + calib.intercept;
  return true;
}

bool
Muon::NSWCalibTool::pdoToCharge(const EventContext& ctx,
                                const bool inCounts,
                                const int pdo,
                                const Identifier& chnlId,
                                float& charge) const {
  if(!inCounts){
    charge = pdo;
    return true;
  }
  const NswCalibDbTimeChargeData* tdoPdoData{nullptr};
  if (!SG::get(tdoPdoData, m_condTdoPdoKey, ctx)) {
    charge = 0.;
    return false;  
  }
  const TimeCalibConst* calib_ptr = tdoPdoData->getCalibForChannel(TimeCalibType::PDO, chnlId);
  if (!calib_ptr) {
    charge = 0.;
    return false; 
  }
  const TimeCalibConst& calib{*calib_ptr};
  charge = (pdo-calib.intercept)/calib.slope;
  if     (m_idHelperSvc->isMM  (chnlId)) charge *= MM_electronsPerfC;
  else if(m_idHelperSvc->issTgc(chnlId)) charge /= sTGC_pCPerfC;
  else return false;
  return true;
}

bool 
Muon::NSWCalibTool::timeToTdo(const EventContext& ctx, const float time, const Identifier& chnlId, int& tdo, int& relBCID) const {
  const NswCalibDbTimeChargeData* tdoPdoData{nullptr};
  if (!SG::get(tdoPdoData, m_condTdoPdoKey, ctx)) return false;
  if (m_idHelperSvc->isMM  (chnlId)) return timeToTdoMM  (tdoPdoData, time, chnlId, tdo, relBCID);
  else if(m_idHelperSvc->issTgc(chnlId)) return timeToTdoSTGC(tdoPdoData, time, chnlId, tdo, relBCID);
  return false;
}

bool 
Muon::NSWCalibTool::timeToTdoMM(const NswCalibDbTimeChargeData* tdoPdoData, const float time, const Identifier& chnlId, int& tdo, int& relBCID) const {
  const float t = time - m_mmPeakTime - m_mmLatencyMC; // subtract peaking time first! This is not supossed to run on data ever only needed for the RDO->Digit step
  const TimeCalibConst* calib_ptr = tdoPdoData->getCalibForChannel(TimeCalibType::TDO, chnlId);
  if (!calib_ptr) {
    tdo = relBCID = 0;
    return false;
  }
  const TimeCalibConst& calib{*calib_ptr};
  float tdoTime = -999.9;
  constexpr float lowerBound = Muon::MM_RawData::s_lowerTimeBound;
  for(int i_relBCID=0; i_relBCID<Muon::MM_RawData::s_BCWindow; i_relBCID++){
    if(t >= lowerBound+i_relBCID*25 && t < (lowerBound+25)+i_relBCID*25){
      tdoTime = i_relBCID*25 - t;
      relBCID = i_relBCID;
      break;
    }
  }
  if(tdoTime < lowerBound) {
    tdo = relBCID = 0;
    return false;
  }
  tdo = tdoTime*calib.slope + calib.intercept;
  return true;
}

bool 
Muon::NSWCalibTool::timeToTdoSTGC(const NswCalibDbTimeChargeData* tdoPdoData, const float time, const Identifier& chnlId, int& tdo, int& relBCID) const {
  const float t = time - m_stgcPeakTime - m_stgcLatencyMC; // subtract peaking time and latency first! This is not supossed to run on data ever only needed for the RDO->Digit step
  const TimeCalibConst* calib_ptr = tdoPdoData->getCalibForChannel(TimeCalibType::TDO, chnlId);
  if (!calib_ptr){
    tdo = relBCID = 0;
    return false;
  }
  const TimeCalibConst& calib = {*calib_ptr};
  float tdoTime = -999.9;
  const float lowerBound = Muon::STGC_RawData::s_lowerTimeBound - m_stgcLatencyMC; // this is not supossed to run on data ever, only needed for the RDO->Digit step
  for(int i_relBCID=0; i_relBCID<Muon::STGC_RawData::s_BCWindow; ++i_relBCID){
    if(t >= lowerBound+i_relBCID*25 && t < (lowerBound+25)+i_relBCID*25){
      tdoTime = i_relBCID*25 - t;
      relBCID = i_relBCID;
      break;
    }
  }
  if(tdoTime < lowerBound) {
    tdo = relBCID = 0;
    return false;
  }
  tdo = tdoTime*calib.slope + calib.intercept;
  return true;
}

float Muon::NSWCalibTool::applyT0Calibration(const EventContext& ctx, const Identifier& id, float time) const {
  const NswT0Data* readT0{nullptr};
  if(!SG::get(readT0, m_condT0Key, ctx).isSuccess()){
    ATH_MSG_ERROR("Cannot find conditions data container for T0s!");
  }
  std::optional<float> t0 = readT0->getT0(id);
  if(!t0){
    ATH_MSG_DEBUG("failed to retrieve good t0 from database, skipping t0 calibration");
    return time;
  }  
  const auto& targetT0 = (m_idHelperSvc->isMM(id) ? m_mmT0TargetValue  : m_stgcT0TargetValue);
  float newTime = time + (targetT0 - (*t0));
  ATH_MSG_DEBUG("doing T0 calibration for RDO " << m_idHelperSvc->toString(id) << " time " << time 
            <<" t0 from  database " <<  (*t0)  << " t0 target " << targetT0  << " new time " <<  newTime);
  return newTime;
  
}


bool 
Muon::NSWCalibTool::tdoToTime(const EventContext& ctx, const bool inCounts, const int tdo, const Identifier& chnlId, float& time, const int relBCID) const {
  if(!inCounts){
    time = tdo;
    return true;
  }
  const NswCalibDbTimeChargeData* tdoPdoData{nullptr};
  if (!SG::get(tdoPdoData, m_condTdoPdoKey, ctx)){
    time = 0.;
    return false;  
  }
  const TimeCalibConst* calib_ptr = tdoPdoData->getCalibForChannel(TimeCalibType::TDO, chnlId);
  if (!calib_ptr){
     time = 0.;
     return false;
  } 
  const TimeCalibConst& calib {*calib_ptr};
  //this shift of 25ns is necessary to align the time of the signal with the way the VMM determines the time
  //(relBCID 0 corresponds to -37.5 ns to - 12.5 ns)
  //Eventually it should go into the conditions db since it is probably not the same for MC and Data
  //but for now it is kept like it is. pscholer 8th of June 2022
  float mmLatency   = (m_isData? m_mmLatencyData   : m_mmLatencyMC  );
  float stgcLatency = (m_isData? m_stgcLatencyData : m_stgcLatencyMC);

  const float peakTime  = m_idHelperSvc->isMM(chnlId) ? mmPeakTime() + mmLatency : stgcPeakTime() + stgcLatency; 
  time = relBCID*25. - (tdo-calib.intercept)/calib.slope + peakTime;
  return true;
}

NSWCalib::MicroMegaGas  Muon::NSWCalibTool::mmGasProperties() const {
  NSWCalib::MicroMegaGas properties{};
  properties.driftVelocity  = m_vDrift;
  properties.longitudinalDiffusionSigma = m_longDiff;
  properties.transverseDiffusionSigma = m_transDiff;
  properties.interactionDensityMean = m_interactionDensityMean;
  properties.interactionDensitySigma = m_interactionDensitySigma;
  properties.lorentzAngleFunction = m_lorentzAngleFunction;
  return properties;
}


std::optional<Amg::Vector2D> Muon::NSWCalibTool::localStripPosition(const EventContext& ctx,
                                                                     const Identifier& id) const {
  
  const MuonGM::MuonDetectorManager* muDetMgr{nullptr};
  if (!SG::get(muDetMgr, m_muDetMgrKey, ctx)) {
    THROW_EXCEPTION("Failed to retrieve the detector manager");
  }
  Amg::Vector2D locPos{Amg::Vector2D::Zero()};
  if(m_idHelperSvc->isMM(id)){
    if (!muDetMgr->getMMReadoutElement(id)->stripPosition(id,locPos)) {
        return std::nullopt;
    }
  } else if(m_idHelperSvc->issTgc(id)){
    if (!muDetMgr->getsTgcReadoutElement(id)->stripPosition(id,locPos)) {
        return std::nullopt;
    }
  } else {
    ATH_MSG_WARNING("Non NSW identifier "<<m_idHelperSvc->toString(id));
    return std::nullopt;
  }
  return locPos;
}
