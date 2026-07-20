/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODCREATORALGS_TRACKPARTICLECREATOR_H
#define XAODCREATORALGS_TRACKPARTICLECREATOR_H


// Athena/Gaudi include(s):
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "ParticleTruth/TrackParticleTruthCollection.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "TrkTruthData/TrackTruthCollection.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "TrkTrack/TrackCollection.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexContainer.h"
#include "TrkValInterfaces/ITrkObserverTool.h"
#include "AthenaKernel/SlotSpecificObj.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"


// Local include(s):
#include "xAODTrackingCnv/ITrackParticleMonitoring.h"
#include "xAODTrackingCnv/ITrackCollectionCnvTool.h"
#include "TrkToolInterfaces/ITrackParticleCreatorTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"



namespace xAODMaker {

  /**
   *  @short Algorithm creating xAOD::TrackParticles from TrackParticles
   *
   *         This algorithm can be used to translate the TrackParticles coming
   *         from an AOD, and create xAOD::TrackParticle objects out of them
   *         for an output xAOD.
   *
   * @author Edward Moyse <Edward.Moyse@cern.ch>
   * @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   */
  class TrackParticleCnvAlg : public AthReentrantAlgorithm {

  public:
    /// Regular algorithm constructor
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    /// Function initialising the algorithm
    virtual StatusCode initialize();
    /// Function executing the algorithm
    virtual StatusCode execute(const EventContext& ctx) const;

  private:

    /// toggle on adding truth links
    Gaudi::Property<bool> m_addTruthLink{this,"AddTruthLink", false };
    /// The key for the input TrackParticleTruthCollection


    /// ToolHandle to particle creator
    ToolHandle<Trk::ITrackParticleCreatorTool> m_particleCreator{this,  "TrackParticleCreator", "Trk::TrackParticleCreatorTool/TrackParticleCreatorTool" };
    /// ToolHandle to truth classifier
    ToolHandle<IMCTruthClassifier> m_truthClassifier{this, "MCTruthClassifier", "MCTruthClassifier/MCTruthClassifier",
                                                     " MCTruthClassifier Instance to use "};

    // handles to the converting tools
    ToolHandle<xAODMaker::ITrackCollectionCnvTool> m_TrackCollectionCnvTool{this, "TrackCollectionCnvTool", "xAODMaker::TrackCollectionCnvTool/TrackCollectionCnvTool"};

    SG::ReadHandleKey<TrackCollection> m_tracks{this, "TrackContainerName", "Tracks"};

    SG::ReadHandleKey<xAOD::VertexContainer> m_primaryVertexContainer{ this, "PrimaryVerticesName", "", "Name of primary vertex container is case the parameters should be calculated with respect to the primary vertex"};

    SG::WriteHandleKey<xAOD::TrackParticleContainer> m_xaodout{this, "xAODTrackParticlesFromTracksContainerName", "InDetTrackParticles"};


    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_truthTypeKey{this, "TruthTypeKey", m_xaodout, "truthType"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_truthOriginKey{this, "TruthOriginKey", m_xaodout, "truthOrigin"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_truthClassKey{this, "TruthClassKey", m_xaodout, "truthClassification"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_truthProbKey{this, "TruthMatchProbKey", m_xaodout, "truthMatchProbability"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackLinkKey{this, "TrackLinkKey", m_xaodout, "trackLink"};
    
    SG::ReadHandleKey<xAODTruthParticleLinkVector> m_truthParticleLinkVec{this, "xAODTruthLinkVector", "xAODTruthLinks"};
    SG::ReadHandleKey<TrackTruthCollection> m_trackTruth{this, "TrackTruthContainerName", ""};

    // Allow monitoring of track parameters during conversion
    Gaudi::Property<bool> m_doMonitoring{this, "DoMonitoring", false};
    ToolHandle<ITrackParticleMonitoring> m_trackMonitoringTool{ this, "TrkMonTool", "", "Tracking Monitoring tool" };

    //for timing we need a handle to the MonTool in the alg
    ToolHandle<GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };

    // Augment observed tracks with information from track observer tool map
    Gaudi::Property<bool> m_augmentObservedTracks{this, "AugmentObservedTracks", false, "augment observed tracks"};
    SG::ReadHandleKey<ObservedTrackMap> m_tracksMap{this, "TracksMapName", "" , "name of observed tracks map saved in store"};

    /// toggle on converting tracks to xAOD
    Gaudi::Property<bool> m_convertTracks{this, "ConvertTracks", false};

    StatusCode convert(const EventContext& ctx,
                      const TrackCollection& trackColl,
                      const TrackTruthCollection* assocTruthColl,
                      xAOD::TrackParticleContainer& outTrackCont,
                      const xAODTruthParticleLinkVector*,
                      const xAOD::Vertex* primaryVertex ,
                      const ObservedTrackMap* obs_track_map) const;

    }; // class TrackParticleCnvAlg

} // namespace xAODMaker

#endif // XAODCREATORALGS_TRACKPARTICLECREATOR_H
