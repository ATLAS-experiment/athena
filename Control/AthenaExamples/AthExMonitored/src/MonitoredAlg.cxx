/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MonitoredAlg.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "TestTools/random.h"

#include <vector>
#include <cmath>

MonitoredAlg::MonitoredAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{
}

StatusCode MonitoredAlg::initialize()
{
  // Only try to retrieve the tool if one has been set
  if (!m_monTool.empty()) CHECK(m_monTool.retrieve());

  return StatusCode::SUCCESS;
}

/// Example class for monitoring
class Track {
public:
  Track(double eta, double phi, double pt) : m_eta(eta), m_phi(phi), m_pt(pt) {}
  double eta() const { return m_eta; }
  double phi() const { return m_phi; }
  double pt() const { return m_pt; }
private:
  double m_eta{0}, m_phi{0}, m_pt{0};
};


StatusCode MonitoredAlg::execute() 
{
  std::vector<Track> tracks;

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  auto count = Monitored::Scalar<int>("nTracks", 0);  // explicit type

  // Access via member
  auto eta = Monitored::Collection("eta", tracks, &Track::eta);

  // Access via function/lambda
  auto absphi = Monitored::Collection("AbsPhi", tracks, []( const Track& t ) { return abs(t.phi()); });

  auto mon = Monitored::Group(m_monTool, count, eta, absphi, timer);

  count = Athena_test::randi_seed (m_seed, 11, 1);   // random number of tracks

  // Creating some random tracks
  for (int i=0; i<count; ++i) {
    float eta = Athena_test::randf_seed (m_seed, 3.0, -3.0);
    float phi = Athena_test::randf_seed (m_seed, M_PI, -M_PI);
    float pt  = Athena_test::randf_seed (m_seed, 100);
    tracks.push_back(Track(eta, phi, pt));
  }
  
  return StatusCode::SUCCESS;
}



