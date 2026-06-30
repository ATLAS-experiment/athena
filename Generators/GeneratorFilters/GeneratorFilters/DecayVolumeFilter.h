/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef DECAYFILTER_DECAYVOLUMEFILTER_H
#define DECAYFILTER_DECAYVOLUMEFILTER_H

#include "GaudiKernel/ServiceHandle.h"
#include "GeneratorModules/GenFilter.h"

#include <vector>
#include <map>

class DecayVolumeFilter : public GenFilter {
 public:
  
  DecayVolumeFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterInitialize();
  virtual StatusCode filterEvent(const EventContext& ctx);
  virtual StatusCode filterFinalize();
  bool accept_flatbinning(float r);
  void reportBinStatistics() const;
  
 private:

  Gaudi::Property<double>   m_RCutMax    {this, "RCutMax",           400., "Max radius of acceptance region"};
  Gaudi::Property<double>   m_RCutMin    {this, "RCutMin",           400., "Min radius of acceptance region"};
  Gaudi::Property<double>   m_zCutMax    {this, "zCutMax",           400., "Max z-coordinate of acceptance region"};
  Gaudi::Property<double>   m_zCutMin    {this, "zCutMin",           400., "Min z-coordinate of acceptance region"};
  Gaudi::Property<double>   m_MaxAbsEta  {this, "MaxAbsEta",           5., "Restrict acceptance region to certain eta"};
  Gaudi::Property<int>      m_LLP_PDGID  {this, "LLP_PDG",             36, "LLP PDG ID"};
  Gaudi::Property<int>      m_MinPass    {this, "MinPass",               2, "Minimum number of decays in specified volume in the event"};
  Gaudi::Property<unsigned int>    m_nBins      {this, "nBins",                 0, "Number of bins for decay radius"};
  Gaudi::Property<int>      m_nEvents    {this, "nEvents",            1000, "Number of requested events in total"};
  Gaudi::Property<float>    m_Rmargin    {this, "DecayRadiusMargin", 100.f, "Leave out the last part of the decay radius to improve filter efficiency"};

  unsigned int m_eventCounter{0}; // counter for events seen
  int m_maxEntries{0}; // maximum number of events per bin
  std::map<int, int> m_stats; 
  std::vector<float> m_bin_edges;
  std::map<int, int> m_decay_radius; // map of bin number to number of events inside this bin
};

#endif
