/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFTRACKING_ACTSCLUSTERCOMPARISONALG_H
#define EFTRACKING_ACTSCLUSTERCOMPARISONALG_H

// Athena includes
#include <array>
#include <optional>
#include <unordered_map>
#include <utility>

#include "Acts/Definitions/Units.hpp"
#include "ActsEvent/TrackContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetReadoutGeometry/SiDetectorDesign.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"
#include "ActsGPUEvent/GeometryIdMapping.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

#include <Gaudi/Accumulators.h>

namespace ActsTrk {

/**
 * @class Cluster Comparison Algorithm
 *
 * @brief Algorithm comparing pixel and strip xAOD clusters and xAOD spacepoint containers to each other
 *
 * This algorithm retrieves the input xAOD containers from event store,
 * compares the initial numbers of clusters found (globally and per module),
 * then creates cluster pairs by comparing the clusters based on module ID and associated RDO content.
 * In debug/verbose mode it provides a detailed print of cluster properties
 * when the cluster pair displays unusually large positional discrepancy (> 0.25 sigma)
 *
 * For spacepoint validation (pixel only for now), cluster validation takes place first to create
 * spacepoint pairs based on the association to clusters and their RDO content.
 *
 * This algorithm was mainly developed to augment the cluster validation in IDTPM
 * by providing a more detailed analysis for the validation of GPU developemnts in EF Tracking
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class ActsClusterComparisonAlg : public AthReentrantAlgorithm {
public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        /// Initialize the algorithm.
        virtual StatusCode initialize() override;
        /// Execute the algorithm.
        virtual StatusCode execute(const EventContext& ctx) const override;
        /// Finalize the algorithm.
        virtual StatusCode finalize() override;

    private:
        StatusCode validateClusters(const EventContext& eventContext, std::unordered_map<const xAOD::PixelCluster*, const xAOD::PixelCluster*>& pixel_cluster_matches, std::unordered_map<const xAOD::StripCluster*, const xAOD::StripCluster*>& strip_cluster_matches) const;
        StatusCode validatePixelSpacepoints(const EventContext& eventContext, std::unordered_map<const xAOD::PixelCluster*, const xAOD::PixelCluster*>& pixel_cluster_matches) const;

        void matchPixelClusters(
            std::vector<const xAOD::PixelCluster*>& monitored_list,
            std::vector<const xAOD::PixelCluster*>& reference_list,
            const std::string& module_id,
            std::vector<std::pair<const xAOD::PixelCluster*, const xAOD::PixelCluster*>>& pairs) const;
        
        void matchStripClusters(
            std::vector<const xAOD::StripCluster*>& monitored_list,
            std::vector<const xAOD::StripCluster*>& reference_list,
            const std::string& module_id,
            std::vector<std::pair<const xAOD::StripCluster*, const xAOD::StripCluster*>>& pairs) const;

        /// Lorentz shift stored in the traccc host conditions object for the given Athena module (side)
        std::optional<std::pair<float, float>> tracccLorentzShift(
            const Identifier& athenaId,
            const traccc::detector_conditions_description::host& cond) const;

        /// Centre of the traccc readout bin (channel0, channel1) on the given Athena module,
        /// computed like traccc::details::position_from_cell from the host design description
        std::optional<std::array<float, 2>> tracccCellPosition(
            const Identifier& athenaId, unsigned int channel0, unsigned int channel1,
            const traccc::detector_design_description::host& design,
            const traccc::detector_conditions_description::host& cond) const;

        Gaudi::Property<std::string> m_monDesignObjectName{
        this, "TracccDesignObjectName", "", "Candidate host design object in detStore"};    
        SG::ReadCondHandleKey<traccc::detector_conditions_description::host> m_monCondKey{
        this, "TracccCondKey", "DeviceDetectorDescriptionHostCond",
        "Key for reading the candidate host conditions object"};
        /// @name The detector description service providing the Athena<->Detray ID map
        Gaudi::Property<std::string> m_geoIdMappingObjectName{this, "GeoIdMapping", "", "ID mapping between the three detector description realms."};
        const ActsTrk::GeometryIdMapping* m_idMapping{nullptr};

        /// @name Boolean varibale turning on/off spacepoint validation
        /// {@
        Gaudi::Property<bool> m_checkSpacepoints{
            this, "checkSpacepoints", false,
            "If you also want to validate spacepounts."};
        /// @}    

        /// @name Names of input monitored and reference pixel/strip/spacepoint collections
        /// {@
        SG::ReadHandleKey<xAOD::SpacePointContainer> m_monitoredSpacepointsKey{
            this, "monitoredSpacepointsKey", "xAODSpacepointsFromTracccCluster",
            "Input monitored spacepoints"};

        SG::ReadHandleKey<xAOD::SpacePointContainer> m_referenceSpacepointsKey{
            this, "referenceSpacepointsKey", "ITkPixelSpacePoints",
            "Input reference spacepoints"};

        SG::ReadHandleKey<xAOD::PixelClusterContainer> m_monitoredPixelClustersKey{
            this, "monitoredPixelClustersKey", "xAODPixelClustersFromTracccCluster",
            "Input monitored pixel clusters"};

        SG::ReadHandleKey<xAOD::StripClusterContainer> m_monitoredStripClustersKey{
            this, "monitoredStripClustersKey", "xAODStripClustersFromTracccCluster",
            "Input monitored strip clusters"};

        SG::ReadHandleKey<xAOD::PixelClusterContainer> m_referencePixelClustersKey{
            this, "referencePixelClustersKey", "ITkPixelClusters",
            "Input reference pixel clusters"};

        SG::ReadHandleKey<xAOD::StripClusterContainer> m_referenceStripClustersKey{
            this, "referenceStripClustersKey", "ITkStripClusters",
            "Input reference strip clusters"};
        /// @}    


        // cluster sumamry
        mutable Gaudi::Accumulators::Counter<> m_pixel_unequal; 
        mutable Gaudi::Accumulators::Counter<> m_strip_unequal;
        mutable Gaudi::Accumulators::Counter<> m_matched_pixel; 
        mutable Gaudi::Accumulators::Counter<> m_matched_strip;
        mutable Gaudi::Accumulators::Counter<> m_pix_unmatched_mon;
        mutable Gaudi::Accumulators::Counter<> m_pix_unmatched_ref;
        mutable Gaudi::Accumulators::Counter<> m_strip_unmatched_mon;
        mutable Gaudi::Accumulators::Counter<> m_strip_unmatched_ref;
        mutable Gaudi::Accumulators::Counter<> m_pixel_pos_diff_1sig;
        mutable Gaudi::Accumulators::Counter<> m_pixel_pos_diff_0p5sig;
        mutable Gaudi::Accumulators::Counter<> m_pixel_pos_diff_0p25sig;
        mutable Gaudi::Accumulators::Counter<> m_strip_pos_diff_1sig;
        mutable Gaudi::Accumulators::Counter<> m_strip_pos_diff_0p5sig;
        mutable Gaudi::Accumulators::Counter<> m_strip_pos_diff_0p25sig;
        mutable Gaudi::Accumulators::Counter<> m_strip_pos_diff_barrel;
        mutable Gaudi::Accumulators::Counter<> m_strip_pos_diff_EC;

        // spacepoint summary
        mutable Gaudi::Accumulators::Counter<> m_nMonSp;
        mutable Gaudi::Accumulators::Counter<> m_nRefSp;
        mutable Gaudi::Accumulators::Counter<> m_nMatchedSp;
        mutable Gaudi::Accumulators::Counter<> m_nUnmatchedMonSp;
        mutable Gaudi::Accumulators::Counter<> m_nUnmatchedRefSp;
        mutable Gaudi::Accumulators::Counter<> m_nSpPosDiff1mm;
        mutable Gaudi::Accumulators::Counter<> m_nSpPosDiff5mm;
        mutable Gaudi::Accumulators::Counter<> m_nSpVarRDiff;
        mutable Gaudi::Accumulators::Counter<> m_nSpVarZDiff;

        Gaudi::Property<std::string> m_pixelManagerKey{
            this, "PixelManager", "ITkPixel"};
        Gaudi::Property<std::string> m_stripManagerKey{
            this, "StripManager", "ITkStrip"};
        const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
        const InDetDD::SCT_DetectorManager* m_stripManager{nullptr};
        const SCT_ID* m_stripID {nullptr};
        ToolHandle<ISiLorentzAngleTool> m_stripLorentzAngleTool{
            this, "StripLorentzAngleTool", "SiLorentzAngleTool",
            "Tool to retrieve Lorentz angle of Strip"};
        ToolHandle<ISiLorentzAngleTool> m_pixelLorentzAngleTool{
            this, "PixelLorentzAngleTool", "",
            "Tool to retreive Lorentz angle of Pixel"};
};

} // end of namespace

#endif
