/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCaloEvent/TrigT2MbtsBits.h"
#include "GaudiKernel/MsgStream.h"
#include <iostream>
#include <sstream>



TrigT2MbtsBits::TrigT2MbtsBits(const std::vector<float>& triggerEnergies, 
                               const std::vector<float>& triggerTimes):
  m_triggerEnergies(triggerEnergies),
  m_triggerTimes(triggerTimes)
{
  m_triggerEnergies.resize(TrigT2MbtsBits::NUM_MBTS,0.);
  m_triggerTimes.resize(TrigT2MbtsBits::NUM_MBTS,0);
}


// Simple debug method using cout
void TrigT2MbtsBits::print(void) const {
  std::cout << str(*this) << std::endl;
}

// Simple debug method using MsgStream
void TrigT2MbtsBits::print(MsgStream& log) const {
  std::vector<float>::const_iterator itr;
  std::vector<float>::const_iterator itr_end;

  itr=m_triggerEnergies.begin();
  itr_end=m_triggerEnergies.end();
  log << MSG::DEBUG << "m_triggerEnergies = {";
  for(;itr != itr_end; ++itr) {
    log << MSG::DEBUG << (*itr) << ",";
  }
  log << MSG::DEBUG << "}\n";  

  itr = m_triggerTimes.begin();
  itr_end = m_triggerTimes.end();
  log << MSG::DEBUG << "m_triggerTimes = {";
  for(;itr != itr_end; ++itr) {
    log << MSG::DEBUG << (*itr) << ",";
  }
  log << MSG::DEBUG << "}" << endmsg;
}

// Simple string conversion method.
std::string str(const TrigT2MbtsBits& trigT2MbtsBits) {
  std::stringstream sstream;
  std::vector<float> counterValues;
  std::vector<float>::const_iterator itr;
  std::vector<float>::const_iterator itr_end;

  counterValues = trigT2MbtsBits.triggerEnergies();
  itr = counterValues.begin();
  itr_end = counterValues.end();
  sstream << "m_triggerEnergies = {";
  for(;itr != itr_end; ++itr) {
    sstream << (*itr) << ",";
  }
  sstream << "}\n";

  counterValues = trigT2MbtsBits.triggerTimes();
  itr = counterValues.begin();
  itr_end = counterValues.end();
  sstream << "m_triggerTimes = {";
  for(;itr != itr_end; ++itr) {
    sstream << (*itr) << ", ";
  }
  sstream << "}";
  
  return sstream.str();
}

MsgStream& operator<< (MsgStream& m, const TrigT2MbtsBits& trigT2MbtsBits) {
  return (m << str(trigT2MbtsBits));
}
