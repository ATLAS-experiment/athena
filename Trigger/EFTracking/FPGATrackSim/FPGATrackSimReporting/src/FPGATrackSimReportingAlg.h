/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATRACKSIMREPORTING_FPGATRACKSIMREPORTINGALG
#define FPGATRACKSIMREPORTING_FPGATRACKSIMREPORTINGALG


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/StoreGateSvc.h"

#include "FPGATrackSimObjects/FPGATrackSimClusterCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimRoadCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"
#include "ActsEvent/ProtoTrackCollection.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackParametersContainer.h"

#include "src/FPGATrackSimActsTrackInspectionTool.h"

class FPGATrackSimTrack;

namespace FPGATrackSim {
        class FPGATrackSimReportingAlg : public ::AthReentrantAlgorithm {
        public:
                FPGATrackSimReportingAlg(const std::string& name, ISvcLocator* pSvcLocator);
                virtual ~FPGATrackSimReportingAlg() = default;

                virtual StatusCode initialize() override final;     //once, before any input is loaded
                virtual StatusCode execute(const EventContext& ctx) const override final;
                virtual StatusCode finalize() override final;

        private:
                mutable std::vector<uint32_t> m_pixelClustersPerFPGATrack ATLAS_THREAD_SAFE, m_stripClustersPerFPGATrack ATLAS_THREAD_SAFE;
                mutable std::vector<uint32_t> m_pixelClustersPerPrototrack ATLAS_THREAD_SAFE, m_stripClustersPerPrototrack ATLAS_THREAD_SAFE;
                
                mutable std::map<std::string, std::vector<FPGATrackSimActsEventTracks> > m_allActsTracks ATLAS_THREAD_SAFE;
                mutable std::map<std::string, std::map<uint32_t,std::vector<uint32_t>> > m_actsTrackStats ATLAS_THREAD_SAFE;

                Gaudi::Property<bool> m_printoutForEveryEvent {this, "perEventReports", false, "A flag to enable per event printout"};
   	        Gaudi::Property<bool> m_isDataPrep {this, "isDataPrep", false, "If True, this is for data prep pipeline only"};
                //_________________________________________________________________________________________________________________________
                // xAOD Pixel clusters to monitor
                SG::ReadHandleKeyArray <xAOD::PixelClusterContainer> m_xAODPixelClusterContainerKeys{
                        this, "xAODPixelClusterContainers", {},
                        "input list of xAOD Pixel Cluster Containers, as resulted from FPGATrackSim (hit/road) EDM conversion" };
                
                // xAOD Strip clusters to monitor
                SG::ReadHandleKeyArray <xAOD::StripClusterContainer> m_xAODStripClusterContainerKeys{
                        this, "xAODStripClusterContainers", {"ITkStripClusters" ,"xAODStripClusters_1stFromFPGACluster", "xAODStripClusters_1stFromFPGAHit"},
                        "input list of xAOD Strip Cluster Containers, as resulted from FPGATrackSim (hit/road) EDM conversion" };
                
                // xAOD SpacePoints to monitor
                SG::ReadHandleKeyArray <xAOD::SpacePointContainer> m_xAODSpacePointContainerKeys{
		  this, "xAODSpacePointContainersFromFPGA", {"xAODStripSpacePoints_1stFromFPGA","xAODPixelSpacePoints_1stFromFPGA"},
                        "input list of xAOD SpacePoint Containers, as resulted from FPGATrackSim (hit/road) EDM conversion" };
                // FPGA Cluster collection
                // To be implemented
                
                // FPGA Road collection
                SG::ReadHandleKey <FPGATrackSimRoadCollection> m_FPGARoadsKey{ this, "FPGATrackSimRoads","","FPGATrackSim Roads key" };
                // FPGA Track collection
                SG::ReadHandleKey <FPGATrackSimTrackCollection> m_FPGATracksKey{ this, "FPGATrackSimTracks","","FPGATrackSim Tracks key" };
                
                SG::ReadHandleKeyArray<ActsTrk::ProtoTrackCollection> m_FPGAProtoTrackCollections{this, "FPGATrackSimProtoTracks",{},"FPGATrackSim PrototrackCollection"};

                SG::ReadHandleKeyArray<ActsTrk::TrackContainer> m_ActsTrackCollections{this, "FPGAActsTracks",{},"Acts track collections from (C)KF"};

                SG::ReadHandleKeyArray<ActsTrk::SeedContainer> m_ActsSeedCollections{this, "FPGAActsSeeds",{},"Acts Seeds collections from (C)KF"};

                SG::ReadHandleKeyArray<ActsTrk::BoundTrackParametersContainer> m_ActsSeedParamCollections{this, "FPGAActsSeedsParam",{},"Acts Seeds param collections from (C)KF"};


                //_________________________________________________________________________________________________________________________              
                // Tools
                ToolHandle<FPGATrackSim::ActsTrackInspectionTool> m_ActsInspectionTool {this, "ActsInspectionTool", "FPGATrackSim::ActsTrackInspectionTool/ActsTrackInspectionTool", "Monitoring tool for acts tracks"};

                //_________________________________________________________________________________________________________________________              
                template <class XAOD_CLUSTER>
                void processxAODClusters(SG::ReadHandle<DataVector< XAOD_CLUSTER >>& clusterContainer) const;
                template <class XAOD_CLUSTER>
                void printxAODClusters(SG::ReadHandle<DataVector< XAOD_CLUSTER >>& clusterContainer) const;

                void processxAODSpacePoints(SG::ReadHandle<DataVector< xAOD::SpacePoint >>& spContainer) const;
                void printxAODSpacePoints(SG::ReadHandle<DataVector< xAOD::SpacePoint >>& spContainer) const;

                void processFPGAClusters(SG::ReadHandle<FPGATrackSimClusterCollection> &FPGAClusters) const;
                void printFPGAClusters(SG::ReadHandle<FPGATrackSimClusterCollection> &FPGAClusters) const;

                void processFPGARoads(SG::ReadHandle<FPGATrackSimRoadCollection> &FPGARoads) const;
                void printFPGARoads(SG::ReadHandle<FPGATrackSimRoadCollection> &FPGARoads) const;

                void processFPGATracks(SG::ReadHandle<FPGATrackSimTrackCollection> &FPGATracks) const;
                void printFPGATracks(SG::ReadHandle<FPGATrackSimTrackCollection> &FPGATracks) const;
                
                void processFPGASeeds(SG::ReadHandle<ActsTrk::SeedContainer> &FPGASeeds) const;
                void printFPGASeeds(SG::ReadHandle<ActsTrk::SeedContainer> &FPGASeeds) const;

                void processFPGASeedsParam(SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> &FPGASeedsParam) const;
                void printFPGASeedsParam(SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> &FPGASeedsParam) const;

                void processFPGAPrototracks(SG::ReadHandle<ActsTrk::ProtoTrackCollection> &FPGAPrototracks) const;
                void printFPGAPrototracks(SG::ReadHandle<ActsTrk::ProtoTrackCollection> &FPGAPrototracks) const;
                
                void processActsTracks(SG::ReadHandle<ActsTrk::TrackContainer> &ActsTracks) const;

                StatusCode summaryReportForEDMConversion();
        };

}

#endif //> !FPGATRACKSIMREPORTING_FPGATRACKSIMREPORTINGALG
