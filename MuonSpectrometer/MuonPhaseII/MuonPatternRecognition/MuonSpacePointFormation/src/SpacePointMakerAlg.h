/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINTFORMATION_MUONSPACEPOINTMAKERALG_H
#define MUONSPACEPOINTFORMATION_MUONSPACEPOINTMAKERALG_H

#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonSpacePoint/SpacePoint.h"
#include "StoreGate/WriteHandleKey.h"



#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcMeasurementContainer.h"
#include "xAODMuonPrepData/TgcStripContainer.h"
#include "xAODMuonPrepData/MMClusterContainer.h"
#include "xAODMuonPrepData/sTgcMeasContainer.h"


namespace MuonR4{
    class SpacePointMakerAlg: public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm; 

            ~SpacePointMakerAlg() = default;

            StatusCode execute(const EventContext& ctx) const override;
            StatusCode initialize() override;
            StatusCode finalize() override;
        
        private:
            /** @brief Returns the transform from the attached Muon chamber frame to the sector frame
             * @param gctx: Geometry context to calculate the relative alignment between chamber & measurement
             * @param meas: Uncalibrated muon measurement */
            template<class MeasType> Amg::Transform3D toChamberTransform(const ActsGeometryContext& gctx, 
                                                                         const MeasType* meas) const;
            /** @brief Returns the position of the uncalibrated muon measurement in the sector frame
             * @param meas: Uncalibrated muon measurement
             * @param toChamberTrans: transform from the attached Muon chamber frame to the sector frame */
            template<class MeasType> Amg::Vector3D positionInChamber(const MeasType* meas,
                                                                     const Amg::Transform3D& toChamberTrans) const;
            /** @brief Returns the direction of the measurement channel in the sector frame
             * @param meas: Uncalibrated muon measurement
             * @param toChamberTrans: transform from the attached Muon chamber frame to the sector frame */
            template<class MeasType> Amg::Vector3D channelDirInChamber(const MeasType* meas,
                                                                       const Amg::Transform3D& toChamberTrans) const;
            /** @brief Returns the direction, in the sector frame, of the precision axis of the measurement, i.e. 
             * the vector pointing to the next strip or tube 
             * @param meas: Uncalibrated muon measurement
             * @param toChamberTrans: transform from the attached Muon chamber frame to the sector frame */
            template<class MeasType> Amg::Vector3D channelNormalInChamber(const MeasType* meas,
                                                                          const Amg::Transform3D& toChamberTrans) const;
            /** @brief Helper function to calculate the covariance for spacepoints having only one measurement 
             * @param primaryMeas: Uncalibrated muon measurement
             * @param dir: direction of the measurement channel in the sector frame
             * @param nor: direction of the precision axis of the measurement in the sector frame */
            template<class MeasType> AmgSymMatrix(2) computeCov(const MeasType* primaryMeas,
                                                                const Amg::Vector3D& dir,
                                                                const Amg::Vector3D& nor) const;
            /** @brief Helper function to calculate the covariance for spacepoints having two measurements
             * @param primaryMeas: Primary uncalibrated muon measurement
             * @param secondaryMeas: Secondary uncalibrated muon measurement
             * @param nor1: direction of the precision axis of the primary measurement in the sector frame
             * @param nor2: direction of the precision axis of the secondary measurement in the sector frame */                                                    
            AmgSymMatrix(2) computeCov(const xAOD::UncalibratedMeasurement* primaryMeas,
                                       const xAOD::UncalibratedMeasurement* secondaryMeas,
                                       const Amg::Vector3D& nor1,
                                       const Amg::Vector3D& nor2) const;
            /** @brief Helper function that creates the spacepoint and populates it, when having one measurement
             * @param pointColl: Collection of spacepoints where the new one is being push
             * @param primaryMeas: Uncalibrated muon measurement
             * @param toChamberTrans: transform from the attached Muon chamber frame to the sector frame */
            template <class MeasType> void fillSpacePoint (std::vector<SpacePoint>& pointColl,
                                                          const MeasType* primaryMeas,
                                                          const Amg::Transform3D& toChamberTrans) const;
            /** @brief Helper function that creates the spacepoint and populates it, when having two measurement
             * @param pointColl: Collection of spacepoints where the new one is being push
             * @param primaryMeas: Primary uncalibrated muon measurement
             * @param secondaryMeas: Secondary uncalibrated muon measurement
             * @param toChamberTrans_eta: transform from the attached Muon chamber frame to the sector frame for the primary meas
             * @param toChamberTrans_phi: transform from the attached Muon chamber frame to the sector frame for the secondary meas */
            template <class MeasType> void fillSpacePoint (std::vector<SpacePoint>& pointColl,
                                                           const MeasType* primaryMeas,
                                                           const MeasType* secondaryMeas,
                                                           const Amg::Transform3D& toChamberTrans_eta, 
                                                           const Amg::Transform3D& toChamberTrans_phi) const;
            
            /** @brief Helper class to keep track of how many eta+phi, eta and phi only space points are built
             *         in various detector regions. The SpacePointStatistics split the counts per muon station layer,
             *         i.e., BarrelInner, BarrelMiddle, EndCapInner, etc. are distinct categoriges. Each category
             *         is further subdivided into the indivudal stationEtas of the chambers and finally also into
             *         the technology type of the hit.
             */
            class SpacePointStatistics{
                public:
                    /** @brief Standard constructor
                     *  @param idHelperSvc: Pointer to the MuonIdHelperSvc needed to sort each hit into
                     *                      a counting category. */
                    SpacePointStatistics(const Muon::IMuonIdHelperSvc* idHelperSvc);
                
                    /** @brief Adds the vector of space points to the overall statistics. */
                    void addToStat(const std::vector<SpacePoint>& spacePoints);
                    /** @brief Print the statistics table of the built space points per category 
                     *         into the log-file / console */
                    void dumpStatisics(MsgStream& msg) const;

                private:
                    /** @brief Helper struct to count the space-points in each 
                     *          detector category. */
                    struct StatField{
                        /** @brief Number of space points measuring eta & phi */
                        unsigned int measEtaPhi{0};
                        /** @brief Number of space points measuring eta only */
                        unsigned int measEta{0};
                        /** @brief Number of space points measuring phi only*/
                        unsigned int measPhi{0};
                        /** @brief Helper method returning the sum of the three
                         *         space point type counts */
                        unsigned int allHits() const;
                    };
                    /** @brief Helper struct to define the counting categories. */
                    struct FieldKey{
                        using StIdx_t = Muon::MuonStationIndex::StIndex;
                        using TechIdx_t = Muon::MuonStationIndex::TechnologyIndex; 
                        StIdx_t stIdx{StIdx_t::StUnknown};
                        TechIdx_t techIdx{TechIdx_t::TechnologyUnknown};
                        int eta{0};
                        bool operator<(const FieldKey& other) const;
                    };

                    const Muon::IMuonIdHelperSvc* m_idHelperSvc{};
                    std::mutex m_mutex{};
                    using StatMap_t = std::map<FieldKey, StatField>;
                    StatMap_t m_map{};
            };
            /** @brief: Helper struct to collect the space point per muon chamber, which are 
             *          later sorted into the space point buckets. */
            struct SpacePointsPerChamber{
                /** @brief Vector of all hits that contain an eta measurement including the 
                 *         ones which are combined with phi measurements */
                std::vector<SpacePoint> etaHits{};
                /** @brief Vector of all space points that are built from single phi hits */
                std::vector<SpacePoint> phiHits{};                
            };
            /** @brief Container abrivation of the presorted space point container per MuonChambers */
            using PreSortedSpacePointMap = std::unordered_map<const MuonGMR4::SpectrometerSector*, SpacePointsPerChamber>;

  
            /** @brief Retrieve an uncalibrated measurement container <ContType> and fill the hits into the
             *         presorted space point map. Per associated MuonChamber, hits from Tgc, Rpc, sTgcs are 
             *         grouped by their gasGap location and then divided into eta & phi measurements. If both
             *         are found, each eta measurement is combined with phi measurement into a SpacePoint. 
             *         In any other case, the measurements are just transformed into a SpacePoint.
             *  @param ctx: Event context of the current event
             *  @param key: ReadHandleKey to access the container of data type <ContType>
             *  @param fillContainer: Global container into which all space points are filled.
             */
            template <class ContType> 
                StatusCode loadContainerAndSort(const EventContext& ctx,
                                                const SG::ReadHandleKey<ContType>& key,
                                                PreSortedSpacePointMap& fillContainer) const;
                                                    
            /** @brief Abrivation of a MuonSapcePoint bucket vector */
            using SpacePointBucketVec = std::vector<SpacePointBucket>;

            /** @brief Distribute the premade spacepoints per chamber into their individual SpacePoint
             *         buckets. A new bucket is created everytime if the hit to fill is along the z-axis 
             *         farther away from the first point in the bucket than the <spacePointWindowSize>.
             *         Hit in the previous bucket which are <spacePointOverlap> away from the first hit
             *         in the new bucket are also mirrored. The bucket formation starts with the eta
             *         Muon space points and then consumes the phi hits.
             * @param ctx: Event context of the current event
             * @param hitsPerChamber: List of all premade space points which have to be sorted
             * @param finalContainer: Output SpacePoint bucket container.
             *  */
            void distributePointsAndStore(
                                          SpacePointsPerChamber&& hitsPerChamber,
                                          SpacePointContainer& finalContainer) const;

            void distributePointsAndStore(
                                          std::vector<SpacePoint>&& spacePoints,
                                          SpacePointBucketVec& splittedContainer) const;

            /** @brief: Check whether the occupancy cuts of hits in a gasGap are surpassed.
             *          The method is specified for each of the 3 strip technologies, 
             *          Rpc, Tgc, sTgc and applies a technology-dependent upper bound on the 
             *          number of phi & eta hits. If the threshold is surpassed, only 1D space
             *          points are built intsead of 2D ones
             * @param etaHits: List of all presorted eta measurements in a gas gap
             * @param phiHits: List of all presorted phi measurements in a gas gap  
             */
            template <class PrdType>
                bool passOccupancy2D(const std::vector<const PrdType*>& etaHits,
                                     const std::vector<const PrdType*>& phiHits) const;
            
            SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_mdtKey{this, "MdtKey", "xMdtMeasurements",
                                                                      "Key to the uncalibrated Drift circle measurements"};
            
            SG::ReadHandleKey<xAOD::RpcMeasurementContainer> m_rpcKey{this, "RpcKey", "xRpcMeasurements",
                                                                "Key to the uncalibrated 1D rpc hits"};
            
            SG::ReadHandleKey<xAOD::TgcStripContainer> m_tgcKey{this, "TgcKey", "xTgcStrips",
                                                                "Key to the uncalibrated 1D tgc hits"};

            SG::ReadHandleKey<xAOD::MMClusterContainer> m_mmKey{this, "MmKey", "xAODMMClusters",
                                                                "Key to the uncalibrated 1D Mm hits"};

            SG::ReadHandleKey<xAOD::sTgcMeasContainer> m_stgcKey{this, "sTgcKey", "xAODsTgcMeasurements"};


            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            
            SG::WriteHandleKey<SpacePointContainer> m_writeKey{this, "WriteKey", "MuonSpacePoints"};

            Gaudi::Property<double> m_spacePointWindow{this, "spacePointWindowSize", 0.8*Gaudi::Units::m,
                                                       "Maximal distance between consecutive hits in a bucket"};

            Gaudi::Property<double> m_maxBucketLength{this, "maxBucketLength", 2.*Gaudi::Units::m,
                                                       "Maximal size of a space point bucket"};
            
            Gaudi::Property<double> m_spacePointOverlap{this, "spacePointOverlap", 25.*Gaudi::Units::cm,
                                                        "Hits that are within <spacePointOverlap> of the bucket margin. "
                                                        "Are copied to the next bucket"};
    
            Gaudi::Property<bool> m_doStat{this, "doStats", true, 
                                           "If enabled the algorithm keeps track how many hits have been made" };
            
            Gaudi::Property<unsigned int> m_capacityBucket{this,"CapacityBucket" , 50};
            std::unique_ptr<SpacePointStatistics> m_statCounter ATLAS_THREAD_SAFE{};

            Gaudi::Property<double> m_maxOccRpcEta{this, "maxRpcEtaOccupancy", 0.1, 
                                                   "Maximum occpancy of Rpc eta hits in a gasGap"};
            Gaudi::Property<double> m_maxOccRpcPhi{this, "maxRpcPhiOccupancy", 0.1, 
                                                   "Maximum occpancy of Rpc phi hits in a gasGap"};

            Gaudi::Property<double> m_maxOccTgcEta{this, "maxTgcEtaOccupancy", 0.1, 
                                                   "Maximum occpancy of Tgc eta hits in a gasGap"};
            Gaudi::Property<double> m_maxOccTgcPhi{this, "maxTgcPhiOccupancy", 0.1, 
                                                   "Maximum occpancy of Tgc phi hits in a gasGap"};

            Gaudi::Property<double> m_maxOccStgcEta{this, "maxSTGCEtaOccupancy", 0.1, 
                                                   "Maximum occpancy of sTgc eta hits in a gasGap"};
            Gaudi::Property<double> m_maxOccStgcPhi{this, "maxSTGCPhiOccupancy", 0.1, 
                                                    "Maximum occpancy of sTgc phi hits in a gasGap"};

    };
}


#endif
