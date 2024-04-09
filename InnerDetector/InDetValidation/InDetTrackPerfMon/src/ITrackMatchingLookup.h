/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_ITRACKMATCHINGLOOKUP_H
#define INDETTRACKPERFMON_ITRACKMATCHINGLOOKUP_H

/**
 * @file ITrackMatchingLookup.h
 * @brief Interace for TrackMatchingLookup objects (templated)
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 21 March 2024
**/


/// Athena include(s)
#include "AsgMessaging/StatusCode.h"

/// STL include(s)
#include <string>
#include <vector>

/// xAOD includes
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"


namespace IDTPM {

  class ITrackMatchingLookup {

  public:

    /// Destructor
    virtual ~ITrackMatchingLookup() = default;

    /// get overall number of matches
    virtual unsigned getNmatches() const = 0;

    /// matching properties
    const std::string& anaTag() const { return m_anaTag; }
    void anaTag( std::string_view anaTag_s ) {
      m_anaTag = anaTag_s;
    }
    const std::string& chainRoiName() const { return m_chainRoiName; }
    void chainRoiName( std::string_view chainRoiName_s ) {
      m_chainRoiName = chainRoiName_s;
    }

    /// get matched reference (1 to 1)
    /// Track -> Track
    virtual const xAOD::TrackParticle* getMatchedRefTrack(
        const xAOD::TrackParticle& t ) const = 0;
    /// Truth -> Track
    virtual const xAOD::TrackParticle* getMatchedRefTrack(
        const xAOD::TruthParticle& t ) const = 0;
    /// Track -> Truth
    virtual const xAOD::TruthParticle* getMatchedRefTruth(
        const xAOD::TrackParticle& t ) const = 0;

    /// get matched test vector (1 to 1+)
    /// vec Track <- Track
    virtual const std::vector< const xAOD::TrackParticle* >& getMatchedTestTracks(
        const xAOD::TrackParticle& r ) const = 0;
    /// vec Track <- Truth
    virtual const std::vector< const xAOD::TrackParticle* >& getMatchedTestTracks(
        const xAOD::TruthParticle& r ) const = 0;
    /// vec Truth <- Track
    virtual const std::vector< const xAOD::TruthParticle* >& getMatchedTestTruths(
        const xAOD::TrackParticle& r ) const = 0;

    /// return true if test is matched
    virtual bool isTestMatched( const xAOD::TrackParticle& t ) const = 0;
    virtual bool isTestMatched( const xAOD::TruthParticle& t ) const = 0;

    /// return true if reference is matched 
    virtual bool isRefMatched( const xAOD::TrackParticle& r ) const = 0;
    virtual bool isRefMatched( const xAOD::TruthParticle& r ) const = 0;

    /// update lookup tables with a new entry
    /// Track -> Track
    virtual StatusCode update( const xAOD::TrackParticle& t,
                               const xAOD::TrackParticle& r ) = 0;
    /// Track -> Truth
    virtual StatusCode update( const xAOD::TrackParticle& t,
                               const xAOD::TruthParticle& r ) = 0;
    /// Truth -> Track
    virtual StatusCode update( const xAOD::TruthParticle& t,
                               const xAOD::TrackParticle& r ) = 0;

    /// clear lookup tables
    virtual void clear() = 0;

    /// print info about matching and reverse matchings
    /// Track -> Track
    virtual std::string printInfo(
        const std::vector< const xAOD::TrackParticle* >& testVec,
        const std::vector< const xAOD::TrackParticle* >& refVec ) const = 0;
    /// Track -> Truth
    virtual std::string printInfo(
        const std::vector< const xAOD::TrackParticle* >& testVec,
        const std::vector< const xAOD::TruthParticle* >& refVec ) const = 0;
    /// Truth -> Track
    virtual std::string printInfo(
        const std::vector< const xAOD::TruthParticle* >& testVec,
        const std::vector< const xAOD::TrackParticle* >& refVec ) const = 0;

  protected:

    /// null vectors
    std::vector< const xAOD::TrackParticle* > m_nullTrackVec{};
    std::vector< const xAOD::TruthParticle* > m_nullTruthVec{};

  private:

    /// Lookup table properties
    std::string m_anaTag;
    std::string m_chainRoiName;
    

  }; // class ITrackMatchingLookup

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_ITRACKMATCHINGLOOKUP_H
