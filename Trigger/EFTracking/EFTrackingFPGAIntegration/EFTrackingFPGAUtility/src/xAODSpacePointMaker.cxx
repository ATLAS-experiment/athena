/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/xAODSpacePointMaker.cxx
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @author levi.samuel.evans@cern.ch
 * @date Mar. 11, 2025
 */

#include "EFTrackingFPGAUtility/xAODSpacePointMaker.h"

#include "Identifier/Identifier.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

StatusCode xAODSpacePointMaker::initialize() {
  ATH_MSG_INFO("Initialising xAODSpacePointMaker tool");

  ATH_CHECK(m_pixelClusterKey.initialize());
  ATH_CHECK(m_stripClusterKey.initialize());
  
  ATH_CHECK(m_pixelSpacePointsKey.initialize());
  ATH_CHECK(m_stripSpacePointsKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode xAODSpacePointMaker::makePixelSpacePointContainer(
    const EFTrackingTransient::SpacePointAuxInput& spAux,
    const EFTrackingTransient::Metadata* metadata,
    const EventContext& ctx) const {
    
    if (msgLvl(MSG::DEBUG)) {
        ATH_MSG_DEBUG("Making xAOD::SpacePointContainer from SpacePointAuxInput");
        
        // debugging information about vector sizes
        ATH_MSG_DEBUG("Metadata numOfPixelSpacePoints: " << metadata->numOfPixelSpacePoints);
        ATH_MSG_DEBUG("spAux.elementIdList size: " << spAux.elementIdList.size());
        ATH_MSG_DEBUG("spAux.globalPosition size: " << spAux.globalPosition.size());
        ATH_MSG_DEBUG("spAux.measurementIndexes size: " << spAux.measurementIndexes.size());
        ATH_MSG_DEBUG("spAux.varianceR size: " << spAux.varianceR.size());
        ATH_MSG_DEBUG("spAux.varianceZ size: " << spAux.varianceZ.size());
    }

    // Check for inconsistent metadata and throw an error if it is
    if (metadata->numOfPixelSpacePoints > 0 && 
        (spAux.elementIdList.empty() || 
         spAux.globalPosition.empty() || 
         spAux.measurementIndexes.empty() ||
         spAux.varianceR.empty() ||
         spAux.varianceZ.empty())) {
        ATH_MSG_WARNING("Inconsistent metadata: numOfPixelSpacePoints = " 
                     << metadata->numOfPixelSpacePoints 
                     << " but one or more required vectors are empty");
        return StatusCode::SUCCESS;
    }

    if (metadata->numOfPixelSpacePoints > 0 && spAux.elementIdList.empty()) {
        ATH_MSG_WARNING("Inconsistent metadata!");
    }

    // Create the containers
    SG::WriteHandle<xAOD::SpacePointContainer> pixelSpacePointsHandle(m_pixelSpacePointsKey, ctx);
    ATH_CHECK(pixelSpacePointsHandle.record(
        std::make_unique<xAOD::SpacePointContainer>(),
        std::make_unique<xAOD::SpacePointAuxContainer>()));
    
    ATH_CHECK(pixelSpacePointsHandle.isValid());
    ATH_MSG_DEBUG("Container '" << m_pixelSpacePointsKey << "' initialised");

    // Get pixel cluster container for linking
    SG::ReadHandle<xAOD::PixelClusterContainer> pixelClusters(m_pixelClusterKey, ctx);
    if (!pixelClusters.isValid()) {
        ATH_MSG_ERROR("Could not retrieve pixel cluster container");
        return StatusCode::FAILURE;
    }

    // Create spacepoints
    for (unsigned int i = 0; i < metadata->numOfPixelSpacePoints; i++) {
        // Add bounds checking for all vector accesses
        if (i >= spAux.elementIdList.size()) {
            ATH_MSG_ERROR("Index out of bounds: i=" << i 
                         << " >= elementIdList.size()=" << spAux.elementIdList.size());
            return StatusCode::FAILURE;
        }
        
        if (i >= spAux.varianceR.size()) {
            ATH_MSG_ERROR("Index out of bounds: i=" << i 
                         << " >= varianceR.size()=" << spAux.varianceR.size());
            return StatusCode::FAILURE;
        }
        
        if (i >= spAux.varianceZ.size()) {
            ATH_MSG_ERROR("Index out of bounds: i=" << i 
                         << " >= varianceZ.size()=" << spAux.varianceZ.size());
            return StatusCode::FAILURE;
        }
        
        if (i >= spAux.measurementIndexes.size()) {
            ATH_MSG_ERROR("Index out of bounds: i=" << i 
                         << " >= measurementIndexes.size()=" << spAux.measurementIndexes.size());
            return StatusCode::FAILURE;
        }
        
        if (i*3+2 >= spAux.globalPosition.size()) {
            ATH_MSG_ERROR("Index out of bounds: i*3+2=" << (i*3+2) 
                         << " >= globalPosition.size()=" << spAux.globalPosition.size());
            return StatusCode::FAILURE;
        }

        // use unique_ptr to avoid memory leak
        auto sp = pixelSpacePointsHandle->push_back(std::make_unique<xAOD::SpacePoint>());

        // Create position vector
        Eigen::Matrix<float, 3, 1> globalPosition(
            spAux.globalPosition.at(i * 3),
            spAux.globalPosition.at(i * 3 + 1),
            spAux.globalPosition.at(i * 3 + 2));

        // Get the measurement index
        const int measIdx = spAux.measurementIndexes.at(i);

        // Create the spacepoint with empty measurements vector
        sp->setSpacePoint(
            spAux.elementIdList[i],
            globalPosition,
            spAux.varianceR[i],
            spAux.varianceZ[i],
            std::vector<const xAOD::UncalibratedMeasurement*>()); // Empty vector

        // Create ElementLink to the pixel cluster
        if (measIdx >= 0 && static_cast<size_t>(measIdx) < pixelClusters->size()) {
            ElementLink<xAOD::PixelClusterContainer> link(*pixelClusters, measIdx);
            static const SG::Decorator<ElementLink<xAOD::PixelClusterContainer>> dec("fpgaPixelClusterLink");
            dec(*sp) = link;
        }
    }

    // After creating all spacepoints print the number of spacepoints created in debug stream
    ATH_MSG_DEBUG("Created " << pixelSpacePointsHandle->size() << " pixel spacepoints");

    return StatusCode::SUCCESS;
}

StatusCode xAODSpacePointMaker::makeStripSpacePointContainer(
    const EFTrackingTransient::SpacePointAuxInput& sspAux,
    const EFTrackingTransient::Metadata* metadata,
    const EventContext& ctx) const {
    ATH_MSG_DEBUG("Making xAOD::SpacePointContainer from SpacePointAuxInput");
    
    // debug stream for information about vector sizes
    ATH_MSG_DEBUG("Metadata numOfStripSpacePoints: " << metadata->numOfStripSpacePoints);
    ATH_MSG_DEBUG("sspAux.elementIdList size: " << sspAux.elementIdList.size());
    ATH_MSG_DEBUG("sspAux.globalPosition size: " << sspAux.globalPosition.size());
    ATH_MSG_DEBUG("sspAux.measurementIndexes size: " << sspAux.measurementIndexes.size());
    ATH_MSG_DEBUG("sspAux.varianceR size: " << sspAux.varianceR.size());
    ATH_MSG_DEBUG("sspAux.varianceZ size: " << sspAux.varianceZ.size());

    // Check for inconsistent metadata and throw an error if it is
    if (metadata->numOfStripSpacePoints > 0 && 
        (sspAux.elementIdList.empty() || 
         sspAux.globalPosition.empty() || 
         sspAux.measurementIndexes.empty() ||
         sspAux.varianceR.empty() ||
         sspAux.varianceZ.empty())) {
        ATH_MSG_ERROR("Inconsistent metadata: numOfStripSpacePoints = " 
                     << metadata->numOfStripSpacePoints 
                     << " but one or more required vectors are empty");
        return StatusCode::FAILURE;
    }

    if (metadata->numOfStripSpacePoints > 0 && sspAux.elementIdList.empty()) {
        ATH_MSG_WARNING("Inconsistent metadata!");
    }

    // create the containers
    SG::WriteHandle<xAOD::SpacePointContainer> stripSpacePointsHandle(m_stripSpacePointsKey, ctx);
    ATH_CHECK(stripSpacePointsHandle.record(
        std::make_unique<xAOD::SpacePointContainer>(),
        std::make_unique<xAOD::SpacePointAuxContainer>()));
    
    ATH_CHECK(stripSpacePointsHandle.isValid());
    ATH_MSG_DEBUG("Container '" << m_stripSpacePointsKey << "' initialised");

    // get the strip cluster container for linking
    SG::ReadHandle<xAOD::StripClusterContainer> stripClusters(m_stripClusterKey, ctx);
    if (!stripClusters.isValid()) {
        ATH_MSG_ERROR("Could not retrieve strip cluster container");
        return StatusCode::FAILURE;
    }

    // create the spacepoints
    for (unsigned int i = 0; i < metadata->numOfStripSpacePoints; i++) {
        // add bounds checking for all vector accesses
        if (i >= sspAux.elementIdList.size()) {
            ATH_MSG_ERROR("Index out of bounds: i=" << i 
                         << " >= elementIdList.size()=" << sspAux.elementIdList.size());
            return StatusCode::FAILURE;
        }
        
        if (i >= sspAux.varianceR.size()) {
            ATH_MSG_ERROR("Index out of bounds: i=" << i 
                         << " >= varianceR.size()=" << sspAux.varianceR.size());
            return StatusCode::FAILURE;
        }
        
        if (i >= sspAux.varianceZ.size()) {
            ATH_MSG_ERROR("Index out of bounds: i=" << i 
                         << " >= varianceZ.size()=" << sspAux.varianceZ.size());
            return StatusCode::FAILURE;
        }
        
        if (i*2+1 >= sspAux.measurementIndexes.size()) {
            ATH_MSG_ERROR("Index out of bounds: i*2+1=" << (i*2+1) 
                         << " >= measurementIndexes.size()=" << sspAux.measurementIndexes.size());
            return StatusCode::FAILURE;
        }
        
        if (i*3+2 >= sspAux.globalPosition.size()) {
            ATH_MSG_ERROR("Index out of bounds: i*3+2=" << (i*3+2) 
                         << " >= globalPosition.size()=" << sspAux.globalPosition.size());
            return StatusCode::FAILURE;
        }

        // use unique_ptr here to avoid any memory leaks
        auto ssp = stripSpacePointsHandle->push_back(std::make_unique<xAOD::SpacePoint>());

        // Create position vector
        Eigen::Matrix<float, 3, 1> globalPosition(
            sspAux.globalPosition.at(i * 3),
            sspAux.globalPosition.at(i * 3 + 1),
            sspAux.globalPosition.at(i * 3 + 2));

        // Get the measurement index
        const int meas_idx1 = sspAux.measurementIndexes.at(i * 2);
        const int meas_idx2 = sspAux.measurementIndexes.at(i * 2 + 1);

        // For strip spacepoints, we need to use the version with two element IDs!
        std::vector<xAOD::DetectorIDHashType> elementIds;
        elementIds.push_back(sspAux.elementIdList.at(i * 2));
        elementIds.push_back(sspAux.elementIdList.at(i * 2 + 1));
        
        
        // Get strip-specific properties
        float topHalfStripLength = 0.0f;
        float bottomHalfStripLength = 0.0f;
        
        if (i < sspAux.topHalfStripLength.size()) {
            topHalfStripLength = sspAux.topHalfStripLength.at(i);
        }
        
        if (i < sspAux.bottomHalfStripLength.size()) {
            bottomHalfStripLength = sspAux.bottomHalfStripLength.at(i);
        }
        
        // Create direction vectors
        Eigen::Matrix<float, 3, 1> topStripDirection = Eigen::Matrix<float, 3, 1>::Zero();
        if (i * 3 + 2 < sspAux.topStripDirection.size()) {
            topStripDirection = Eigen::Matrix<float, 3, 1>(
                sspAux.topStripDirection.at(i * 3),
                sspAux.topStripDirection.at(i * 3 + 1),
                sspAux.topStripDirection.at(i * 3 + 2));
        }
        
        Eigen::Matrix<float, 3, 1> bottomStripDirection = Eigen::Matrix<float, 3, 1>::Zero();
        if (i * 3 + 2 < sspAux.bottomStripDirection.size()) {
            bottomStripDirection = Eigen::Matrix<float, 3, 1>(
                sspAux.bottomStripDirection.at(i * 3),
                sspAux.bottomStripDirection.at(i * 3 + 1),
                sspAux.bottomStripDirection.at(i * 3 + 2));
        }
        
        Eigen::Matrix<float, 3, 1> stripCenterDistance = Eigen::Matrix<float, 3, 1>::Zero();
        if (i * 3 + 2 < sspAux.stripCenterDistance.size()) {
            stripCenterDistance = Eigen::Matrix<float, 3, 1>(
                sspAux.stripCenterDistance.at(i * 3),
                sspAux.stripCenterDistance.at(i * 3 + 1),
                sspAux.stripCenterDistance.at(i * 3 + 2));
        }
        
        Eigen::Matrix<float, 3, 1> topStripCenter = Eigen::Matrix<float, 3, 1>::Zero();
        if (i * 3 + 2 < sspAux.topStripCenter.size()) {
            topStripCenter = Eigen::Matrix<float, 3, 1>(
                sspAux.topStripCenter.at(i * 3),
                sspAux.topStripCenter.at(i * 3 + 1),
                sspAux.topStripCenter.at(i * 3 + 2));
        }
        
        // Create the spacepoint with all required parameters
        ssp->setSpacePoint(
            std::move(elementIds),
            globalPosition, 
            sspAux.varianceR.at(i), 
            sspAux.varianceZ.at(i),
            std::vector<const xAOD::UncalibratedMeasurement*>(),  // Empty vector of strip measurements
            topHalfStripLength,
            bottomHalfStripLength,
            topStripDirection,
            bottomStripDirection,
            stripCenterDistance,
            topStripCenter);

        // Instead of storing measurements directly, store ElementLinks
        if (meas_idx1 >= 0 && static_cast<size_t>(meas_idx1) < stripClusters->size() &&
            meas_idx2 >= 0 && static_cast<size_t>(meas_idx2) < stripClusters->size()) {
            
            // Create ElementLinks to the strip clusters
            ElementLink<xAOD::StripClusterContainer> link1(*stripClusters, meas_idx1);
            ElementLink<xAOD::StripClusterContainer> link2(*stripClusters, meas_idx2);
            
            // Store the links as auxiliary data
            static const SG::Decorator<ElementLink<xAOD::StripClusterContainer>> dec1("fpgaStripClusterLink1");
            static const SG::Decorator<ElementLink<xAOD::StripClusterContainer>> dec2("fpgaStripClusterLink2");
            dec1(*ssp) = link1;
            dec2(*ssp) = link2;
        }

        // set the global position of the space point
        ssp->globalPosition() = globalPosition;

        // validate the position of the space point
        ATH_MSG_DEBUG("Strip Spacepoint " << i << " position: (" 
                     << globalPosition(0) << ", "
                     << globalPosition(1) << ", "
                     << globalPosition(2) << ")");
    }
    
    ATH_MSG_DEBUG("Created " << stripSpacePointsHandle->size() << " strip spacepoints");
    
    return StatusCode::SUCCESS;
} 
