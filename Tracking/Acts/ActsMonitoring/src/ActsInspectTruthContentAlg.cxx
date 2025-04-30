/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/ActsInspectTruthContentAlg.h"
#include "ActsInterop/TableUtils.h"
#include "TruthUtils/MagicNumbers.h"
#include <unordered_map>

namespace ActsTrk {

  template <typename T>
  std::underlying_type_t<T> to_underlying(T val)
  { return static_cast< std::underlying_type_t<T> >(val); }

  
  ActsInspectTruthContentAlg::ActsInspectTruthContentAlg(const std::string& name,
							 ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode ActsInspectTruthContentAlg::initialize() {
    ATH_MSG_INFO( "Initializing " << name() << " ..." );

    ATH_CHECK( m_clusters.initialize() );
    ATH_CHECK( m_seeds.initialize() );
    ATH_CHECK( m_associationMap_key.initialize() );
    ATH_CHECK( m_tracks.initialize() );
    
    if (m_clusters.size() != m_associationMap_key.size()) {
      ATH_MSG_ERROR("Inconsistent sizes of Clusters and TruthAssociationMaps");
      return StatusCode::FAILURE;
    }

    for (const auto& trackKey : m_tracks) {
      std::string containerName = trackKey.key();
      m_onTrack_clusterStat.push_back( std::make_pair(containerName, cluster_stat_t()) );
      m_trackStat.push_back( std::make_pair(containerName, track_stat_t()) );
    }

    return StatusCode::SUCCESS;
  }

  
  StatusCode ActsInspectTruthContentAlg::finalize() {
    ATH_MSG_INFO( "Finalizing " << name() << " ..." );
    ATH_MSG_INFO( "Statistics from Seed check with truth info:" );

    if ( printStatTables<ActsInspectTruthContentAlg::EStatClusters, xAOD::UncalibMeasType>("Clusters", m_clusterStat).isFailure() ) {
      ATH_MSG_FATAL("Problem dumping Cluster truth information");
      return StatusCode::FAILURE;
    }

    if ( printStatTables<ActsInspectTruthContentAlg::EStatSeeds, ActsInspectTruthContentAlg::SeedType>("Seeds", m_seedStat).isFailure() ) {
      ATH_MSG_FATAL("Problem dumping Seed truth information");
      return StatusCode::FAILURE;
    }

    for (const auto& [trackCollectionName, onTrackStat] : m_onTrack_clusterStat) {
      std::string reportName = "On Track Clusters (" + trackCollectionName + ")";
      if ( printStatTables<ActsInspectTruthContentAlg::EStatClusters, xAOD::UncalibMeasType>(reportName, onTrackStat).isFailure() ) {
	ATH_MSG_FATAL("Problem dumping On Track Cluster truth info (" << trackCollectionName << ")");
	return StatusCode::FAILURE;
      }
    }
    
    for (const auto& [trackCollectionName, trackStat] : m_trackStat) {
      std::string reportName = "Track (" + trackCollectionName + ")";
      if ( printStatTables<ActsInspectTruthContentAlg::EStatTracks, ActsInspectTruthContentAlg::TrackType>(reportName, trackStat).isFailure() ) {
        ATH_MSG_FATAL("Problem dumping Track truth info (" << trackCollectionName << ")");
        return StatusCode::FAILURE;
      }
    }
    
    return StatusCode::SUCCESS;
  }

  
  StatusCode ActsInspectTruthContentAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << " ..." );
    
    // contextual stat collectors
    cluster_stat_t clusterStat {};
    seed_stat_t seedStat {};
    
    std::array<const ActsTrk::MeasurementToTruthParticleAssociation*, s_nClusterTypes> truths {};
    // Fill Stat info for Cluster collection(s)
    for (std::size_t i(0); i<m_clusters.size(); ++i) {
      ATH_MSG_DEBUG( "Retrieving cluster collection with key " << m_clusters.at(i).key() );
      SG::ReadHandle<xAOD::UncalibratedMeasurementContainer> clusterHandle = SG::makeHandle( m_clusters.at(i), ctx );
      ATH_CHECK( clusterHandle.isValid() );
      const xAOD::UncalibratedMeasurementContainer *clusters = clusterHandle.cptr();

      if (clusters->empty()) continue;
      xAOD::UncalibMeasType elementType = clusters->front()->type();
	
      ATH_MSG_DEBUG( "Retrieving Measurement to Truth Particle Association map with key: " << m_associationMap_key.at(i).key() );
      SG::ReadHandle<ActsTrk::MeasurementToTruthParticleAssociation> truthHandle = SG::makeHandle( m_associationMap_key.at(i), ctx );
      ATH_CHECK( truthHandle.isValid() );
      truths[static_cast<std::size_t>(elementType)] = truthHandle.cptr();

      // Check truth and clusters are compatible
      ATH_CHECK( truths[static_cast<std::size_t>(elementType)]->isCompatibleWith(clusters) );

      ATH_CHECK( fillStatClusters(*clusters,
				  *truths[static_cast<std::size_t>(elementType)],
				  clusterStat) );
    }

    ATH_CHECK( copyStatTable(clusterStat, m_clusterStat) );
    
    for (std::size_t i(0); i<m_seeds.size(); ++i) {
      ATH_MSG_DEBUG( "Retrieving seed collection with key: " << m_seeds.at(i).key() );
      SG::ReadHandle<ActsTrk::SeedContainer> seedHandle = SG::makeHandle( m_seeds.at(i), ctx );
      ATH_CHECK( seedHandle.isValid() );
      const ActsTrk::SeedContainer* seeds = seedHandle.cptr();

      ATH_CHECK( fillStatSeeds(*seeds,
			       truths,
			       seedStat) );
    }

    ATH_CHECK( copyStatTable(seedStat, m_seedStat) );

    for (std::size_t i(0); i<m_tracks.size(); ++i) {
      ATH_MSG_DEBUG( "Retrieving tracks collection with key " << m_tracks.at(i).key() );
      SG::ReadHandle<ActsTrk::TrackContainer> trackHandle = SG::makeHandle( m_tracks.at(i), ctx );
      ATH_CHECK( trackHandle.isValid() );
      const ActsTrk::TrackContainer* tracks = trackHandle.cptr();

      cluster_stat_t onTrack_clusterStat {};
      track_stat_t trackStat {};
      ATH_CHECK( fillStatTracks(*tracks,				
				truths,
				trackStat,
				onTrack_clusterStat) );
      
      ATH_CHECK( copyStatTable(trackStat, m_trackStat.at(i).second) );
      ATH_CHECK( copyStatTable(onTrack_clusterStat, m_onTrack_clusterStat.at(i).second) );
    }

    return StatusCode::SUCCESS;
  }

  
  StatusCode ActsInspectTruthContentAlg::fillStatClusters(const xAOD::UncalibratedMeasurementContainer& container,
							  const ActsTrk::MeasurementToTruthParticleAssociation& truth,
							  typename ActsInspectTruthContentAlg::cluster_stat_t& stat) const
  {
    ATH_MSG_DEBUG( "Checking truth for clusters ..." );
    for (const xAOD::UncalibratedMeasurement* meas : container) {
      std::size_t clusterTypeIndex = static_cast<std::size_t>(meas->type());
      ++stat[to_underlying(EStatClusters::kNTotal)][clusterTypeIndex];
      const auto& tps = truth.at(meas->index());

      // get number of (valid) contributions
      if (tps.empty()) {
        ++stat[to_underlying(EStatClusters::kNClustersWithNoBarcode)][clusterTypeIndex];
        continue;
      }
      
      // Check all barcodes are from primary particles
      bool allValidParticles = true;
      for (const auto* tp : tps) {
	if ( not HepMC::is_simulation_particle(*tp) ) continue;
	allValidParticles = false;
	break;
      }
      
      if (tps.size() == 1) {
	++stat[to_underlying(EStatClusters::kNClustersWith1Contribution)][clusterTypeIndex];
	if (allValidParticles) ++stat[to_underlying(EStatClusters::kNClustersWith1ValidContribution)][clusterTypeIndex];
      }
      else if (tps.size() == 2) {
	++stat[to_underlying(EStatClusters::kNClustersWith2Contribution)][clusterTypeIndex];
	if (allValidParticles) ++stat[to_underlying(EStatClusters::kNClustersWith2ValidContribution)][clusterTypeIndex];
      }
      else {
	++stat[to_underlying(EStatClusters::kNClustersWith3Contribution)][clusterTypeIndex];
	if (allValidParticles) ++stat[to_underlying(EStatClusters::kNClustersWith3ValidContribution)][clusterTypeIndex];
      }
      
      // get main contribution
      bool hasContributionFromPrimaryParticle = false;
      for (const auto* tp : tps) {
	if ( HepMC::is_simulation_particle(*tp) ) continue;
	hasContributionFromPrimaryParticle = true;
	break;
      }

      if (not hasContributionFromPrimaryParticle) ++stat[to_underlying(EStatClusters::kNClustersWith200kBarcode)][clusterTypeIndex];
      else ++stat[to_underlying(EStatClusters::kNClustersFromPrimaries)][clusterTypeIndex];
    }
    
    return StatusCode::SUCCESS;
  }
  

  StatusCode ActsInspectTruthContentAlg::fillStatSeeds(const ActsTrk::SeedContainer& seeds,
						       std::array<const ActsTrk::MeasurementToTruthParticleAssociation*, s_nClusterTypes>& truths,
						       seed_stat_t& stat) const
  {
    ATH_MSG_DEBUG( "Checking truth for seeds ..." );
    for (std::size_t i(0); i<seeds.size(); ++i) {
      const ActsTrk::Seed* seed = seeds.at(i);

      int nMatches = 0;
      int nMeasurements = 0;
      std::unordered_map<std::size_t, int> particleIds {};

      std::size_t seedType = to_underlying(deduceSeedType(*seed));
      ++stat[to_underlying(EStatSeeds::kNTotal)][seedType];

      const auto& sps = seed->sp();
      for ( const xAOD::SpacePoint* sp : sps ) {
	const auto& measurements = sp->measurements();
	for (const xAOD::UncalibratedMeasurement* meas : measurements ) {
	  ++nMeasurements;
	  
	  std::size_t clusterTypeIndex = to_underlying(meas->type());
	  const ActsTrk::MeasurementToTruthParticleAssociation* truth = truths.at(clusterTypeIndex);
	  auto tps = truth->at(meas->index());

	  if (tps.empty()) continue;
	  
	  bool contributionOnlyFromSimulationParticles = true;
	  for (const auto* tp : tps) {
	    if ( HepMC::is_simulation_particle(*tp) ) continue;
	    contributionOnlyFromSimulationParticles = false;

	    std::size_t pid = HepMC::uniqueID(tp);
	    particleIds.try_emplace( pid, 0 );
	    ++particleIds[pid];
	  }
	  if (contributionOnlyFromSimulationParticles) continue;

	  ++nMatches;
	} // loop on measurements
      } // loop on space points

      // check if we have measurements associated to the same particle id
      // if the number of entries for the same id (the key) is the same as the
      // number of measurements in the seed, then we have a match
      bool isFromSameParticle = false;
      for (const auto [pid, nEntries] : particleIds) {
	if (nEntries != nMeasurements) continue;
	isFromSameParticle = true;
	break;
      }

      
      if (nMatches == 0) {
	++stat[to_underlying(EStatSeeds::nKSeedsWith0Matches)][seedType];
      } else if (nMatches == 1) {
	++stat[to_underlying(EStatSeeds::nKSeedsWith1Matches)][seedType];
      } else if	(nMatches == 2) {
	++stat[to_underlying(EStatSeeds::nKSeedsWith2Matches)][seedType];
	if (isFromSameParticle) ++stat[to_underlying(EStatSeeds::nKSeedsSame2Matches)][seedType];
      } else if (nMatches == 3) {
        ++stat[to_underlying(EStatSeeds::nKSeedsWith3Matches)][seedType];
	if (isFromSameParticle) ++stat[to_underlying(EStatSeeds::nKSeedsSame3Matches)][seedType];
      } else if (nMatches == 4) {
        ++stat[to_underlying(EStatSeeds::nKSeedsWith4Matches)][seedType];
	if (isFromSameParticle) ++stat[to_underlying(EStatSeeds::nKSeedsSame4Matches)][seedType];
      } else if (nMatches == 5) {
        ++stat[to_underlying(EStatSeeds::nKSeedsWith5Matches)][seedType];
	if (isFromSameParticle) ++stat[to_underlying(EStatSeeds::nKSeedsSame5Matches)][seedType];
      } else if (nMatches == 6) {
        ++stat[to_underlying(EStatSeeds::nKSeedsWith6Matches)][seedType];
	if (isFromSameParticle) ++stat[to_underlying(EStatSeeds::nKSeedsSame6Matches)][seedType];
      }                  
    } // loop on seed
    
    return StatusCode::SUCCESS;
  }

  ActsInspectTruthContentAlg::SeedType ActsInspectTruthContentAlg::deduceSeedType(const ActsTrk::Seed& seed) const {
    const auto&	[bottom, middle, top] = seed.sp();
    xAOD::UncalibMeasType bottom_type = bottom->measurements().front()->type();
    xAOD::UncalibMeasType middle_type = middle->measurements().front()->type();
    xAOD::UncalibMeasType top_type = top->measurements().front()->type();

    if (bottom_type == xAOD::UncalibMeasType::PixelClusterType and
	middle_type == xAOD::UncalibMeasType::PixelClusterType and
	top_type == xAOD::UncalibMeasType::PixelClusterType) {
      return SeedType::PPP;
    } else if (bottom_type == xAOD::UncalibMeasType::PixelClusterType and
	       middle_type == xAOD::UncalibMeasType::PixelClusterType and
	       top_type == xAOD::UncalibMeasType::StripClusterType) {
      return SeedType::PPS;
    } else if (bottom_type == xAOD::UncalibMeasType::PixelClusterType and
	       middle_type == xAOD::UncalibMeasType::StripClusterType and
	       top_type == xAOD::UncalibMeasType::StripClusterType) {
      return SeedType::PSS;
    } else if (bottom_type == xAOD::UncalibMeasType::StripClusterType and
	       middle_type == xAOD::UncalibMeasType::StripClusterType and
	       top_type == xAOD::UncalibMeasType::StripClusterType) {
      return SeedType::SSS;
    } else {
      return SeedType::Others;
    }
  }

  
  StatusCode ActsInspectTruthContentAlg::fillStatTracks(const ActsTrk::TrackContainer& tracks,
							std::array<const ActsTrk::MeasurementToTruthParticleAssociation*, s_nClusterTypes>& truths,
							track_stat_t& trackStat,
							cluster_stat_t& onTrackStat) const {
    ATH_MSG_DEBUG( "Checking truth for tracks ..." );
    for (const auto& track : tracks) {
      ++trackStat[to_underlying(EStatTracks::kNTotal)][0];
      std::size_t nHoles = track.nHoles();
      if (nHoles == 0) ++trackStat[to_underlying(EStatTracks::kNTracks0Holes)][0];
      else if (nHoles == 1) ++trackStat[to_underlying(EStatTracks::kNTracks1Holes)][0];
      else if (nHoles == 2) ++trackStat[to_underlying(EStatTracks::kNTracks2Holes)][0];
      else ++trackStat[to_underlying(EStatTracks::kNTracks3Holes)][0];

      std::size_t nOutliers = track.nOutliers();
      if (nOutliers == 0) ++trackStat[to_underlying(EStatTracks::kNTracks0Outliers)][0];
      else if (nOutliers == 1) ++trackStat[to_underlying(EStatTracks::kNTracks1Outliers)][0];
      else if (nOutliers == 2) ++trackStat[to_underlying(EStatTracks::kNTracks2Outliers)][0];
      else ++trackStat[to_underlying(EStatTracks::kNTracks3Outliers)][0];


      bool AllValids = true;
      int nConsideredMeasurements = 0;
      std::unordered_map<std::size_t, int> particleIds {};
      
      // on track clusters
      track.container()
	.trackStateContainer().visitBackwards(track.tipIndex(),
					      [&truths, &onTrackStat,
					       &AllValids,
					       &nConsideredMeasurements, &particleIds]
					      (const auto& state) {
						auto flags = state.typeFlags();
						if (not flags.test(Acts::TrackStateFlag::MeasurementFlag) and
						    not flags.test(Acts::TrackStateFlag::OutlierFlag)) return;
						++nConsideredMeasurements;
						
						// get cluster
						auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
						assert( sl != nullptr);
						const xAOD::UncalibratedMeasurement &meas = getUncalibratedMeasurement(sl);
						
						std::size_t clusterTypeIndex = to_underlying(meas.type());
						++onTrackStat[to_underlying(EStatClusters::kNTotal)][clusterTypeIndex];
						
						const ActsTrk::MeasurementToTruthParticleAssociation* truth = truths[clusterTypeIndex];
						const auto& tps = truth->at(meas.index());

						if (tps.empty()) {
						  ++onTrackStat[to_underlying(EStatClusters::kNClustersWithNoBarcode)][clusterTypeIndex];
						  AllValids = false;
						  return;
						}
						
						bool allBarcodesValid = true;
						bool hasContributionFromPrimaryParticle = false;
						for (const auto* tp : tps) {
						  if ( not HepMC::is_simulation_particle(*tp) ) {
						    hasContributionFromPrimaryParticle = true;
						    continue;
						  }
						  allBarcodesValid = false;
						  break;
						}
						if (not hasContributionFromPrimaryParticle) AllValids = false;
						
						if (tps.size() == 1) {
						  ++onTrackStat[to_underlying(EStatClusters::kNClustersWith1Contribution)][clusterTypeIndex];
						  if (allBarcodesValid) ++onTrackStat[to_underlying(EStatClusters::kNClustersWith1ValidContribution)][clusterTypeIndex];
						}
						else if (tps.size() == 2) {
						  ++onTrackStat[to_underlying(EStatClusters::kNClustersWith2Contribution)][clusterTypeIndex];
						  if (allBarcodesValid) ++onTrackStat[to_underlying(EStatClusters::kNClustersWith2ValidContribution)][clusterTypeIndex];
						}
						else {
						  ++onTrackStat[to_underlying(EStatClusters::kNClustersWith3Contribution)][clusterTypeIndex];
						  if (allBarcodesValid) ++onTrackStat[to_underlying(EStatClusters::kNClustersWith3ValidContribution)][clusterTypeIndex];
						}

						bool contributionOnlyFromSimulationParticles = true;
						for (const auto* tp : tps) {
						  if ( HepMC::is_simulation_particle(*tp) ) continue;
						  contributionOnlyFromSimulationParticles = false;
						  
						  std::size_t pid = HepMC::uniqueID(tp);
						  particleIds.try_emplace( pid, 0 );
						  ++particleIds[pid];
						}
						
						// get main contribution
						if (contributionOnlyFromSimulationParticles) ++onTrackStat[to_underlying(EStatClusters::kNClustersWith200kBarcode)][clusterTypeIndex];
						else ++onTrackStat[to_underlying(EStatClusters::kNClustersFromPrimaries)][clusterTypeIndex];
					      }); // llop on states

      bool AllSameBarcode = false;
      for ( const auto [pid, nEntries] : particleIds ) {
	if (nEntries != nConsideredMeasurements) continue;
	AllSameBarcode = true;
	break;
      }
      
      if (AllValids) ++trackStat[to_underlying(EStatTracks::kNFullMatch)][0];
      if (AllSameBarcode) ++trackStat[to_underlying(EStatTracks::kNPerfectMatch)][0];
    } // loop on tracks    

    return StatusCode::SUCCESS;
  }
  
} // namespace 

