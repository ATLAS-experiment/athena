/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_PROTOTRACKREPORTINGALG_H
#define ACTSTRACKRECONSTRUCTION_PROTOTRACKREPORTINGALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TrkTrack/TrackCollection.h"
#include "xAODTracking/TrackParticleContainer.h"


namespace ActsTrk{
    /// @brief Very lightweight algorithm to print out the results of the EF track finding.
    /// Will later be superseded by dedicated monitoring  
    class ProtoTrackReportingAlg: public ::AthReentrantAlgorithm { 
    public: 
    ProtoTrackReportingAlg( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~ProtoTrackReportingAlg() = default;

    ///uncomment and implement methods as required

                                            //IS EXECUTED:
    virtual StatusCode  initialize() override final;     //once, before any input is loaded
    virtual StatusCode  execute(const EventContext & ctx) const override final;
    
    private: 
        // the track collection to print 
        SG::ReadHandleKey<TrackCollection> m_EFTracks{this, "TrackCollection","","track collection to look for"};
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_xAODTracks{this, "xAODTrackCollection", ""," xAOD track collection to look for"};


    }; 

}

#endif //> !ACTSTRACKRECONSTRUCTION_PROTOTRACKREPORTINGALG_H
