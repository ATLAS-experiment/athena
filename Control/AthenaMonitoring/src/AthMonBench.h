/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class AthMonBench                         //
//                                                            //
//  Description: Helper class for taking                      //
//               per-mon-tool benchmarks of CPU and mem.      //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: June 2009                                //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef ATHMONBENCH_H
#define ATHMONBENCH_H

#include <ctime>
#include <iosfwd>
#include "GaudiKernel/IMessageSvc.h"

class AthMonBench {
public:

  static const MSG::Level s_resourceMonThreshold = MSG::DEBUG;

  AthMonBench() = default;
  ~AthMonBench() = default;

  //Modify:
  void reset();
  bool isReset() const;
  void setUnitCount();//To avoid normalising when accessing results (for "Total" kind of reports)

  //Taking data:
  void startMeasurement();
  void finishMeasurement();

  //For adding/subtracting individual measurements:
  void operator-=(const AthMonBench& o);
  void operator+=(const AthMonBench& o);

  //Access results:
  double deltaMem_mb() const;
  double deltaCPU_ms() const;
  bool valid() const;

private:
  typedef long long TMem;//bytes
  TMem m_deltaMem{};
  clock_t m_deltaCPU{};
  int m_count{};
  static TMem currentVMem();
};

std::ostream& operator << ( std::ostream& os, const AthMonBench& br);

/////////////
// Inlines //
/////////////

inline void AthMonBench::reset(){
  m_deltaMem = 0;
  m_deltaCPU = 0;
  m_count = 0;
}

inline bool AthMonBench::isReset() const {
  return !(m_deltaMem||m_deltaCPU||m_count);
}

//For creating single measurements
inline void AthMonBench::startMeasurement() {
  if (!isReset())
    m_count = -99999;
  m_deltaMem = currentVMem();
  m_deltaCPU = clock();
}

inline void AthMonBench::finishMeasurement() {
  m_deltaMem = currentVMem() - m_deltaMem;
  m_deltaCPU = clock() - m_deltaCPU;
  ++m_count;
  if (m_count!=1) {
    //Something is wrong:
    reset();
    m_count = -99999;
  }
}

//For adding/subtracting individual measurements:
inline void AthMonBench::operator-=(const AthMonBench& o) {
  m_deltaMem -= o.m_deltaMem;
  m_deltaCPU -= o.m_deltaCPU;
  m_count -= o.m_count;
}

inline void AthMonBench::operator+=(const AthMonBench& o) {
  m_deltaMem += o.m_deltaMem;
  m_deltaCPU += o.m_deltaCPU;
  m_count += o.m_count;
}

//To get results:
inline double AthMonBench::deltaMem_mb() const { return valid()?m_deltaMem/(1024.0*1024.0*m_count): -99.99; }
inline double AthMonBench::deltaCPU_ms() const { return valid()?m_deltaCPU*1.0e3/double(m_count*CLOCKS_PER_SEC) : -99.99; }
inline bool AthMonBench::valid() const { return m_count>0; }
inline void AthMonBench::setUnitCount() { if (valid()) m_count=1; }

#endif
