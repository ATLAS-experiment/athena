/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CompetingMuonClustersOnTrack.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "GaudiKernel/MsgStream.h"
// Muon
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
// std
#include <cmath>
#include <ranges>

namespace Muon {

// copy constructor
CompetingMuonClustersOnTrack::CompetingMuonClustersOnTrack(const CompetingMuonClustersOnTrack& compROT):
   CompetingMuonClustersOnTrack{} {
    (*this) = compROT;
}

// explicit constructor
CompetingMuonClustersOnTrack::CompetingMuonClustersOnTrack(
  std::vector<std::unique_ptr<const MuonClusterOnTrack>>&& childrots,
  std::vector<AssignmentProb>&& assgnProb)
  : Trk::CompetingRIOsOnTrack(std::move(assgnProb))
  , m_containedChildRots(std::move(childrots))
{
  setLocalParametersAndErrorMatrix();
}

CompetingMuonClustersOnTrack::CompetingMuonClustersOnTrack(
  Trk::LocalParameters&& locPars,
  Amg::MatrixX&& error,
  const Trk::Surface* assSurf,
  std::vector<std::unique_ptr<const MuonClusterOnTrack>>&& childrots,
  std::vector<AssignmentProb>&& assgnProb)
  : Trk::CompetingRIOsOnTrack{std::move(assgnProb)}
  , Trk::SurfacePtrHolderDetEl{assSurf}
  , m_containedChildRots(std::move(childrots))
{
  Trk::MeasurementBase::m_localParams = std::move(locPars);
  Trk::MeasurementBase::m_localCovariance = std::move(error);
}

CompetingMuonClustersOnTrack&
CompetingMuonClustersOnTrack::operator=(
  const CompetingMuonClustersOnTrack& compROT) noexcept
{
  if (this != &compROT) {
    // assingment operator of base class
    Trk::CompetingRIOsOnTrack::operator=(compROT);
    Trk::SurfacePtrHolderDetEl::operator=(compROT);
    m_globalPosition.release();
    m_containedChildRots.clear();
    std::ranges::transform(compROT.m_containedChildRots, 
                           std::back_inserter(m_containedChildRots),
                           [](const std::unique_ptr<const MuonClusterOnTrack>& cluster) {
                              return std::unique_ptr<const MuonClusterOnTrack>{cluster->clone()};
                           });

  }
  return (*this);
}


MsgStream&
CompetingMuonClustersOnTrack::dump(MsgStream& out) const
{
  out << "Muon::CompetingMuonClustersOnTrack (Muon competingROTs) "
      << std::endl;
  out << "  - it contains   : " << m_containedChildRots.size()
      << " RIO_OnTrack objects" << std::endl;
  out << "  - parameters     : " << std::endl;
  out << "  - parameter key : " << std::endl;
  return out;
}

std::ostream&
CompetingMuonClustersOnTrack::dump(std::ostream& out) const
{
  out << "Muon::CompetingMuonClustersOnTrack (Muon competingROTs) "
      << std::endl;
  out << "  - it contains   : " << m_containedChildRots.size()
      << " RIO_OnTrack objects" << std::endl;
  out << "  - it contains   : " << numberOfContainedROTs()
      << " RIO_OnTrack objects" << std::endl;
  out << "  - parameters     : " << std::endl;
  out << "  - parameter key : " << std::endl;
  return out;
}

// Have all the contained ROTs a common associated surface?
bool
CompetingMuonClustersOnTrack::ROTsHaveCommonSurface(const bool) const
{
  return true;
}

}
