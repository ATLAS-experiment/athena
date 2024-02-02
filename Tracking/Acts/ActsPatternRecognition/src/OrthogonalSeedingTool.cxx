/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/OrthogonalSeedingTool.h"

namespace ActsTrk {

  OrthogonalSeedingTool::OrthogonalSeedingTool(const std::string& type,
    const std::string& name,
    const IInterface* parent)
    : base_class(type, name, parent)
  {}

  StatusCode OrthogonalSeedingTool::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG(" *  Used by SeedFinderOrthogonalConfig");
    ATH_MSG_DEBUG("   " << m_minPt);
    ATH_MSG_DEBUG("   " << m_cotThetaMax);
    ATH_MSG_DEBUG("   " << m_deltaRMinTopSP);
    ATH_MSG_DEBUG("   " << m_deltaRMaxTopSP);
    ATH_MSG_DEBUG("   " << m_deltaRMinBottomSP);
    ATH_MSG_DEBUG("   " << m_deltaRMaxBottomSP);

    ATH_MSG_DEBUG("   " << m_impactMax);
    ATH_MSG_DEBUG("   " << m_sigmaScattering);
    ATH_MSG_DEBUG("   " << m_maxPtScattering);
    ATH_MSG_DEBUG("   " << m_maxSeedsPerSpM);

    ATH_MSG_DEBUG("   " << m_collisionRegionMin);
    ATH_MSG_DEBUG("   " << m_collisionRegionMax);
    ATH_MSG_DEBUG("   " << m_phiMin);
    ATH_MSG_DEBUG("   " << m_phiMax);
    ATH_MSG_DEBUG("   " << m_zMin);
    ATH_MSG_DEBUG("   " << m_zMax);
    ATH_MSG_DEBUG("   " << m_rMax);
    ATH_MSG_DEBUG("   " << m_rMin);

    ATH_MSG_DEBUG("   " << m_rMinMiddle);
    ATH_MSG_DEBUG("   " << m_rMaxMiddle);
    ATH_MSG_DEBUG("   " << m_deltaPhiMax);
    ATH_MSG_DEBUG("   " << m_deltaZMax);

    ATH_MSG_DEBUG("   " << m_interactionPointCut);
    ATH_MSG_DEBUG("   " << m_seedConfirmation);
    ATH_MSG_DEBUG("   " << m_radLengthPerSeed);

    ATH_MSG_DEBUG(" *  Used by SeedFilterConfig" );
    ATH_MSG_DEBUG("   " << m_deltaInvHelixDiameter);
    ATH_MSG_DEBUG("   " << m_impactWeightFactor);
    ATH_MSG_DEBUG("   " << m_zOriginWeightFactor);
    ATH_MSG_DEBUG("   " << m_compatSeedWeight);
    ATH_MSG_DEBUG("   " << m_deltaRMin);
    ATH_MSG_DEBUG("   " << m_maxSeedsPerSpM);
    ATH_MSG_DEBUG("   " << m_compatSeedLimit);

    ATH_MSG_DEBUG("   " << m_seedWeightIncrement);
    ATH_MSG_DEBUG("   " << m_numSeedIncrement);

    ATH_MSG_DEBUG("   " << m_seedConfirmationInFilter);
    ATH_MSG_DEBUG("   " << m_maxSeedsPerSpMConf);
    ATH_MSG_DEBUG("   " << m_maxQualitySeedsPerSpMConf);
    ATH_MSG_DEBUG("   " << m_useDeltaRorTopRadius);

    ATH_MSG_DEBUG(" *  Used by SeedFilterConfig" );
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

    ATH_CHECK( prepareConfiguration() );

    return StatusCode::SUCCESS;
  }

  StatusCode
  OrthogonalSeedingTool::createSeeds(const EventContext& /*ctx*/,
				     const std::vector<const xAOD::SpacePoint*>& spContainer,
				     const Acts::Vector3& beamSpotPos,
				     const Acts::Vector3& bField,
				     ActsTrk::SeedContainer& seedContainer ) const
  {
    // Seed Finder Options
    Acts::SeedFinderOptions finderOpts;
    finderOpts.beamPos = Acts::Vector2(beamSpotPos[Amg::x],
                                       beamSpotPos[Amg::y]);
    finderOpts.bFieldInZ = bField[2];
    finderOpts = finderOpts.toInternalUnits().calculateDerivedQuantities(m_finderCfg);

    std::function<std::tuple<Acts::Vector3, Acts::Vector2, std::optional<Acts::ActsScalar>>(const xAOD::SpacePoint *sp)>
      create_coordinates = [](const xAOD::SpacePoint *sp) {
      Acts::Vector3 position(sp->x(), sp->y(), sp->z());
      Acts::Vector2 variance(sp->varianceR(), sp->varianceZ());
      return std::make_tuple(position, variance, std::nullopt);
    };
    
    // Compute seeds
    auto groupSeeds = m_finder.createSeeds(finderOpts, spContainer,
					   create_coordinates);

    // Store seeds
    seedContainer.reserve(groupSeeds.size());
    for( const auto& seed: groupSeeds) {
      std::unique_ptr<seed_type> to_add = std::make_unique<seed_type>(seed);
      seedContainer.push_back(std::move(to_add));  
    }

    return StatusCode::SUCCESS;
  }

  StatusCode
  OrthogonalSeedingTool::prepareConfiguration()
  {
    // Configuration for Acts::SeedFilter
    Acts::SeedFilterConfig filterCfg;
    filterCfg.deltaInvHelixDiameter = m_deltaInvHelixDiameter;
    filterCfg.impactWeightFactor = m_impactWeightFactor;
    filterCfg.zOriginWeightFactor = m_zOriginWeightFactor;
    filterCfg.compatSeedWeight = m_compatSeedWeight;    
    filterCfg.deltaRMin = m_deltaRMin;
    filterCfg.maxSeedsPerSpM = m_maxSeedsPerSpM;
    filterCfg.compatSeedLimit = m_compatSeedLimit;
    filterCfg.seedWeightIncrement = m_seedWeightIncrement;
    filterCfg.numSeedIncrement = m_numSeedIncrement;
    filterCfg.seedConfirmation = m_seedConfirmationInFilter;
    filterCfg.maxSeedsPerSpMConf = m_maxSeedsPerSpMConf;
    filterCfg.maxQualitySeedsPerSpMConf = m_maxQualitySeedsPerSpMConf;
    filterCfg.useDeltaRorTopRadius = m_useDeltaRorTopRadius;
    filterCfg.centralSeedConfirmationRange.zMinSeedConf = m_seedConfCentralZMin;
    filterCfg.centralSeedConfirmationRange.zMaxSeedConf = m_seedConfCentralZMax;
    filterCfg.centralSeedConfirmationRange.rMaxSeedConf = m_seedConfCentralRMax;
    filterCfg.centralSeedConfirmationRange.nTopForLargeR = m_seedConfCentralNTopLargeR;
    filterCfg.centralSeedConfirmationRange.nTopForSmallR = m_seedConfCentralNTopSmallR;
    filterCfg.centralSeedConfirmationRange.seedConfMinBottomRadius = m_seedConfCentralMinBottomRadius;
    filterCfg.centralSeedConfirmationRange.seedConfMaxZOrigin = m_seedConfCentralMaxZOrigin;
    filterCfg.centralSeedConfirmationRange.minImpactSeedConf = m_seedConfCentralMinImpact;
    filterCfg.forwardSeedConfirmationRange.zMinSeedConf = m_seedConfForwardZMin;
    filterCfg.forwardSeedConfirmationRange.zMaxSeedConf = m_seedConfForwardZMax;
    filterCfg.forwardSeedConfirmationRange.rMaxSeedConf = m_seedConfForwardRMax;
    filterCfg.forwardSeedConfirmationRange.nTopForLargeR = m_seedConfForwardNTopLargeR;
    filterCfg.forwardSeedConfirmationRange.nTopForSmallR = m_seedConfForwardNTopSmallR;
    filterCfg.forwardSeedConfirmationRange.seedConfMinBottomRadius = m_seedConfForwardMinBottomRadius;
    filterCfg.forwardSeedConfirmationRange.seedConfMaxZOrigin = m_seedConfForwardMaxZOrigin;
    filterCfg.forwardSeedConfirmationRange.minImpactSeedConf = m_seedConfForwardMinImpact;
    
    // Configuration Acts::SeedFinderOrthogonal
    m_finderCfg.seedFilter = std::make_shared<Acts::SeedFilter<value_type>>(filterCfg.toInternalUnits()); 
    m_finderCfg.cotThetaMax = m_cotThetaMax;
    m_finderCfg.deltaRMinTopSP = m_deltaRMinTopSP;
    m_finderCfg.deltaRMaxTopSP = m_deltaRMaxTopSP;
    m_finderCfg.deltaRMinBottomSP = m_deltaRMinBottomSP;
    m_finderCfg.deltaRMaxBottomSP = m_deltaRMaxBottomSP;
    m_finderCfg.impactMax = m_impactMax;
    m_finderCfg.sigmaScattering = m_sigmaScattering;
    m_finderCfg.maxPtScattering = m_maxPtScattering;
    m_finderCfg.maxSeedsPerSpM = m_maxSeedsPerSpM;
    m_finderCfg.collisionRegionMin = m_collisionRegionMin;
    m_finderCfg.collisionRegionMax = m_collisionRegionMax;
    m_finderCfg.phiMin = m_phiMin;
    m_finderCfg.phiMax = m_phiMax;
    m_finderCfg.zMin = m_zMin;
    m_finderCfg.zMax = m_zMax;
    m_finderCfg.rMax = m_rMax;
    m_finderCfg.rMin = m_rMin;
    m_finderCfg.rMinMiddle = m_rMinMiddle;
    m_finderCfg.rMaxMiddle = m_rMaxMiddle;
    m_finderCfg.deltaPhiMax = m_deltaPhiMax;
    m_finderCfg.deltaZMax = m_deltaZMax;
    m_finderCfg.interactionPointCut = m_interactionPointCut;
    m_finderCfg.seedConfirmation = m_seedConfirmation;
    m_finderCfg.centralSeedConfirmationRange = filterCfg.centralSeedConfirmationRange;
    m_finderCfg.forwardSeedConfirmationRange = filterCfg.forwardSeedConfirmationRange;
    m_finderCfg.radLengthPerSeed = m_radLengthPerSeed;
    m_finderCfg = m_finderCfg.toInternalUnits();

    m_finder = Acts::SeedFinderOrthogonal<value_type>(m_finderCfg);

    return StatusCode::SUCCESS;
  }

} // namespace ActsTrk
