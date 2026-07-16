/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#ifndef ACTSCALOALGS_CALOEXTENSIONALG_H
#define ACTSCALOALGS_CALOEXTENSIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "ActsEvent/CaloExtension.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"

#include "egammaInterfaces/IegammaCaloClusterSelector.h"

#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"


#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Utilities/Grid.hpp"

#include <span>
#include <map>

namespace ActsTrk{
    /** @brief The CaloExtensionAlg takes the ID / ITk tracks from their track state
     *         of their last measurement and propagates them to the exit portal of 
     *         the calorimeter tracking volume. Surface crossing along the track 
     *         are recorded as @ref Acts::BoundTrackParameters and then stored in
     *         the `CaloExtension` object which is written to StoreGate but also
     *         decorated to the TrackParticle via an ElementLink.
     * 
     *         CaloClusters are then sorted into an eta/phi grid to speed-up the matching
     *         with the ITk tracks. Coarsely matched clusters are then matched to particular
     *         track parameters that have been recorded before.
     */
    class CaloExtensionAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;

        private:
            /** @brief Use an Acts::Grid to segment the cluster in eta / phi space
             *         to reduce the matching combinatorics between tracks and clusters
             *         First define some abrivations to compactify the syntax
             *    --> Abrivation of the CaloClusters per grid cell */
            using ClusterVec_t = std::vector<const xAOD::CaloCluster*>;
            /** @brief Along eta define an equidistance binning without any overflow bin */
            using EtaAxis_t = Acts::Axis<Acts::AxisType::Equidistant, 
                                         Acts::AxisBoundaryType::Bound>;
            /** @brief Along phi define an equidistance binning but with a circular
             *         wrapping connection between the two edge bins */
            using PhiAxis_t = Acts::Axis<Acts::AxisType::Equidistant, 
                                         Acts::AxisBoundaryType::Closed>;
            /** @brief Define the segmen cluster grid made out of calo cluster containers
             *         segmented in eta & phi */
            using SortedCluster_t = Acts::Grid<ClusterVec_t, EtaAxis_t, PhiAxis_t>;

            /** @brief Selects the calo clusters for the matching to the ID tracks 
             *         based on the eta range and on a minimum threshold on the energy.
             *         Optionally, the selection tool can be used which requires the 
             *         use of the CaloDetDescrManager. The Clusters are sorted into
             *         a grid which dimension in eta is defined by @ref m_maxClustEta.
             *         The number of bins in the two dimensions are determined by 
             *         the @ref m_broadDeltaEta and @ref m_broadDeltaPhi windows, respectively.
             * @param clusters: The calorimeter cluster vector to sort into the grid
             * @param detMgr: Calo description manager needed if the calo selection tool is used. */
            SortedCluster_t selectAndSort(const xAOD::CaloClusterContainer& clusters,
                                          const CaloDetDescrManager* detMgr) const;
                                          
            /** @brief Propagates the Track from the last track measurement to the calorimeter exit and
             *         records all surface crossings between as @ref Acts::BoundTrackParameters. If one
             *         record happened, the result is packed into a @ref CaloExtension object and returned.
             *         A nullptr is given back otherwise. 
             * @param mfContext: The ATLAS magnetic field conditions
             * @param tgContext: The ATLAS geometry context to align the surfaces in space
             * @param track: The ID track particle to be propagated to the exit */
            std::unique_ptr<CaloExtension> propagateToCaloExit(const Acts::MagneticFieldContext& mfContext,
                                                               const Acts::GeometryContext& tgContext,
                                                               const xAOD::TrackParticle* track) const;

            /** @brief Match the selected calorimeter clusters to the calo extension
             *  @param tgContext: The geometry context to align the bound track parameters
             *  @param clusterContainer: List of pre-selected clusters
             *  @param caloExtension: Mutable reference to the calo extension where the calorimeter
             *                         clusters are appended if they satisfiy the dR distance criteria */
            void matchClusters(const Acts::GeometryContext& tgContext,
                               std::span<const xAOD::CaloCluster* const> clusterContainer,
                               CaloExtension& caloExtension) const;

            /** @brief Checks whether the track @ its last measurement state
             *         is roughly compatible with the calorimeter cluster
             *  @param tgContext: The geometry context to align the bound track parameters
             *  @param cluster: Calorimeter cluster to consider
             *  @param lastTrkPars: The last track parameters of the ID track */
            bool checkBroadCriteria(const Acts::GeometryContext& tgContext,
                                    const xAOD::CaloCluster& cluster,
                                    const Acts::BoundTrackParameters& lastTrkPars) const;
            
            /** @brief Setup an own instance of the Acts Prpoagator to schedule the 
             *         BoundTrackParameter recording & to define a more prope abort
             *         condition */
            using CurvedStepper_t = Acts::EigenStepper<Acts::EigenStepperDefaultExtension>;
            using CurvedPropagator_t = Acts::Propagator<CurvedStepper_t, Acts::Navigator>;
            /** @brief The instance of the propagator */
            std::unique_ptr<CurvedPropagator_t> m_propagator{};
            /** @brief Track quality selection tool (optional) */
            ToolHandle<InDet::IInDetTrackSelectionTool> m_trackSelector{this, "TrackSelectionTool" , ""};
            /** @brief Tool to filter the calo clusters. */
            ToolHandle<IegammaCaloClusterSelector> m_clusterSelector{this, "ClusterSelector", "egammaCaloClusterSelector", 
                                                                                "Tool that makes the cluster selection"};
            /** @brief Tracking geometry tool */
            PublicToolHandle<ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
            
            /** @brief The input calorimeter cluster collection */
            SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clusterContainerKey {this,  "ClusterContainerName", "CaloCalTopoClusters"};
            /** @brief The input track particle collection */
            SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerKey{this, "TrackParticleContainerName",
                                                                                         "InDetTrackParticles" };
            /** @brief Decorate the link to the associated CaloExtension directly onto the ID / ITk track */
            SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_extensionDecorKey{this, "ExtensionDecoration", m_trackParticleContainerKey, 
                                                                                     "caloExtensionLink"};
            /** @brief Dependency on the ATLAS magnetic field needed to propagate the tracks */
            SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};
            /** @brief Calo description manager */
            SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey {this, "CaloDetDescrManager", "CaloDetDescrManager"};

            SG::WriteHandleKey<CaloExtensionContainer> m_caloExtensionKey{this, "CaloExtensionName", "CaloExtension"};

            /** @brief The minimum momentum cut applied on the ID tracks to be considered */
            Gaudi::Property<float> m_trackPt{this, "minPt", 2.5*Gaudi::Units::GeV};

            /** @brief Broad windowsto initially match the calo cluster with the track exit parameters */
            Gaudi::Property<float> m_broadDeltaEta{this, "broadDeltaEta", 0.2, "Value of broad cut for delta eta" };

            Gaudi::Property<float> m_broadDeltaPhi{this, "broadDeltaPhi", 0.3, "Value of broad cut for delta phi" };
            /** @brief Minimum requirement on the cluster's transversal energy in order to be considered */
            Gaudi::Property<float> m_minClustEt{this, "minClusterEt", 10.*Gaudi::Units::MeV};
            /** @brief Maximum cut on the cluster's eta in order to be considered */
            Gaudi::Property<float> m_maxClustEta{this, "maxClustEta", 10.};


            using CaloSample = xAOD::CaloCluster::CaloSample;
            /** @brief Narrow windows. */
            Gaudi::Property<float> m_narrowDeltaEta{this, "narrowDeltaEta", 0.05};

            Gaudi::Property<float> m_narrowDeltaPhi{this, "narrowDeltaPhi", 0.05};

            std::unordered_map<Acts::GeometryIdentifier, CaloSample> m_geoLayerIds{};

    };
}

#endif