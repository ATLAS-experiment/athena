/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkStripAmp.h"

//STD includes
#include <cmath>
#include <fstream>
#include "GaudiKernel/SystemOfUnits.h"



//----------------------------------------------------------------------
// Initialize
//----------------------------------------------------------------------
StatusCode ITkStripAmp::initialize() {
  StatusCode sc{AthAlgTool::initialize()};
  m_PeakTime.setValue(m_PeakTime.value() * Gaudi::Units::ns);
  m_NormConstCentral = 1.0;
  return sc;
}

//----------------------------------------------------------------------
// Amplifier impulse response is now CR-RC^3 
//----------------------------------------------------------------------
float ITkStripAmp::response(const list_t& /*Charges*/, const float /*timeOfThreshold*/) const {
  float resp{1.0f};
  return resp;
}

void ITkStripAmp::response(const list_t& Charges, const float time, std::vector<float>& response) const {
  auto bin_max{std::ssize(response)};
  std::fill(response.begin(), response.end(), 0.0);
  float tp{m_PeakTime/3.0f}; // for CR-RC^3
  for (const SiCharge& charge: Charges) {
    float ch{static_cast<float>(charge.charge())};
    float ch_time{static_cast<float>(charge.time())};
    auto bin_end{bin_max-1};
    for (int bin{-1}; bin<bin_end; ++bin) {
      float bin_time{time + bin*25};//25, fix me
      float tC{bin_time - ch_time};
      if (tC > 0.0f) {
        tC/=tp; //to avoid doing it four times
        response[bin+1] += ch*tC*tC*tC*std::exp(-tC); //faster than pow
      }
    }
  }
  for (int bin{0}; bin<bin_max; ++bin) response[bin] = response[bin]*m_NormConstCentral;
}

// ----------------------------------------------------------------------
// Crosstalk on the neighbour strip
// ----------------------------------------------------------------------
float ITkStripAmp::crosstalk(const list_t& /*Charges*/, const float /*timeOfThreshold*/) const {
  float resp{1};
  return resp;
}

void ITkStripAmp::crosstalk(const list_t& /*Charges*/, const float /*timeOfThreshold*/, std::vector<float>& response) const {
  std::fill(response.begin(), response.end(), 1.0);
}

