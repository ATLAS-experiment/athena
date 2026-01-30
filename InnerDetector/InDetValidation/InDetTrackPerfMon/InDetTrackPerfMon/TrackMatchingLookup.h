/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKMATCHINGLOOKUP_H
#define INDETTRACKPERFMON_TRACKMATCHINGLOOKUP_H

/**
 * @file TrackMatchingLookup.h
 * @brief Look-up table (templated) class to store (internally to IDTPM)
 *        all the matches between test && reference tracks && vice versa 
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 21 March 2024
**/

/// Athena include(s)
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMessaging.h"

/// STL include(s)
#include <unordered_map>

// local includes
#include "InDetTrackPerfMon/ITrackMatchingLookup.h"

namespace IDTPM {

  /// ---------------------------------------------
  /// ---------- Base (templated) Lookup ----------
  /// ---------------------------------------------
  template< typename T, typename R=T > 
  class TrackMatchingLookupBase : public AthMessaging {

  public:

    /// useful typedefs
    typedef std::unordered_map< const T*, const R* > mapTtoR_t;
    typedef std::unordered_map< const R*, std::vector<const T*> > mapRtoT_t;
    typedef std::unordered_map< const T*, float > mapTtoDist_t;

    /// Constructor
    TrackMatchingLookupBase( const std::string& anaTag_s );

    /// Destructor
    ~TrackMatchingLookupBase() = default;

    /// = operator (disabled)
    TrackMatchingLookupBase& operator=( const TrackMatchingLookupBase<T,R>& ) = delete;

    /// get the overall number of matches
    unsigned getMapsSize( bool getRefN = false ) const {
      return getRefN ? m_mapRefToTest.size() : m_mapTestToRef.size(); }

    /// get matched reference from map
    const R* getMatchedRef( const T& t ) const;

    /// get matched test vector from map
    const std::vector<const T*>& getMatchedTest( const R& r ) const;

    /// get distance parameter for matched test
    float getDist( const T& t ) const;

    /// return true if test is matched 
    bool isTestInMaps( const T& t ) const;

    /// return true if reference is matched 
    bool isRefInMaps( const R& r ) const;

    /// update maps with a new entry
    StatusCode updateMaps( const T& t, const R& r, float dist = 0. );

    /// clear lookup tables
    void clearMaps();

    /// print info about matching && reverse matchings
    std::string printMaps( const std::vector<const T*>& testVec,
                           const std::vector<const R*>& refVec,
                           std::string_view chainRoiName_s ) const;

  private:

    /// analysis tag
    std::string m_anaTag;

    /// Mapping test to its uniquely associated reference
    mapTtoR_t m_mapTestToRef;

    /// Mapping reference to its (possibly multiple) associated test(s)
    mapRtoT_t m_mapRefToTest;

    /// Mapping test to its uniquely associated reference
    mapTtoDist_t m_mapTestToDist;

    /// null Vectors
    std::vector<const T*> m_nullTest{};

  }; // class TrackMatchingLookupBase


  /// -------------------------------------------
  /// ---------- Track -> Track Lookup ----------
  /// -------------------------------------------
  class TrackMatchingLookup_trk :
      public TrackMatchingLookupBase< xAOD::TrackParticle >,
      public virtual ITrackMatchingLookup {

  public:

    /// Constructor
    TrackMatchingLookup_trk( const std::string& anaTag_s ) :
        TrackMatchingLookupBase< xAOD::TrackParticle >( anaTag_s ) {
      anaTag( anaTag_s );
    }

    /// Destructor
    ~TrackMatchingLookup_trk() = default;

    /// getNmatches
    virtual unsigned getNmatches( bool getRefN ) const override { return getMapsSize( getRefN ); }

    /// getMatchedRefTrack
    virtual const xAOD::TrackParticle* getMatchedRefTrack(
        const xAOD::TrackParticle& t ) const override { return getMatchedRef(t); }

    virtual const xAOD::TrackParticle* getMatchedRefTrack(
        const xAOD::TruthParticle& ) const override { 
      ATH_MSG_WARNING( "getMatchedRefTrack: Truth->Track disabled" );
      return nullptr;
    }

    virtual const xAOD::TruthParticle* getMatchedRefTruth(
        const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedRefTruth: Track->Truth disabled" );
      return nullptr;
    }

    /// getMatchedTestTracks
    virtual const std::vector< const xAOD::TrackParticle* >& getMatchedTestTracks(
        const xAOD::TrackParticle& r ) const override { return getMatchedTest(r); }

    virtual const std::vector< const xAOD::TrackParticle* >& getMatchedTestTracks(
        const xAOD::TruthParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedTestTracks: Tracks<-Truth disabled" );
      return m_nullTrackVec;
    }

    virtual const std::vector< const xAOD::TruthParticle* >& getMatchedTestTruths(
        const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedTestTruths: Truths<-Track disabled" );
      return m_nullTruthVec;
    }

    /// isTestMatched
    virtual bool isTestMatched( const xAOD::TrackParticle& t ) const override {
      return isTestInMaps(t);
    }

    virtual bool isTestMatched( const xAOD::TruthParticle& ) const override {
      ATH_MSG_WARNING( "isTestMatched: Truth->X disabled" );
      return false;
    }

    /// isRefMatched
    virtual bool isRefMatched( const xAOD::TrackParticle& r ) const override {
      return isRefInMaps(r);
    }

    virtual bool isRefMatched( const xAOD::TruthParticle& ) const override {
      ATH_MSG_WARNING( "isRefMatched: X<-Truth disabled" );
      return false;
    }

    /// update
    virtual StatusCode update( const xAOD::TrackParticle& t,
                               const xAOD::TrackParticle& r,
                               float dist ) override {
      ATH_CHECK( updateMaps( t, r, dist ) );
      return StatusCode::SUCCESS;
    }

    virtual StatusCode update( const xAOD::TrackParticle&,
                               const xAOD::TruthParticle&,
                               float ) override {
      ATH_MSG_WARNING( "update: Track->Truth disabled" );
      return StatusCode::SUCCESS;
    }

    virtual StatusCode update( const xAOD::TruthParticle&,
                               const xAOD::TrackParticle&,
                               float ) override {
      return StatusCode::SUCCESS;
      ATH_MSG_WARNING( "update: Truth->Track disabled" );
    }

    /// clear
    virtual void clear() override { 
      ATH_MSG_DEBUG( "Deleting whole cache..." );
      chainRoiName("");
      clearMaps();
    }

    /// printInfo
    virtual std::string printInfo(
        const std::vector< const xAOD::TrackParticle* >& testVec,
        const std::vector< const xAOD::TrackParticle* >& refVec ) const override {
      return printMaps( testVec, refVec, chainRoiName() );
    }

    virtual std::string printInfo(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TruthParticle* >& ) const override {
      ATH_MSG_WARNING( "printInfo: Track->Truth matches disabled" );
      return std::string("--> No matches found");
    }

    virtual std::string printInfo(
        const std::vector< const xAOD::TruthParticle* >&,
        const std::vector< const xAOD::TrackParticle* >& ) const override {
      ATH_MSG_WARNING( "printInfo: Truth->Track matches disabled" );
      return std::string("--> No matches found");
    }

  }; // class TrackMatchingLookup_trk


  /// -------------------------------------------
  /// ---------- Track -> Truth Lookup ----------
  /// -------------------------------------------
  class TrackMatchingLookup_trkTruth :
      public TrackMatchingLookupBase< xAOD::TrackParticle, xAOD::TruthParticle >,
      public virtual ITrackMatchingLookup {

  public:

    /// Constructor
    TrackMatchingLookup_trkTruth( const std::string& anaTag_s ) :
        TrackMatchingLookupBase< xAOD::TrackParticle, xAOD::TruthParticle >( anaTag_s ) {
      anaTag( anaTag_s );
    }

    /// Destructor
    ~TrackMatchingLookup_trkTruth() = default;

    /// getNmatches
    virtual unsigned getNmatches( bool getRefN ) const override { return getMapsSize( getRefN ); }

    /// getMatchedRefTrack
    virtual const xAOD::TrackParticle* getMatchedRefTrack(
        const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedRefTrack: Track->Track disabled" );
      return nullptr;
    }

    virtual const xAOD::TrackParticle* getMatchedRefTrack(
        const xAOD::TruthParticle& ) const override { 
      ATH_MSG_WARNING( "getMatchedRefTrack: Truth->Track disabled" );
      return nullptr;
    }

    virtual const xAOD::TruthParticle* getMatchedRefTruth(
        const xAOD::TrackParticle& t) const override { return getMatchedRef(t); }

    /// getMatchedTestTracks
    virtual const std::vector< const xAOD::TrackParticle* >& getMatchedTestTracks(
        const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedTestTracks: Track<-Track disabled" );
      return m_nullTrackVec;
    }

    virtual const std::vector< const xAOD::TrackParticle* >& getMatchedTestTracks(
        const xAOD::TruthParticle& r ) const override { return getMatchedTest(r); }

    virtual const std::vector< const xAOD::TruthParticle* >& getMatchedTestTruths(
        const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedTestTruths: Truths<-Track disabled" );
      return m_nullTruthVec;
    }

    /// isTestMatched
    virtual bool isTestMatched( const xAOD::TrackParticle& t ) const override {
      return isTestInMaps(t);
    }

    virtual bool isTestMatched( const xAOD::TruthParticle& ) const override {
      ATH_MSG_WARNING( "isTestMatched: Truth->X disabled" );
      return false;
    }

    /// isRefMatched
    virtual bool isRefMatched( const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "isRefMatched: X<-Track disabled" );
      return false;
    }

    virtual bool isRefMatched( const xAOD::TruthParticle& r ) const override {
      return isRefInMaps(r);
    }

    /// update
    virtual StatusCode update( const xAOD::TrackParticle&,
                               const xAOD::TrackParticle&,
                               float ) override {
      ATH_MSG_WARNING( "update: Track->Track disabled" );
      return StatusCode::SUCCESS;
    }

    virtual StatusCode update( const xAOD::TrackParticle& t,
                               const xAOD::TruthParticle& r,
                               float dist ) override {
      ATH_CHECK( updateMaps( t, r, dist ) );
      return StatusCode::SUCCESS;
    }

    virtual StatusCode update( const xAOD::TruthParticle&,
                               const xAOD::TrackParticle&,
                               float ) override {
      return StatusCode::SUCCESS;
      ATH_MSG_WARNING( "update: Truth->Track disabled" );
    }

    /// clear
    virtual void clear() override { 
      ATH_MSG_DEBUG( "Deleting whole cache..." );
      chainRoiName("");
      clearMaps();
    }

    /// printInfo
    virtual std::string printInfo(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TrackParticle* >& ) const override {
      ATH_MSG_WARNING( "printInfo: Track->Track matches disabled" );
      return std::string("--> No matches found");
    }

    virtual std::string printInfo(
        const std::vector< const xAOD::TrackParticle* >& testVec,
        const std::vector< const xAOD::TruthParticle* >& refVec ) const override {
      return printMaps( testVec, refVec, chainRoiName() );
    }

    virtual std::string printInfo(
        const std::vector< const xAOD::TruthParticle* >&,
        const std::vector< const xAOD::TrackParticle* >& ) const override {
      ATH_MSG_WARNING( "printInfo: Truth->Track matches disabled" );
      return std::string("--> No matches found");
    }

  }; // class TrackMatchingLookup_trkTruth


  /// -------------------------------------------
  /// ---------- Truth -> Track Lookup ----------
  /// -------------------------------------------
  class TrackMatchingLookup_truthTrk :
      public TrackMatchingLookupBase< xAOD::TruthParticle, xAOD::TrackParticle >,
      public virtual ITrackMatchingLookup {

  public:

    /// Constructor
    TrackMatchingLookup_truthTrk( const std::string& anaTag_s ) :
        TrackMatchingLookupBase< xAOD::TruthParticle, xAOD::TrackParticle >( anaTag_s ) {
      anaTag( anaTag_s );
    }

    /// Destructor
    ~TrackMatchingLookup_truthTrk() = default;

    /// getNmatches
    virtual unsigned getNmatches( bool getRefN ) const override { return getMapsSize( getRefN ); }

    /// getMatchedRefTrack
    virtual const xAOD::TrackParticle* getMatchedRefTrack(
        const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedRefTrack: Track->Track disabled" );
      return nullptr;
    }

    virtual const xAOD::TrackParticle* getMatchedRefTrack(
        const xAOD::TruthParticle& t ) const override { return getMatchedRef(t); }

    virtual const xAOD::TruthParticle* getMatchedRefTruth(
        const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedRefTruth: Track->Truth disabled" );
      return nullptr;
    }

    /// getMatchedTestTracks
    virtual const std::vector< const xAOD::TrackParticle* >& getMatchedTestTracks(
        const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedTestTruths: Track<-Track disabled" );
      return m_nullTrackVec;
    }

    virtual const std::vector< const xAOD::TrackParticle* >& getMatchedTestTracks(
        const xAOD::TruthParticle& ) const override {
      ATH_MSG_WARNING( "getMatchedTestTracks: Tracks<-Truth disabled" );
      return m_nullTrackVec;
    }

    virtual const std::vector< const xAOD::TruthParticle* >& getMatchedTestTruths(
        const xAOD::TrackParticle& r ) const override { return getMatchedTest(r); }

    /// isTestMatched
    virtual bool isTestMatched( const xAOD::TrackParticle& ) const override {
      ATH_MSG_WARNING( "isTestMatched: Track->X disabled" );
      return false;
    }

    virtual bool isTestMatched( const xAOD::TruthParticle& t ) const override {
      return isTestInMaps(t);
    }

    /// isRefMatched
    virtual bool isRefMatched( const xAOD::TrackParticle& r ) const override {
      return isRefInMaps(r);
    }

    virtual bool isRefMatched( const xAOD::TruthParticle& ) const override {
      ATH_MSG_WARNING( "isRefMatched: X<-Truth disabled" );
      return false;
    }

    /// update
    virtual StatusCode update( const xAOD::TrackParticle&,
                               const xAOD::TrackParticle&,
                               float ) override {
      return StatusCode::SUCCESS;
      ATH_MSG_WARNING( "update: Track->Track disabled" );
    }

    virtual StatusCode update( const xAOD::TrackParticle&,
                               const xAOD::TruthParticle&,
                               float ) override {
      ATH_MSG_WARNING( "update: Track->Truth disabled" );
      return StatusCode::SUCCESS;
    }

    virtual StatusCode update( const xAOD::TruthParticle& t,
                               const xAOD::TrackParticle& r,
                               float dist ) override {
      ATH_CHECK( updateMaps( t, r, dist ) );
      return StatusCode::SUCCESS;
    }

    /// clear
    virtual void clear() override { 
      ATH_MSG_DEBUG( "Deleting whole cache..." );
      chainRoiName("");
      clearMaps();
    }

    /// printInfo
    virtual std::string printInfo(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TrackParticle* >& ) const override {
      ATH_MSG_WARNING( "printInfo: Track->Track matches disabled" );
      return std::string("--> No matches found");
    }

    virtual std::string printInfo(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TruthParticle* >& ) const override {
      ATH_MSG_WARNING( "printInfo: Track->Truth matches disabled" );
      return std::string("--> No matches found");
    }

    virtual std::string printInfo(
        const std::vector< const xAOD::TruthParticle* >& testVec,
        const std::vector< const xAOD::TrackParticle* >& refVec ) const override {
      return printMaps( testVec, refVec, chainRoiName() );
    }

  }; // class TrackMatchingLookup_truthTrk

} // namespace IDTPM

#include "InDetTrackPerfMon/TrackMatchingLookup.icc"

#endif // > !INDETTRACKPERFMON_TRACKMATCHINGLOOKUP_H
