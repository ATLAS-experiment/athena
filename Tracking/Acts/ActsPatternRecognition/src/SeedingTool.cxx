/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/SeedingTool.h"

// ACTS
#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/Seeding/BinnedGroup.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFinder.hpp"
#include "Acts/Seeding/SeedFinderConfig.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Seeding/SeedConfirmationRangeConfig.hpp"

namespace ActsTrk {
   SeedingTool::SeedingTool(const std::string& type,
    const std::string& name,
    const IInterface* parent)
    : base_class(type, name, parent)
  {}
  
  StatusCode SeedingTool::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG("   " << m_zBinNeighborsTop);
    ATH_MSG_DEBUG("   " << m_zBinNeighborsBottom);
    ATH_MSG_DEBUG("   " << m_numPhiNeighbors);

    ATH_MSG_DEBUG(" *  Used by SpacePointGridConfig");
    ATH_MSG_DEBUG("   " << m_minPt);
    ATH_MSG_DEBUG("   " << m_cotThetaMax);
    ATH_MSG_DEBUG("   " << m_impactMax);
    ATH_MSG_DEBUG("   " << m_zMin);
    ATH_MSG_DEBUG("   " << m_zMax);
    ATH_MSG_DEBUG("   " << m_gridPhiMin);
    ATH_MSG_DEBUG("   " << m_gridPhiMax);
    ATH_MSG_DEBUG("   " << m_zBinEdges);
    ATH_MSG_DEBUG("   " << m_deltaRMax);
    ATH_MSG_DEBUG("   " << m_gridRMax);
    ATH_MSG_DEBUG("   " << m_phiBinDeflectionCoverage);

    ATH_MSG_DEBUG(" * Used by SeedFinderConfig:");
    ATH_MSG_DEBUG("   " << m_minPt);
    ATH_MSG_DEBUG("   " << m_cotThetaMax);
    ATH_MSG_DEBUG("   " << m_impactMax);
    ATH_MSG_DEBUG("   " << m_zMin);
    ATH_MSG_DEBUG("   " << m_zMax);
    ATH_MSG_DEBUG("   " << m_zBinEdges);
    ATH_MSG_DEBUG("   " << m_rMax);
    ATH_MSG_DEBUG("   " << m_deltaRMin);
    ATH_MSG_DEBUG("   " << m_deltaRMax);
    ATH_MSG_DEBUG("   " << m_deltaRMinTopSP);
    ATH_MSG_DEBUG("   " << m_deltaRMaxTopSP);
    ATH_MSG_DEBUG("   " << m_deltaRMinBottomSP);
    ATH_MSG_DEBUG("   " << m_deltaRMaxBottomSP);
    ATH_MSG_DEBUG("   " << m_deltaZMax);
    ATH_MSG_DEBUG("   " << m_collisionRegionMin);
    ATH_MSG_DEBUG("   " << m_collisionRegionMax);
    ATH_MSG_DEBUG("   " << m_sigmaScattering);
    ATH_MSG_DEBUG("   " << m_maxPtScattering);
    ATH_MSG_DEBUG("   " << m_radLengthPerSeed);
    ATH_MSG_DEBUG("   " << m_maxSeedsPerSpM);
    ATH_MSG_DEBUG("   " << m_interactionPointCut);
    ATH_MSG_DEBUG("   " << m_zBinsCustomLooping);
    ATH_MSG_DEBUG("   " << m_useVariableMiddleSPRange);
    if ( m_useVariableMiddleSPRange ) {
      ATH_MSG_DEBUG("   " << m_deltaRMiddleMinSPRange);
      ATH_MSG_DEBUG("   " << m_deltaRMiddleMaxSPRange);
    } else if ( not m_rRangeMiddleSP.empty() )
      ATH_MSG_DEBUG("   " << m_rRangeMiddleSP);
    ATH_MSG_DEBUG("   " << m_seedConfirmation);
    if ( m_seedConfirmation ) {
      ATH_MSG_DEBUG("   " << m_seedConfCentralZMin);
      ATH_MSG_DEBUG("   " << m_seedConfCentralZMax);
      ATH_MSG_DEBUG("   " << m_seedConfCentralRMax);
      ATH_MSG_DEBUG("   " << m_seedConfCentralNTopLargeR);
      ATH_MSG_DEBUG("   " << m_seedConfCentralNTopSmallR);
      ATH_MSG_DEBUG("   " << m_seedConfCentralMinBottomRadius);
      ATH_MSG_DEBUG("   " << m_seedConfCentralMaxZOrigin);
      ATH_MSG_DEBUG("   " << m_seedConfCentralMinImpact);
      ATH_MSG_DEBUG("   " << m_seedConfForwardZMin);
      ATH_MSG_DEBUG("   " << m_seedConfForwardZMax);
      ATH_MSG_DEBUG("   " << m_seedConfForwardRMax);
      ATH_MSG_DEBUG("   " << m_seedConfForwardNTopLargeR);
      ATH_MSG_DEBUG("   " << m_seedConfForwardNTopSmallR);
      ATH_MSG_DEBUG("   " << m_seedConfForwardMinBottomRadius);
      ATH_MSG_DEBUG("   " << m_seedConfForwardMaxZOrigin);
      ATH_MSG_DEBUG("   " << m_seedConfForwardMinImpact);
    }
    ATH_MSG_DEBUG("   " << m_useDetailedDoubleMeasurementInfo);
    ATH_MSG_DEBUG("   " << m_toleranceParam);
    ATH_MSG_DEBUG("   " << m_phiMin);
    ATH_MSG_DEBUG("   " << m_phiMax);
    ATH_MSG_DEBUG("   " << m_rMin);
    ATH_MSG_DEBUG("   " << m_zAlign);
    ATH_MSG_DEBUG("   " << m_rAlign);
    ATH_MSG_DEBUG("   " << m_sigmaError);

    ATH_MSG_DEBUG(" * Used by SeedFilterConfig:");
    ATH_MSG_DEBUG("   " << m_deltaRMin);
    ATH_MSG_DEBUG("   " << m_maxSeedsPerSpM);
    ATH_MSG_DEBUG("   " << m_useDeltaRorTopRadius);
    ATH_MSG_DEBUG("   " << m_seedConfirmationInFilter);
    if (m_seedConfirmationInFilter) {
      ATH_MSG_DEBUG("   " << m_maxSeedsPerSpMConf);
      ATH_MSG_DEBUG("   " << m_maxQualitySeedsPerSpMConf);
      ATH_MSG_DEBUG("   " << m_seedConfCentralZMin);
      ATH_MSG_DEBUG("   " << m_seedConfCentralZMax);
      ATH_MSG_DEBUG("   " << m_seedConfCentralRMax);
      ATH_MSG_DEBUG("   " << m_seedConfCentralNTopLargeR);
      ATH_MSG_DEBUG("   " << m_seedConfCentralNTopSmallR);
      ATH_MSG_DEBUG("   " << m_seedConfCentralMinBottomRadius);
      ATH_MSG_DEBUG("   " << m_seedConfCentralMaxZOrigin);
      ATH_MSG_DEBUG("   " << m_seedConfCentralMinImpact);
      ATH_MSG_DEBUG("   " << m_seedConfForwardZMin);
      ATH_MSG_DEBUG("   " << m_seedConfForwardZMax);
      ATH_MSG_DEBUG("   " << m_seedConfForwardRMax);
      ATH_MSG_DEBUG("   " << m_seedConfForwardNTopLargeR);
      ATH_MSG_DEBUG("   " << m_seedConfForwardNTopSmallR);
      ATH_MSG_DEBUG("   " << m_seedConfForwardMinBottomRadius);
      ATH_MSG_DEBUG("   " << m_seedConfForwardMaxZOrigin);
      ATH_MSG_DEBUG("   " << m_seedConfForwardMinImpact);
    }
    ATH_MSG_DEBUG("   " << m_impactWeightFactor);
    ATH_MSG_DEBUG("   " << m_compatSeedWeight);
    ATH_MSG_DEBUG("   " << m_compatSeedLimit);
    ATH_MSG_DEBUG("   " << m_seedWeightIncrement);
    ATH_MSG_DEBUG("   " << m_numSeedIncrement);
    ATH_MSG_DEBUG("   " << m_deltaInvHelixDiameter);

 
    if (m_zBinEdges.size() - 1 !=
      m_zBinNeighborsTop.size() and
      not m_zBinNeighborsTop.empty()) {
      ATH_MSG_ERROR("Inconsistent config zBinNeighborsTop");
      return StatusCode::FAILURE;
    }

    if (m_zBinEdges.size() - 1 !=
      m_zBinNeighborsBottom.size() and
      not m_zBinNeighborsBottom.empty()) {
      ATH_MSG_ERROR("Inconsistent config zBinNeighborsBottom");
      return StatusCode::FAILURE;
    }

    if (m_zBinsCustomLooping.size() != 0) {
      // zBinsCustomLooping can contain a number of elements <= to the total number
      // of bin in zBinEdges 
      for (std::size_t i : m_zBinsCustomLooping) {
	if (i >= m_zBinEdges.size()) {
          ATH_MSG_ERROR("Inconsistent config zBinsCustomLooping contains bins that are not in zBinEdges");
          return StatusCode::FAILURE;
        }
      }
    }

    ATH_CHECK( prepareConfiguration() );

    m_bottomBinFinder = std::make_unique< Acts::GridBinFinder< 2ul > >(m_numPhiNeighbors.value(), m_zBinNeighborsBottom.value());
    m_topBinFinder = std::make_unique< Acts::GridBinFinder< 2ul > >(m_numPhiNeighbors.value(), m_zBinNeighborsTop.value());

    m_navigation[0ul] = {};
    m_navigation[1ul] = m_finderCfg.zBinsCustomLooping;
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode
  SeedingTool::createSeeds(const EventContext& /*ctx*/,
                           const std::vector<const xAOD::SpacePoint*>& spContainer,
                           const Acts::Vector3& beamSpotPos,
                           const Acts::Vector3& bField,
                           ActsTrk::SeedContainer& seedContainer ) const
  {
    // Create Seeds
    //TODO POSSIBLE OPTIMISATION come back here: see MR !52399 ( i.e. use static thread_local)
    std::vector<Acts::Seed< xAOD::SpacePoint >> groupSeeds;
    ATH_CHECK(createSeeds(spContainer.begin(),
			  spContainer.end(),
			  beamSpotPos,
			  bField,
			  groupSeeds));
    
    // Store seeds
    seedContainer.reserve(groupSeeds.size());
    for( const auto& seed: groupSeeds) {
      std::unique_ptr< seed_type > to_add = 
	std::make_unique< seed_type >(seed);
      seedContainer.push_back(std::move(to_add));  
    }
    
    return StatusCode::SUCCESS;
  }
  
  template< typename external_iterator_t >
  StatusCode
  SeedingTool::createSeeds(external_iterator_t spBegin,
			   external_iterator_t spEnd,
			   const Acts::Vector3& beamSpotPos,
			   const Acts::Vector3& bField,
			   std::vector< seed_type >& seeds) const 
  {
    static_assert(std::is_same<typename external_spacepoint< external_iterator_t >::type, value_type>::value,
		  "Inconsistent type");
    
    seeds.clear();
    if (spBegin == spEnd)
      return StatusCode::SUCCESS;

    // Space Point Grid Options
    Acts::CylindricalSpacePointGridOptions gridOpts;
    gridOpts.bFieldInZ = bField[2];
    gridOpts = gridOpts.toInternalUnits();
    
    // Seed Finder Options
    Acts::SeedFinderOptions finderOpts;
    finderOpts.beamPos = Acts::Vector2(beamSpotPos[Amg::x], 
    		       	               beamSpotPos[Amg::y]);
    finderOpts.bFieldInZ = bField[2];
    finderOpts = finderOpts.toInternalUnits().calculateDerivedQuantities(m_finderCfg);
    
    auto extractCovariance = [](const value_type& sp, 
				float, float, float) -> std::tuple<Acts::Vector3, Acts::Vector2, std::optional<Acts::ActsScalar>> 
      {
	/// Do not convert coordinates w.r.t. beam spot
	/// Coordinates are converted internally when constructing 
	/// InternalSpacePoints 
	Acts::Vector3 position(sp.x(),
			       sp.y(),
			       sp.z());
	Acts::Vector2 covariance(sp.varianceR(), sp.varianceZ());
	return std::make_tuple(position, covariance, std::nullopt);
      };
    
    
    Acts::Extent rRangeSPExtent;
        
    Acts::CylindricalSpacePointGrid< value_type > grid =
      Acts::CylindricalSpacePointGridCreator::createGrid< value_type >(m_gridCfg, gridOpts);

    Acts::CylindricalSpacePointGridCreator::fillGrid(m_finderCfg, finderOpts, grid,
						     spBegin, spEnd, extractCovariance, rRangeSPExtent);

    Acts::CylindricalBinnedGroup< value_type > spacePointsGrouping(std::move(grid), *m_bottomBinFinder,
								   *m_topBinFinder, m_navigation);
    
    // variable middle SP radial region of interest
    const Acts::Range1D<float> rMiddleSPRange(std::floor(rRangeSPExtent.min(Acts::binR) / 2) * 2 +
					      m_finderCfg.deltaRMiddleMinSPRange,
					      std::floor(rRangeSPExtent.max(Acts::binR) / 2) * 2 -
					      m_finderCfg.deltaRMiddleMaxSPRange);
    
    //TODO POSSIBLE OPTIMISATION come back here: see MR !52399 ( i.e. use static thread_local)
    typename decltype(m_finder)::SeedingState state;
    state.spacePointData.resize(std::distance(spBegin, spEnd),
				m_useDetailedDoubleMeasurementInfo);

    if (m_useDetailedDoubleMeasurementInfo) {
      for (std::size_t idx(0); idx < spacePointsGrouping.grid().size(); ++idx) {
        const std::vector<std::unique_ptr<Acts::InternalSpacePoint<xAOD::SpacePoint>>>& collection = spacePointsGrouping.grid().at(idx);
        for (const std::unique_ptr<Acts::InternalSpacePoint<xAOD::SpacePoint>>& sp : collection) {
          std::size_t index = sp->index();

          const float topHalfStripLength =
              m_finderCfg.getTopHalfStripLength(sp->sp());
          const float bottomHalfStripLength =
              m_finderCfg.getBottomHalfStripLength(sp->sp());
          const Acts::Vector3 topStripDirection =
              m_finderCfg.getTopStripDirection(sp->sp());
          const Acts::Vector3 bottomStripDirection =
              m_finderCfg.getBottomStripDirection(sp->sp());

          state.spacePointData.setTopStripVector(
              index, topHalfStripLength * topStripDirection);
          state.spacePointData.setBottomStripVector(
              index, bottomHalfStripLength * bottomStripDirection);
          state.spacePointData.setStripCenterDistance(
              index, m_finderCfg.getStripCenterDistance(sp->sp()));
          state.spacePointData.setTopStripCenterPosition(
              index, m_finderCfg.getTopStripCenterPosition(sp->sp()));

        }
      }
    }

    for (const auto [bottom, middle, top] : spacePointsGrouping) {
      m_finder.createSeedsForGroup(finderOpts, state, spacePointsGrouping.grid(), 
          std::back_inserter(seeds), bottom, middle, top, rMiddleSPRange);
    }

    if (m_seedQualitySelection) {
      // Selection function - temporary implementation
      // need change from ACTS for final implementation
      // To be used only on PPP
      auto selectionFunction = [&state] (const Acts::Seed<xAOD::SpacePoint>& seed) -> bool 
	{
	  const xAOD::SpacePoint* bottom_sp =  seed.sp()[0];
	  const xAOD::SpacePoint* middle_sp =  seed.sp()[1];
	  const xAOD::SpacePoint* top_sp =  seed.sp()[2];
	  
	  float seed_quality = seed.seedQuality();
	  float bottom_quality = state.spacePointData.quality(bottom_sp->index());
	  float middle_quality = state.spacePointData.quality(middle_sp->index());
	  float top_quality = state.spacePointData.quality(top_sp->index());
	  
	  if (bottom_quality > seed_quality and
	      middle_quality > seed_quality and
	      top_quality > seed_quality) {
            return false;
          }

	  return true;
	};

      // Select the seeds
      std::size_t acceptedSeeds = 0;
      for (std::size_t i(0); i<seeds.size(); ++i) {
	const auto& seed = seeds[i];
	if (not selectionFunction(seed)) {
	  continue;
	}
	// move passing seeds at the beginning of the vector/collection
	// no need to swap them both. The seed currently at acceptedSeeds 
	// didn't make it (if i != acceptedSeeds)
	if (acceptedSeeds != i)
	  seeds[acceptedSeeds] = std::move(seeds[i]);
	++acceptedSeeds;
      }
      // remove seeds that didn't make it
      // they are all at the end of the collection
      seeds.erase(seeds.begin() + acceptedSeeds, seeds.end());     
    }

    return StatusCode::SUCCESS;
  }

  StatusCode 
  SeedingTool::prepareConfiguration() {
    // Prepare the Acts::SeedFinderConfig object
    // This is done only once, during initialization using the
    // parameters set in the JO

    // Configuration for Acts::SeedFinder
    m_finderCfg.minPt = m_minPt;
    m_finderCfg.cotThetaMax = m_cotThetaMax;
    m_finderCfg.impactMax = m_impactMax;
    m_finderCfg.zMin = m_zMin;
    m_finderCfg.zMax = m_zMax;
    m_finderCfg.zBinEdges = m_zBinEdges;
    m_finderCfg.rMax = m_rMax;
    m_finderCfg.binSizeR = m_binSizeR;
    m_finderCfg.deltaRMin = m_deltaRMin;
    m_finderCfg.deltaRMax = m_deltaRMax;
    m_finderCfg.deltaRMinTopSP = m_deltaRMinTopSP;
    m_finderCfg.deltaRMaxTopSP = m_deltaRMaxTopSP;
    m_finderCfg.deltaRMinBottomSP = m_deltaRMinBottomSP;
    m_finderCfg.deltaRMaxBottomSP = m_deltaRMaxBottomSP;
    m_finderCfg.deltaZMax = m_deltaZMax;
    m_finderCfg.collisionRegionMin = m_collisionRegionMin;
    m_finderCfg.collisionRegionMax = m_collisionRegionMax;
    m_finderCfg.sigmaScattering = m_sigmaScattering;
    m_finderCfg.maxPtScattering = m_maxPtScattering;
    m_finderCfg.radLengthPerSeed = m_radLengthPerSeed;
    m_finderCfg.maxSeedsPerSpM = m_maxSeedsPerSpM;
    m_finderCfg.interactionPointCut = m_interactionPointCut;
    m_finderCfg.zBinsCustomLooping = m_zBinsCustomLooping;
    m_finderCfg.useVariableMiddleSPRange = m_useVariableMiddleSPRange;
    m_finderCfg.deltaRMiddleMinSPRange = m_deltaRMiddleMinSPRange;
    m_finderCfg.deltaRMiddleMaxSPRange = m_deltaRMiddleMaxSPRange;
    m_finderCfg.seedConfirmation = m_seedConfirmation;
    m_finderCfg.centralSeedConfirmationRange.zMinSeedConf = m_seedConfCentralZMin;
    m_finderCfg.centralSeedConfirmationRange.zMaxSeedConf = m_seedConfCentralZMax;
    m_finderCfg.centralSeedConfirmationRange.rMaxSeedConf = m_seedConfCentralRMax;
    m_finderCfg.centralSeedConfirmationRange.nTopForLargeR = m_seedConfCentralNTopLargeR;
    m_finderCfg.centralSeedConfirmationRange.nTopForSmallR = m_seedConfCentralNTopSmallR;
    m_finderCfg.centralSeedConfirmationRange.seedConfMinBottomRadius = m_seedConfCentralMinBottomRadius;
    m_finderCfg.centralSeedConfirmationRange.seedConfMaxZOrigin = m_seedConfCentralMaxZOrigin;
    m_finderCfg.centralSeedConfirmationRange.minImpactSeedConf = m_seedConfCentralMinImpact;
    m_finderCfg.forwardSeedConfirmationRange.zMinSeedConf = m_seedConfForwardZMin;
    m_finderCfg.forwardSeedConfirmationRange.zMaxSeedConf = m_seedConfForwardZMax;
    m_finderCfg.forwardSeedConfirmationRange.rMaxSeedConf = m_seedConfForwardRMax;
    m_finderCfg.forwardSeedConfirmationRange.nTopForLargeR = m_seedConfForwardNTopLargeR;
    m_finderCfg.forwardSeedConfirmationRange.nTopForSmallR = m_seedConfForwardNTopSmallR;
    m_finderCfg.forwardSeedConfirmationRange.seedConfMinBottomRadius = m_seedConfForwardMinBottomRadius;
    m_finderCfg.forwardSeedConfirmationRange.seedConfMaxZOrigin = m_seedConfForwardMaxZOrigin;
    m_finderCfg.forwardSeedConfirmationRange.minImpactSeedConf = m_seedConfForwardMinImpact;
    m_finderCfg.useDetailedDoubleMeasurementInfo = m_useDetailedDoubleMeasurementInfo;
    m_finderCfg.toleranceParam = m_toleranceParam;
    m_finderCfg.phiMin = m_phiMin;
    m_finderCfg.phiMax = m_phiMax;
    m_finderCfg.rMin = m_rMin;
    m_finderCfg.zAlign = m_zAlign;
    m_finderCfg.rAlign = m_rAlign;
    m_finderCfg.sigmaError = m_sigmaError;

    if (m_useDetailedDoubleMeasurementInfo) {
      m_finderCfg.getTopHalfStripLength.connect(
        [](const void*, const value_type& sp) -> float {
          return sp.topHalfStripLength();
        });
      m_finderCfg.getBottomHalfStripLength.connect(
        [](const void*, const value_type& sp) -> float {
          return sp.bottomHalfStripLength();
        });
      m_finderCfg.getTopStripDirection.connect(
        [](const void*, const value_type& sp) -> Acts::Vector3 {
          return sp.topStripDirection().cast<double>();
        });
      m_finderCfg.getBottomStripDirection.connect(
        [](const void*, const value_type& sp) -> Acts::Vector3 {
          return sp.bottomStripDirection().cast<double>();
        });
      m_finderCfg.getStripCenterDistance.connect(
          [](const void*, const value_type& sp) -> Acts::Vector3 {
            return sp.stripCenterDistance().cast<double>();
          });
      m_finderCfg.getTopStripCenterPosition.connect(
          [](const void*, const value_type& sp) -> Acts::Vector3 {
            return sp.topStripCenter().cast<double>();
          });
    }

    // Fast tracking
    // manually convert the two types
    for (const auto& vec : m_rRangeMiddleSP) {
	std::vector<float> convertedVec;
	
	for (const auto& val : vec) {
	    convertedVec.push_back(static_cast<float>(val));
	}
	
	m_finderCfg.rRangeMiddleSP.push_back(convertedVec);
    }
    
    // define cuts used for fast tracking configuration
    if (m_useExperimentCuts) {
      m_finderCfg.experimentCuts.connect(
					 [](const void*, float bottomRadius, float cotTheta) -> bool {
					   
					   float fastTrackingRMin = 50.;
					   float fastTrackingCotThetaMax = 1.5;
					   
					   if (bottomRadius < fastTrackingRMin and
					       (cotTheta > fastTrackingCotThetaMax or
						cotTheta < -fastTrackingCotThetaMax)) {
					     return false;
					   }
					   return true;
					 });
    }
    
    // Configuration for Acts::SeedFilter (used by FinderCfg)
    Acts::SeedFilterConfig filterCfg;
    filterCfg.deltaRMin = m_deltaRMin;
    filterCfg.maxSeedsPerSpM = m_maxSeedsPerSpM;
    filterCfg.useDeltaRorTopRadius = m_useDeltaRorTopRadius;
    filterCfg.seedConfirmation = m_seedConfirmationInFilter;
    filterCfg.maxSeedsPerSpMConf = m_maxSeedsPerSpMConf;
    filterCfg.maxQualitySeedsPerSpMConf = m_maxQualitySeedsPerSpMConf;
    filterCfg.centralSeedConfirmationRange = m_finderCfg.centralSeedConfirmationRange;
    filterCfg.forwardSeedConfirmationRange = m_finderCfg.forwardSeedConfirmationRange;
    filterCfg.impactWeightFactor = m_impactWeightFactor;
    filterCfg.zOriginWeightFactor = m_zOriginWeightFactor;
    filterCfg.compatSeedWeight = m_compatSeedWeight;
    filterCfg.compatSeedLimit = m_compatSeedLimit;
    filterCfg.seedWeightIncrement = m_seedWeightIncrement;
    filterCfg.numSeedIncrement = m_numSeedIncrement;
    filterCfg.deltaInvHelixDiameter = m_deltaInvHelixDiameter;
    m_finderCfg.seedFilter = std::make_unique<Acts::SeedFilter< value_type > >(filterCfg.toInternalUnits());    

    m_finderCfg = m_finderCfg.toInternalUnits().calculateDerivedQuantities();

    // Grid Configuration
    m_gridCfg.minPt = m_minPt;
    m_gridCfg.cotThetaMax = m_cotThetaMax;
    m_gridCfg.impactMax = m_impactMax;
    m_gridCfg.zMin = m_zMin;
    m_gridCfg.zMax = m_zMax;
    m_gridCfg.phiMin = m_gridPhiMin;
    m_gridCfg.phiMax = m_gridPhiMax;
    m_gridCfg.zBinEdges = m_zBinEdges;
    m_gridCfg.deltaRMax = m_deltaRMax;
    m_gridCfg.rMax = m_gridRMax;
    m_gridCfg.phiBinDeflectionCoverage = m_phiBinDeflectionCoverage;
    m_gridCfg.maxPhiBins = m_maxPhiBins;
    m_gridCfg = m_gridCfg.toInternalUnits();

    // Seed Finder
    m_finder = Acts::SeedFinder< value_type >(m_finderCfg);
 
    return StatusCode::SUCCESS;
  }

} // namespace ActsTrk
