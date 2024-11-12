/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
*/

#include "TauElecSubtractAlg.h"
#include <algorithm>

StatusCode TauElecSubtractAlg::initialize()
{
    ATH_CHECK( m_elecInput.initialize() );
    ATH_CHECK( m_clustersInput.initialize() );
    ATH_CHECK( m_clustersOutput.initialize() );
    ATH_CHECK( m_tracksInput.initialize() );
    ATH_CHECK( m_tracksOutput.initialize() );
    ATH_CHECK( m_removedClustersOutput.initialize() );
    ATH_CHECK( m_removedTracksOutput.initialize() );
    ATH_CHECK( m_eleLHSelectTool.retrieve() );
    ATH_CHECK( m_stdJetTVADecoKey.initialize() );

    return StatusCode::SUCCESS;
}

StatusCode TauElecSubtractAlg::execute (const EventContext& ctx) const
{
    SG::ReadHandle<xAOD::ElectronContainer>      electronInput (m_elecInput,     ctx);
    SG::ReadHandle<xAOD::CaloClusterContainer>   clustersInput (m_clustersInput, ctx);
    SG::ReadHandle<xAOD::TrackParticleContainer> tracksInput   (m_tracksInput,   ctx);
    if(!electronInput.isValid() || !clustersInput.isValid() || !tracksInput.isValid()) 
    {
        if (!electronInput.isValid())   ATH_MSG_ERROR( "Collection " << electronInput.key() << " is not valid" );
        if (!clustersInput.isValid())   ATH_MSG_ERROR( "Collection " << clustersInput.key() << " is not valid" );
        if (!tracksInput.isValid())     ATH_MSG_ERROR( "Collection " << tracksInput.key()   << " is not valid" );
        return StatusCode::FAILURE;
    }

    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<ElementLink<xAOD::VertexContainer>>> stdJetTVADecoHandle (m_stdJetTVADecoKey, ctx);
    if (!stdJetTVADecoHandle.isPresent())
    {
        ATH_MSG_ERROR("Standard jet TVA decoration is not valid for InDetTrackParticles");
        return StatusCode::FAILURE;
    }

    SG::WriteHandle<xAOD::CaloClusterContainer> clustersOutputHandle (m_clustersOutput, ctx);
    ATH_CHECK( clustersOutputHandle.record(std::make_unique<xAOD::CaloClusterContainer>(), std::make_unique<xAOD::CaloClusterAuxContainer>()) );

    SG::WriteHandle<xAOD::TrackParticleContainer> tracksOutputHandle (m_tracksOutput, ctx);
    ATH_CHECK( tracksOutputHandle.record(std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>()) );

    SG::WriteHandle<xAOD::CaloClusterContainer> removedClustersOutputHandle (m_removedClustersOutput, ctx);
    ATH_CHECK( removedClustersOutputHandle.record(std::make_unique<xAOD::CaloClusterContainer>(), std::make_unique<xAOD::CaloClusterAuxContainer>()) );

    SG::WriteHandle<xAOD::TrackParticleContainer> removedTracksOutputHandle (m_removedTracksOutput, ctx);
    ATH_CHECK( removedTracksOutputHandle.record(std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>()) );

    std::vector<bool> selectElectron(electronInput->size(), false);

    for (const xAOD::Electron* elec : *electronInput.cptr()) {
      selectElectron.at(elec->index()) = static_cast<bool>(m_eleLHSelectTool->accept(elec));
    }

    // early stopping: if no electron is selected, the EleRM reconstruction should not run
    // the simplest strategy is to write out empty cluster and track containers
    if (std::find(selectElectron.begin(),selectElectron.end(),true) == selectElectron.end()) return StatusCode::SUCCESS;

    xAOD::CaloClusterContainer* clustersOutputContainer = clustersOutputHandle.ptr();
    clustersOutputContainer->reserve(clustersInput->size());

    xAOD::TrackParticleContainer* tracksOutputContainer = tracksOutputHandle.ptr();
    tracksOutputContainer->reserve(tracksInput->size());

    std::vector<const xAOD::TrackParticle *> tracks_to_remove;
    std::vector<const xAOD::CaloCluster *>   clusters_to_remove;
    if(!m_doNothing){
        for (const xAOD::Electron* elec : *electronInput.cptr())
        {
	  if (selectElectron.at(elec->index()))
            {
                tracks_to_remove = xAOD::EgammaHelpers::getTrackParticlesVec(elec, true, true);
                std::vector<ElementLink< xAOD::CaloClusterContainer>> elec_cluster_links = elec->caloClusterLinks();
                for (const auto& elec_cluster_link : elec_cluster_links){
                    if (elec_cluster_link.isValid()){
                        std::vector<const xAOD::CaloCluster*> orig_clusters = xAOD::EgammaHelpers::getAssociatedTopoClusters(*elec_cluster_link);
                        clusters_to_remove.insert(clusters_to_remove.end(), orig_clusters.cbegin(), orig_clusters.cend());
                    }
                }
            }
        }
    } else {
        ATH_MSG_WARNING("DOING NOTHING... For validation only, please check your config.");
    }
    for (const xAOD::TrackParticle* old_track : *tracksInput.cptr())
    {
        auto where = std::find_if(tracks_to_remove.cbegin(), tracks_to_remove.cend(), 
            [&](const xAOD::TrackParticle* target){return (target == old_track);}
        );
        if (where == tracks_to_remove.cend())
        {
            auto *new_track = new xAOD::TrackParticle();
            tracksOutputContainer->push_back(new_track);
            *new_track = *old_track;
            auto link = ElementLink< xAOD::TrackParticleContainer >( *tracksInput, old_track->index() );
            static const SG::Accessor<ElementLink<xAOD::TrackParticleContainer>> acc_originalObjectDecor("ERMOriginalTrack");
            acc_originalObjectDecor(*new_track) = link;
        }
    }
    ATH_MSG_DEBUG("Old tracks size = " << tracksInput->size() << ", new tracks size = " << tracksOutputContainer->size() << ", expected diff = " << tracks_to_remove.size());
    
    for (const xAOD::CaloCluster* old_cluster : *clustersInput.cptr()) { 
        auto where = std::find_if(clusters_to_remove.cbegin(), clusters_to_remove.cend(), 
            [&](const xAOD::CaloCluster* target){return (target == old_cluster);}
        );
        if (where == clusters_to_remove.cend()){
            auto *new_cluster = new xAOD::CaloCluster();
            clustersOutputContainer->push_back(new_cluster);
            *new_cluster = *old_cluster;
            auto link = ElementLink< xAOD::CaloClusterContainer >( *clustersInput, old_cluster->index() );
            static const SG::Accessor<ElementLink<xAOD::CaloClusterContainer>> acc_originalObjectDecor("ERMOriginalCaloCluster");
            acc_originalObjectDecor(*new_cluster) = link;
        }
    }
    ATH_MSG_DEBUG("Old cluster size = " << clustersInput->size() << ", new cluster size = " << clustersOutputContainer->size() << ", expected diff = " << clusters_to_remove.size());

    xAOD::CaloClusterContainer* removedClustersOutputCont = removedClustersOutputHandle.ptr();
    removedClustersOutputCont->reserve(clusters_to_remove.size());
    for (const xAOD::CaloCluster* cls : clusters_to_remove){
        auto *new_removed_cluster = new xAOD::CaloCluster();
        *new_removed_cluster = *cls;
        removedClustersOutputCont->push_back(new_removed_cluster);
    }

    xAOD::TrackParticleContainer* removedTracksOutputCont = removedTracksOutputHandle.ptr();
    removedTracksOutputCont->reserve(tracks_to_remove.size());
    for (const xAOD::TrackParticle* trk : tracks_to_remove){
        auto *new_removed_track = new xAOD::TrackParticle();
        *new_removed_track = *trk;
        removedTracksOutputCont->push_back(new_removed_track);
    }
    
    return StatusCode::SUCCESS;
}
