/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TritonTracccTrackMaker_H
#define TritonTracccTrackMaker_H

// System include(s).
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

// Athena includes
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/PersistentTrackContainer.h"
#include "ActsEvent/ContextUtility.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "ActsCalibBase/MeasurementCalibratorBase.h"
#include "Acts/EventData/BoundTrackParameters.hpp"

#include "TrkEventPrimitives/PdgToParticleHypothesis.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetReadoutGeometry/SiDetectorDesign.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePoint.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "xAODTruth/TruthVertex.h"

// Tool handles
#include "ITracccTritonTool.h"

/**
 * @class TritonTracccTrackMaker
 *
 * @brief TritonTracccTrackMaker is an algorithm that uses the Traccc-algorithm
 * to reconstruct tracks from cells. Cells are sent to a Traccc backend using Triton
 * and tracks are sent back. 
 *
 * @author miles.cb@cern.ch
 */
class TritonTracccTrackMaker : public AthReentrantAlgorithm {

public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

protected:
    /// --------------------
    /// @name Input helpers
    /// --------------------
    //@{

    // Input helpers
    SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey{this, "PixelRDO",
                                                        "ITkPixelRDOs"};
    SG::ReadHandleKey<SCT_RDO_Container> m_stripRDOKey{this, "StripRDO",
                                                       "ITkStripRDOs"};

    const PixelID* m_pixelID{nullptr};
    const SCT_ID* m_stripID{nullptr};
    const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
    const InDetDD::SCT_DetectorManager* m_stripManager{nullptr};

    // Read input data to hits for Traccc use!
    std::vector<int> map_index(int index,
                               int low_bound,
                               int high_bound,
                               int threshold,
                               int shift) const;

    std::vector<std::pair<int, int>> correct_indices(
            int phiIndex, int etaIndex, int rows,
            int columns) const;

    StatusCode read_cells(std::vector<TracccCell>& cells,
                        const EventContext& evtcontext) const;


    std::unordered_map<int64_t, int> readAndConvertClusters(
        const EventContext& eventContext) const;

    StatusCode convertInDetToXaodCluster(
        const InDet::PixelCluster& indetCluster,
        const InDetDD::SiDetectorElement& element, xAOD::PixelCluster& xaodCluster) const;

    StatusCode convertInDetToXaodCluster(
        const InDet::SCT_Cluster& indetCluster,
        const InDetDD::SiDetectorElement& element, xAOD::StripCluster& xaodCluster) const;
    //@}

    /// --------------------
    /// @name Output helpers
    /// --------------------
    //@{
    // output container
    // TODO: this should return type PersistentTrackContainer
    // The tool ActsTrackToTrackParticleCnvAlgCfg needs to be appropriately updated
    SG::WriteHandleKey<ActsTrk::TrackContainer> m_ActsTracccTrackContainerKey{
        this, "TracccTracks", "TracccTracks",
        "Output track collection (ActsTrk variant)"};

    // acts helper for the output
    ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper{
        this};
    Trk::PdgToParticleHypothesis m_pdgToParticleHypothesis;

    std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;
    const ActsTrk::DetectorElementToActsGeometryIdMap* m_detEleToGeoIdMap{nullptr};

    const Acts::Surface* actsSurfaceFromAtlasId(const Identifier& atlasID) const;

    Acts::BoundMatrix buildBoundCovariance(
        const LocalMeasurementInfoInTracks& state) const;

    std::optional<Acts::BoundTrackParameters> convertToActsParameters(
        const LocalMeasurementInfoInTracks& state) const;

    StatusCode convertTracks(
        EventContext const& eventContext,
        std::vector<TracccTrackParameters>& trackParams,
        std::vector<LocalMeasurementInfoInTracks>& measInfo,
        const std::unordered_map<int64_t, int>& cluster_map,
        unsigned& nb_output_tracks) const;

    //@}

    /// --------------------
    /// @name Tool handles
    /// --------------------
    //@{
    ToolHandle<ITracccTritonTool> m_tracccTrackingTool{
        this, "TracccTritonTool", "TracccTritonTool"};

    ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{
        this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
    // @}

    /** @brief Utility to fetch the geometry, magnetic field and calibration context in the event */
    ActsTrk::ContextUtility m_ctxProvider{this};

    StringProperty m_featureNames{this, "FeatureNames",
                                "geometry_id,measurement_id,channel0,channel1,timestamp,value"};
    std::vector<std::string> m_featureNamesVec;


    /// Truth association for plotting and debugging
    Gaudi::Property<bool> m_doTruth{
        this, "doTruth", true,
        "Create output containers and link to truth"};

    SG::ReadHandleKey<ActsTrk::MeasurementToTruthParticleAssociation>
        m_pixelClustersToTruth{
            this, "PixelClustersToTruthParticlesMap",
            "PixelClustersForTruthSeedingToTruthParticlesMap",
            "Association map from pixel measurements to generator particles."};

    SG::ReadHandleKey<ActsTrk::MeasurementToTruthParticleAssociation>
        m_stripClustersToTruth{
            this, "StripClustersToTruthParticlesMap",
            "StripClustersForTruthSeedingToTruthParticlesMap",
            "Association map from strip measurements to generator particles."};

    SG::ReadHandleKey<InDet::PixelClusterContainer>
        m_inputPixelClusterContainerKey{
            this, "InputPixelClustersName", "ITkPixelClusters",
            "name of the input InDet pixel cluster container"};
    SG::ReadHandleKey<InDet::SCT_ClusterContainer>
        m_inputStripClusterContainerKey{
            this, "InputStripClustersName", "ITkStripClusters",
            "name of the input InDet strip cluster container"};

    SG::WriteHandleKey<xAOD::PixelClusterContainer>
        m_xAODPixelClusterFromInDetClusterKey{
            this, "xAODPixelClusterFromInDetClusterKey",
            "xAODPixelClustersFromInDetCluster",
            "InDet cluster->xAOD PixelClusters Container"};
    SG::WriteHandleKey<xAOD::StripClusterContainer>
        m_xAODStripClusterFromInDetClusterKey{
            this, "xAODStripClusterFromInDetClusterKey",
            "xAODStripClustersFromInDetCluster",
            "InDet cluster ->xAOD StripClusters Container"};
    SG::WriteHandleKey<xAOD::SpacePointContainer>
        m_xAODSpacepointFromInDetClusterKey{
            this, "xAODSpacepointFromInDetClusterKey",
            "xAODSpacepointFromInDetCluster",
            "InDet cluster->xAOD Spacepoint Container"};

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection>
        m_pixelDetEleCollKey{this, "PixelDetEleCollKey",
                             "ITkPixelDetectorElementCollection",
                             "Key of SiDetectorElementCollection for Pixel"};
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection>
        m_stripDetEleCollKey{this, "StripDetEleCollKey",
                             "ITkStripDetectorElementCollection",
                             "Key of SiDetectorElementCollection for Strip"};


}; // TritonTracccTrackMaker

#endif
