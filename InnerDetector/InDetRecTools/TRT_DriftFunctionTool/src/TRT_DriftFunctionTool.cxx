/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRT_DriftFunctionTool.cxx
//   Implementation file for class TRT_DriftFunctionTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// AlgTool used to go from drift time to drift distance
///////////////////////////////////////////////////////////////////


#include "TRT_DriftFunctionTool.h"

#include "GaudiKernel/IToolSvc.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "TRT_ReadoutGeometry/TRT_Numerology.h"
#include "InDetReadoutGeometry/Version.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include <cmath> 
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
//
// Constructor
TRT_DriftFunctionTool::TRT_DriftFunctionTool(const std::string& type,
				     const std::string& name,
				     const IInterface* parent)
  : base_class(type, name, parent),
    m_TRTCalDbTool("TRT_CalDbTool",this),
    m_TRTCalDbTool2("",this),
    m_drifttimeperbin(3.125 * CLHEP::ns),
    m_error(0.17),
    m_ismc(true),
    m_isoverlay(false),
    m_istestbeam(false),
    m_dummy(false),
    m_err_fudge(1.0),
    m_allow_digi_version_override(false),
    m_forced_digiversion(11),
    m_override_simcal(false),
    m_force_universal_errors(false),
    m_uni_error(0.136),
    m_inputfile(""),
    m_key(""),
    m_trt_mgr_location("TRT"),
    m_ht_correction_barrel_Xe(0.0), // initialised from python
    m_ht_correction_endcap_Xe(0.0), // initialised from python
    m_ht_correction_barrel_Ar(0.0), // initialised from python
    m_ht_correction_endcap_Ar(0.0), // initialised from python
    m_tot_corrections_barrel_Xe(20, 0.), // initialised from python
    m_tot_corrections_endcap_Xe(20, 0.), // initialised from python
    m_tot_corrections_barrel_Ar(20, 0.), // initialised from python
    m_tot_corrections_endcap_Ar(20, 0.) // initialised from python
{
  declareProperty("IsMC",m_ismc);
  declareProperty("AllowDigiVersionOverride",m_allow_digi_version_override);
  declareProperty("ForcedDigiVersion",m_forced_digiversion);
  declareProperty("IsOverlay",m_isoverlay=false);
  declareProperty("OverrideSimulationCalibration",m_override_simcal);
  declareProperty("ForceUniversalErrors",m_force_universal_errors);
  declareProperty("UniversalError",m_uni_error);
  declareProperty("DummyMode",m_dummy);
  declareProperty("ErrorFudgeFactor",m_err_fudge);
  declareProperty("TRTCalDbTool", m_TRTCalDbTool);
  declareProperty("TRTCalDbTool2", m_TRTCalDbTool2);
  declareProperty("DriftFunctionFile", m_inputfile);
  declareProperty("TrtDescrManageLocation",m_trt_mgr_location);
  declareProperty("ToTCorrectionsBarrelXe",m_tot_corrections_barrel_Xe);
  declareProperty("ToTCorrectionsEndcapXe",m_tot_corrections_endcap_Xe);
  declareProperty("ToTCorrectionsBarrelAr",m_tot_corrections_barrel_Ar);
  declareProperty("ToTCorrectionsEndcapAr",m_tot_corrections_endcap_Ar);
  declareProperty("HTCorrectionBarrelXe",m_ht_correction_barrel_Xe);
  declareProperty("HTCorrectionEndcapXe",m_ht_correction_endcap_Xe);
  declareProperty("HTCorrectionBarrelAr",m_ht_correction_barrel_Ar);
  declareProperty("HTCorrectionEndcapAr",m_ht_correction_endcap_Ar);

  // make sure all arrays are initialized - use DC3version2 as default
  for (int i=0; i<3; i++) m_t0_barrel[i] = 15.625;
  for (int i=0; i<14; i++) m_t0_endcap[i] = 14.2;
  m_t0_shift=0.;

  for(size_t i=0; i<s_size_default; ++i){
    m_radius[i] = s_radius_default[i];
    m_errors[i] = s_errors_default[i];
  }
  for(size_t i=s_size_default; i<MaxTimeBin; ++i) {
    m_radius[i]=2.;
    m_errors[i]=m_uni_error;
  }

}

//
// Destructor--------------------------------------------------
TRT_DriftFunctionTool::~TRT_DriftFunctionTool()= default;

//
// Initialize--------------------------------------------------
StatusCode TRT_DriftFunctionTool::initialize()
{
  ATH_MSG_DEBUG( "initialize()");

  StatusCode sc = AthAlgTool::initialize();
  if (sc.isFailure())
    {
      ATH_MSG_FATAL("Cannot initialize AthAlgTool!");
      return StatusCode::FAILURE;
    } 

  if(m_dummy){
    ATH_MSG_INFO(" Drift time information ignored ");
  }

  // Retrieve TRT_DetectorManager and helper
  sc = AthAlgTool::detStore()->retrieve(m_manager, m_trt_mgr_location);
  if (sc.isFailure() || !m_manager)
  {
    ATH_MSG_FATAL("Could not find the Manager: "
		  << m_trt_mgr_location << " !");
    return sc;
  }

  // Get TRT ID helper
  sc = detStore()->retrieve(m_trtid,"TRT_ID");
  if ( sc.isFailure() ) {
    ATH_MSG_FATAL( "Could not retrieve TRT ID helper." );
    return sc;
  }

  // Check that ToT corrections have the correct length
  if (m_tot_corrections_barrel_Xe.size() != 20) {
    ATH_MSG_FATAL( "Length of ToTCorrectionsBarrelXe is not 20." );
    return sc;
  }
  if (m_tot_corrections_endcap_Xe.size() != 20) {
    ATH_MSG_FATAL( "Length of ToTCorrectionsEndcapXe is not 20." );
    return sc;
  }
  if (m_tot_corrections_barrel_Ar.size() != 20) {
    ATH_MSG_FATAL( "Length of ToTCorrectionsBarrelAr is not 20." );
    return sc;
  }
  if (m_tot_corrections_endcap_Ar.size() != 20) {
    ATH_MSG_FATAL( "Length of ToTCorrectionsEndcapAr is not 20." );
    return sc;
  }

  //
  // Get GeoModel version key
  IGeoModelSvc *geomodel;
  sc=service("GeoModelSvc",geomodel);
  if(sc.isFailure()){
    ATH_MSG_FATAL(" Could not locate GeoModelSvc ");
    return sc;
  }

  DecodeVersionKey versionKey(geomodel,"TRT");
  m_key=versionKey.tag();

  int numB = m_manager->getNumerology()->getNBarrelPhi();
  ATH_MSG_DEBUG(" Number of Barrel elements "<< numB);      
      
  m_istestbeam = numB==2;

  setupRtRelation();

  return sc;
}

//
// Finalize-----------------------------------------------------------------
StatusCode TRT_DriftFunctionTool::finalize()
{
  StatusCode sc = AlgTool::finalize();
  return sc;
}

// Drift time in ns for any non negative drift radius; Not calibrated for
// individual straws and run range, but otherwise adapted to any
// setup.
double TRT_DriftFunctionTool::approxDriftTime(double driftradius) const
{
  double t = 0.;
  int i=0;
  if(driftradius<0.100) {
    t = 2.5*m_drifttimeperbin*driftradius/0.1;
  } else if(driftradius<1.99) {
    while(driftradius>=m_radius[i]) ++i;
    if(i>0) i--;
    t=(i+0.5+(driftradius-m_radius[i])/(m_radius[i+1]-m_radius[i]))*m_drifttimeperbin;
  } else {
    t = m_drifttimeperbin*( 19. + (driftradius-1.99)/0.08 );
  }

  return t;
}

// Drift radius in mm for valid drift time in MC; zero otherwise.----------
double TRT_DriftFunctionTool::driftRadius(double drifttime) const
{
  if( !isValidTime(drifttime) ) return 0;
  int drifttimebin = std::max(int(drifttime/m_drifttimeperbin),0); 

  // Interpolate linearly 
  if(drifttime < (drifttimebin+0.5)*m_drifttimeperbin) {
    if (drifttimebin-1 > -1)
        return m_radius[drifttimebin-1]+
         (m_radius[drifttimebin]-m_radius[drifttimebin-1])*
	  (drifttime - (drifttimebin-0.5)*m_drifttimeperbin)/m_drifttimeperbin;
  } else if (drifttimebin+1 < 20) {
        return m_radius[drifttimebin]+
         (m_radius[drifttimebin+1]-m_radius[drifttimebin])*
	  (drifttime - (drifttimebin+0.5)*m_drifttimeperbin)/m_drifttimeperbin;
  }

  return m_radius[drifttimebin];
}

//
// Drift radius in mm for valid drift time (rawtime-t0) in data; --------------
// zero otherwise; truncated to at most 2mm.
double TRT_DriftFunctionTool::driftRadius(double rawtime, Identifier id, double& t0, bool& isOK, unsigned int word) const
{
  isOK = true;
  const double crawtime=rawtime - m_t0_shift; // const cast
  const Identifier cid=id;       // const cast
  t0   = 0.;
  float ft0=t0;                  //float cast

  //case of no drifttime information wanted
  if (m_dummy) return 0.;

  double radius = 0.;
  if (!m_isoverlay){ //standard case
    radius = m_TRTCalDbTool->driftRadius(crawtime,ft0,cid,isOK);
    t0 = ft0 + m_t0_shift;
  }
  else{ //overlay case
    // no m_t0_shift in rawtime, and use data TRTCalDbSvc
    radius = m_TRTCalDbTool->driftRadius(rawtime,ft0,cid,isOK);
    t0 = ft0;
    bool mcdigit = word & (1u<<31);
    if (mcdigit){
      //check if it's a MC digit, and if so apply other calibration
      ATH_MSG_DEBUG ("Overlay TRTCalDbTool  gave  radius: "<<radius<<", t0: "<<t0);
      //t0_shift in crawtime, and use MC TRTCalDbSvc(2)
      radius = m_TRTCalDbTool2->driftRadius(crawtime,ft0,cid,isOK);
      t0 = ft0 + m_t0_shift;
      ATH_MSG_DEBUG ("Overlay TRTCalDbTool2 gives radius: "<<radius<<", t0: "<<t0);
    }
  }
  double drifttime = rawtime-t0;
  if( !isValidTime(drifttime) ) isOK=false;
  return radius;

}
 
// Error of drift radius in mm -----------------------------------------------
double TRT_DriftFunctionTool::errorOfDriftRadius(double drifttime, Identifier id, float mu, unsigned int word) const
{
  if(m_dummy) return 4./std::sqrt(12.);
  if(m_force_universal_errors && m_uni_error!=0) return m_uni_error;
  bool founderr=true;
  bool foundslope=true;
  double error = m_TRTCalDbTool->driftError(drifttime,id,founderr);
  double slope = m_TRTCalDbTool->driftSlope(drifttime,id,foundslope);
  bool mcdigit = word & (1u<<31);
  if(m_isoverlay && mcdigit){
    //check if it's a MC digit, and if so apply other calibration
    ATH_MSG_DEBUG ("Overlay TRTCalDbTool gave error: "<<error<<", found="<<founderr);
    error = m_TRTCalDbTool2->driftError(drifttime,id,founderr);
    ATH_MSG_DEBUG ("Overlay TRTCalDbTool2 gives error: "<<error<<", found="<<founderr);
    ATH_MSG_DEBUG ("Overlay TRTCalDbTool gave slope: "<<slope<<", found="<<foundslope);
    slope = m_TRTCalDbTool2->driftSlope(drifttime,id,foundslope);
    ATH_MSG_DEBUG ("Overlay TRTCalDbTool2 gives slope: "<<slope<<", found="<<foundslope);
  }

  if(founderr && foundslope) {
    return error+mu*slope;
//to add condition for old setup
  }
  else if ((founderr && !foundslope)  || (mu<0)) {
		return error; }
  else {  //interpolate
    if(drifttime<=0.) {
      return m_errors[0];
    } else if(drifttime >= 18.*m_drifttimeperbin) {
      return m_errors[18];
    } else {
      float drifttimeinbins = 	drifttime/m_drifttimeperbin;
      int drifttimebin = (int)drifttimeinbins;
      float fracbin = drifttimeinbins-drifttimebin;
      return (1-fracbin)*m_errors[drifttimebin]+fracbin*m_errors[drifttimebin+1];
    }
  }
}

//
// returns the time over threshold correction in ns
double TRT_DriftFunctionTool::driftTimeToTCorrection(double tot, Identifier id, bool isArgonStraw) const
{
  int tot_index = tot/m_drifttimeperbin;
  if (tot_index < 0) tot_index = 0;
  if (tot_index > 19) tot_index = 19;

  int bec_index = std::abs(m_trtid->barrel_ec(id)) - 1;

  if (isArgonStraw) {
    return (bec_index) ? m_tot_corrections_endcap_Ar[tot_index] : m_tot_corrections_barrel_Ar[tot_index];
  }
  return (bec_index) ? m_tot_corrections_endcap_Xe[tot_index] : m_tot_corrections_barrel_Xe[tot_index];
}

// Returns high threshold correction to the drift time (ns)
double TRT_DriftFunctionTool::driftTimeHTCorrection(Identifier id, bool isArgonStraw) const
{
  int bec_index = std::abs(m_trtid->barrel_ec(id)) - 1;

  if (isArgonStraw) {
    return (bec_index) ? m_ht_correction_endcap_Ar : m_ht_correction_barrel_Ar;
  }
  return (bec_index) ? m_ht_correction_endcap_Xe : m_ht_correction_barrel_Xe;
}

//
// Initialise R-t relation ------------------------------------------
void TRT_DriftFunctionTool::setupRtRelation()
{     

  ATH_MSG_DEBUG(" Using TRTCalDbTool ");
  if ( m_TRTCalDbTool.retrieve().isFailure() ) {
    ATH_MSG_FATAL(m_TRTCalDbTool.propertyName() <<
		  ": Failed to retrieve service " << m_TRTCalDbTool.type());
    return;
    
  } else {
    ATH_MSG_DEBUG(m_TRTCalDbTool.propertyName() <<
		  ": Retrieved service " << m_TRTCalDbTool.type());
  }

  if (m_isoverlay){
    ATH_MSG_DEBUG("Using TRTCalDbTool2 for overlay ! ");
    if ( m_TRTCalDbTool2.retrieve().isFailure() ) {
      ATH_MSG_FATAL(m_TRTCalDbTool2.propertyName() <<": Failed to retrieveservice " << m_TRTCalDbTool2.type());
      return;
    }
  }
  //temporary: we need some way to automatically link digi version with db tag
  //for now we make a hack in order always to get the right t0 after having centered the
  //drifttime spectrum better in the allowed time-window with digi version 12 in release 14.

  int type = m_forced_digiversion;
  if(m_ismc || m_isoverlay){
   
    if(!m_allow_digi_version_override) {
      type = m_manager->digitizationVersion();
      ATH_MSG_DEBUG("TRT detector manager returned digitization version "<< type <<
		    " corresponding to "<< m_manager->digitizationVersionName());
    } else {
      ATH_MSG_WARNING("Digitization version chosen by user for global t0 correction: "<<type);
    }


    if(type>10) {
      m_t0_shift=-8.;
      ATH_MSG_DEBUG(" Digitization version " << type << " - T0 for barrel is shifted by "
                   << m_t0_shift);
    }

  }   
  //temporary: we need to think about how to store the uncertainties in the db!!!

  if(m_key.compare(6,4,"Comm")==0) {
    for(size_t i=0; i<s_size_Comm; ++i){
      m_radius[i] = s_radius_Comm[i];
      m_errors[i] = s_errors_Comm[i];
    }
  } else {
    for(size_t i=0; i<s_size_default; ++i){
      m_errors[i] = s_errors_default[i]*m_err_fudge;
    }
  }
    
  m_error = 0.136;
}
