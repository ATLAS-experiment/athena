/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*********************************************************************
    SeedNewtonTrkDistanceFinder.cxx - Description in header file
*********************************************************************/

//#define SEEDNEWTONTRKDISTANCEFINDER_DEBUG


#include "TrkVertexSeedFinderUtils/SeedNewtonTrkDistanceFinder.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkParticleBase/TrackParticleBase.h"
#include "TrkTrack/Track.h"
#include "TrkVertexSeedFinderUtils/NewtonTrkDistanceFinder.h"
#include "TrkVertexSeedFinderUtils/SeedFinderParamDefs.h"
#include "TrkVertexSeedFinderUtils/Trk2dDistanceSeeder.h"
#include <cmath>


namespace Trk
{


  SeedNewtonTrkDistanceFinder::SeedNewtonTrkDistanceFinder(const std::string& t, const std::string& n, const IInterface*  p) :
    base_class(t,n,p),
    m_2ddistanceseeder("Trk::Trk2dDistanceSeeder"),
    m_distancefinder("Trk::NewtonTrkDistanceFinder"),
    m_numberOfMinimizationFailures(0)
  {
    declareProperty("Trk2dDistanceSeeder",     m_2ddistanceseeder);
    declareProperty("TrkDistanceFinderImplementation",     m_distancefinder);
  }

  SeedNewtonTrkDistanceFinder::~SeedNewtonTrkDistanceFinder() = default;

  StatusCode SeedNewtonTrkDistanceFinder::initialize()
  {

    //initialize number of failures to 0
    m_numberOfMinimizationFailures=0;

    ATH_CHECK( AlgTool::initialize() );
    ATH_CHECK( m_2ddistanceseeder.retrieve() );
    ATH_CHECK( m_distancefinder.retrieve() );
    ATH_MSG_DEBUG( "Initialize successful" );
    return StatusCode::SUCCESS;
  }

  StatusCode SeedNewtonTrkDistanceFinder::finalize()
  {

    ATH_MSG_DEBUG( "Finalize successful. Number of failed minimizations: " << m_numberOfMinimizationFailures << ". Few per events is OK!" );
    return StatusCode::SUCCESS;
  }


  /** method to do the calculation starting from two MeasuredPerigees*/
  /** return value is true if calculation is successfull */
  std::optional<ITrkDistanceFinder::TwoPoints>
  SeedNewtonTrkDistanceFinder::CalculateMinimumDistance(const Trk::Perigee & a,
                                                        const Trk::Perigee & b)  const
  {
    //defragmenting the meory: local variable instead of private data member
    std::pair<PointOnTrack,PointOnTrack> minpoints;

    //try first to get the minimum directly with the Newton method
    auto ret = m_distancefinder->GetClosestPoints(a,b);
    if (std::holds_alternative<TwoPoints>(ret)) {
      return std::get<TwoPoints>(ret);
    }
    ATH_MSG_DEBUG( "Problem with Newton finder: " <<
                   std::get<std::string>(ret) );
    minpoints = m_2ddistanceseeder->GetSeed(TwoTracks(a,b));
    ret = m_distancefinder->GetClosestPoints(minpoints);
    if (std::holds_alternative<TwoPoints>(ret)) {
      return std::get<TwoPoints>(ret);
    }
    ATH_MSG_DEBUG( "Problem with Newton finder, even after 2d seeder: no minimum between tracks found" << std::get<std::string>(ret));
    m_numberOfMinimizationFailures+=1;

    return std::nullopt;
  }


  /** method to do the calculation starting from two tracks */
  std::optional<ITrkDistanceFinder::TwoPoints>
  SeedNewtonTrkDistanceFinder::CalculateMinimumDistance(const Trk::Track & a,
                                                        const Trk::Track & b) const
  {
    if (std::isnan(a.perigeeParameters()->parameters()[Trk::d0])||std::isnan(b.perigeeParameters()->parameters()[Trk::d0])) {
      ATH_MSG_ERROR( "Nan parameters in tracks. Cannot use them" );
      return std::nullopt;
    }

    return CalculateMinimumDistance(*(a.perigeeParameters()),*(b.perigeeParameters()));

  }

} // namespace Trk
