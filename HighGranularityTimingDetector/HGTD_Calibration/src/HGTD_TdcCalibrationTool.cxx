/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/src/HGTD_TdcCalibrationTool.cxx
 *
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 *
 * @date May, 2025
 */

#include "HGTD_Calibration/HGTD_TdcCalibrationTool.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "GaudiKernel/PhysicalConstants.h"

HGTD_TdcCalibrationTool::HGTD_TdcCalibrationTool(const std::string &type,
                                            const std::string &name,
                                            const IInterface *parent)
    : AthAlgTool(type, name, parent)
{}

// StatusCode HGTD_TdcCalibrationTool::initialize() {
//   return StatusCode::SUCCESS;
// }

float HGTD_TdcCalibrationTool::activeWindowUpperBound(const InDetDD::SolidStateDetectorElementBase* element) const{

  //NB this "expected time" will change once we need to follow the beamspot!!
  float hit_time_expected = element->center().norm() / Gaudi::Units::c_light;

  //Should never happen, but testing anyway
  if(hit_time_expected > m_lhc_rise_edge + m_active_window/2){
    ATH_MSG_DEBUG("expected hit above calibration range");
    return m_lhc_rise_edge.value() + m_active_window;
  }
  else if(hit_time_expected < m_lhc_rise_edge - 1.5*m_active_window){
    ATH_MSG_DEBUG("expected hit below calibration range");
    return m_lhc_rise_edge.value() - m_active_window;    
  }

  float window_walk = (hit_time_expected + m_active_window/2) - m_lhc_rise_edge;

  int window_steps;
  float window_upper_bound;

  if(window_walk > m_ps_large_step){
    window_steps = (window_walk - m_ps_large_step)/m_ps_small_step;
    window_upper_bound = m_lhc_rise_edge.value() + m_ps_large_step + window_steps*m_ps_small_step;
  }
  else if(window_walk < - m_ps_large_step){
    window_steps = (window_walk + m_ps_large_step)/m_ps_small_step;
    window_upper_bound = m_lhc_rise_edge.value() - m_ps_large_step + window_steps*m_ps_small_step;
  }
  else{
    window_steps = window_walk/m_ps_small_step;
    window_upper_bound = m_lhc_rise_edge.value() + window_steps*m_ps_small_step;
  }

  return window_upper_bound;
}


uint8_t HGTD_TdcCalibrationTool::Time2TOA(const InDetDD::SolidStateDetectorElementBase* element, float hit_time) const {
  
  float  window_upper_bound = activeWindowUpperBound(element);
  
  //TOA time will be the distance between hit_time and upper bound of active window 
  float tdc_time =  window_upper_bound - hit_time;

  // Check if hit is within the measurement window, if not return overflow flag
  if( tdc_time < 0 || tdc_time > 2.5){
    ATH_MSG_DEBUG("charge at " << hit_time
                                << " outside of TOA TDC range ["
                                << window_upper_bound - 2.5 << ", " 
                                << window_upper_bound << "]" );
   return HGTD::TOA_OVERLFLOW_MASK;
  }

  //TOOD: Include TOA TDC bin size smearing here, for now only using nominal value
  uint8_t toa = tdc_time/m_toa_bin_size;

  ATH_MSG_DEBUG("hit time: "<< hit_time << 
    " hit time digitized: " << TOA2Time(element, toa) << 
    " TOA: " <<  static_cast<unsigned int>(toa));
  
  return toa; 

}

float HGTD_TdcCalibrationTool::TOA2Time(const InDetDD::SolidStateDetectorElementBase* element, uint8_t toa) const{
  // Using middle of bin as estimate to recover the digitized time
  return activeWindowUpperBound(element) - (toa + 0.5)*m_toa_bin_size;
}