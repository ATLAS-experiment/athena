/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATRACKSIMREPORTING_FPGATRACKSIMREPORTINGALG
#define FPGATRACKSIMREPORTING_FPGATRACKSIMREPORTINGALG


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/StoreGateSvc.h"

#include "FPGATrackSimObjects/FPGATrackSimClusterCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimRoadCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"
#include "ActsEvent/ProtoTrackCollection.h"

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
                
                Gaudi::Property<bool> m_printoutForEveryEvent {this, "perEventReports", false, "A flag to enable per event printout"};
   	        Gaudi::Property<bool> m_isDataPrep {this, "isDataPrep", false, "If True, this is for data prep pipeline only"};
                //_________________________________________________________________________________________________________________________
                // xAOD Pixel clusters to monitor
                SG::ReadHandleKeyArray <xAOD::PixelClusterContainer> m_xAODPixelClusterContainerKeys{
                        this, "xAODPixelClusterContainersFromFPGA", {},
                        "input list of xAOD Pixel Cluster Containers, as resulted from FPGATrackSim (hit/road) EDM conversion" };
                
                // xAOD Strip clusters to monitor
                SG::ReadHandleKeyArray <xAOD::StripClusterContainer> m_xAODStripClusterContainerKeys{
		  this, "xAODStripClusterContainersFromFPGA", {"ITkStripClusters" ,"xAODStripClusters_1stFromFPGACluster", "xAODStripClusters_1stFromFPGAHit", "xAODStripClusters_1stFromSP"},
                        "input list of xAOD Strip Cluster Containers, as resulted from FPGATrackSim (hit/road) EDM conversion" };
                
                // FPGA Cluster collection
                // FPGA Road collection
                SG::ReadHandleKey <FPGATrackSimRoadCollection> m_FPGARoadsKey{ this, "FPGATrackSimRoads","","FPGATrackSim Roads key" };
                // FPGA Track collection
                SG::ReadHandleKey <FPGATrackSimTrackCollection> m_FPGATracksKey{ this, "FPGATrackSimTracks","","FPGATrackSim Tracks key" };
                

                
                SG::ReadHandleKey<ActsTrk::ProtoTrackCollection> m_FPGAProtoTrackCollection{this, "FPGATrackSimProtoTracks","","FPGATrackSim PrototrackCollection"};

                //_________________________________________________________________________________________________________________________              
                template <class XAOD_CLUSTER>
                void processxAODClusters(SG::ReadHandle<DataVector< XAOD_CLUSTER >>& clusterContainer) const;
                template <class XAOD_CLUSTER>
                void printxAODClusters(SG::ReadHandle<DataVector< XAOD_CLUSTER >>& clusterContainer) const;

                void processFPGAClusters(SG::ReadHandle<FPGATrackSimClusterCollection> &FPGAClusters) const;
                void printFPGAClusters(SG::ReadHandle<FPGATrackSimClusterCollection> &FPGAClusters) const;

                void processFPGARoads(SG::ReadHandle<FPGATrackSimRoadCollection> &FPGARoads) const;
                void printFPGARoads(SG::ReadHandle<FPGATrackSimRoadCollection> &FPGARoads) const;

                void processFPGATracks(SG::ReadHandle<FPGATrackSimTrackCollection> &FPGATracks) const;
                void printFPGATracks(SG::ReadHandle<FPGATrackSimTrackCollection> &FPGATracks) const;
                
                void processFPGAPrototracks(SG::ReadHandle<ActsTrk::ProtoTrackCollection> &FPGAPrototracks) const;
                void printFPGAPrototracks(SG::ReadHandle<ActsTrk::ProtoTrackCollection> &FPGAPrototracks) const;

                

                StatusCode summaryReportForEDMConversion();
        };

}

#endif //> !FPGATRACKSIMREPORTING_FPGATRACKSIMREPORTINGALG
