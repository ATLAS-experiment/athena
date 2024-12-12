/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#ifndef EVGENPRODTOOLS_SIMTIMEESTIMATE_H
#define EVGENPRODTOOLS_SIMTIMEESTIMATE_H

#include "GeneratorModules/GenBase.h"
#include <array>
#include <vector>

/// Algorithm to estimate the amount of CPU time that simulation will take
///
/// This algorithm runs on generated events and attempts to estimate
/// the amount of CPU time that the simulation will require to run.
///
class SimTimeEstimate : public GenBase {
public:

  SimTimeEstimate(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

 private:
  Gaudi::Property<double> m_etaMax{this, "EtaLimit", 6.0, "Max eta used for simulation"};

  int         m_total_Events{}; //!< Number of events that I've seen
  double      m_total_Energy{}; //!< Amount of energy that I've seen
  double      m_eventEnergy{}; //!< Amount of energy that I've seen in one event
  std::vector<double>   m_particleEtas; //!< list holds eta of each particle
  std::vector<double>   m_particleEnergies; //!< list holds energy of each particle
  std::vector<int> m_particleIDs; //!< list holds all the particle IDs for one event
  int         m_particleNumber{}; //!< Number of particles in one event
  const std::array<int, 4> m_pidsToSkip{12, 13, 14, 16}; // !< PIDs of particles which should not be taken into account
  
};


#endif

#endif
