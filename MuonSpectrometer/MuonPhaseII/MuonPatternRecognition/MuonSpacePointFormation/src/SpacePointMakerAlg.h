/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINTFORMATION_MUONSPACEPOINTMAKERALG_H
#define MUONSPACEPOINTFORMATION_MUONSPACEPOINTMAKERALG_H

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonSpacePoint/SpacePoint.h"
#include "StoreGate/WriteHandleKey.h"
#include "Acts/Utilities/PointerTraits.hpp"



#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcMeasurementContainer.h"
#include "xAODMuonPrepData/TgcStripContainer.h"
#include "xAODMuonPrepData/MMClusterContainer.h"
#include "xAODMuonPrepData/sTgcMeasContainer.h"
#include <xAODMuonViews/ChamberViewer.h>


namespace MuonR4{
    /** @brief Data preparation algorithm that transforms the uncalibrated measurements into muon space points. Mdt, Mm measurements
     *         are directly transformed. The remaining three technologies provide eta & phi measurements, each 1D. The measurements are
     *         sorted by gas gap and if the occupancy in the gas gap is low enough, then each eta measurement is combined with each phi 
     *         measurement to a 2D space point. Otherwise, single 1D space points are produced. Space points in the same MS layer 
     *         & phi-sector are expressed in the common sector frame. */
    class SpacePointMakerAlg: public AthReentrantAlgorithm {
        public:
            template <Acts::PointerConcept Prd_t>
            using PrdVec_t = std::vector<Prd_t>;
            template <typename T>
            using EtaPhi2DHits = std::array<PrdVec_t<T>, 3>;
            template <typename T>
            using EtaPhi2DHitsVec = std::vector<EtaPhi2DHits<T>>;

            using AthReentrantAlgorithm::AthReentrantAlgorithm; 
            ~SpacePointMakerAlg() = default;

            StatusCode execute(const EventContext& ctx) const override;
            StatusCode initialize() override;
            StatusCode finalize() override;
        
        private:
            /**  @brief Helper struct to define the bucket parameters for a given chamber. The bucket parameters are used to define the size of the buckets in which the space points are sorted. */
            struct BucketParameters {
                double spacePointWindow{0.8 * Gaudi::Units::m};
                double maxBucketLength{2.0 * Gaudi::Units::m};
                double spacePointOverlap{0.25 * Gaudi::Units::m};
            };

            /** @brief Helper struct to store the resolved bucket parameters for a given chamber. */
            struct ResolvedParameter {
                double value{0.};
                bool matched{false};
            };

            /** @brief Helper class to keep track of how many eta+phi, eta and phi only space points are built
             *         in various detector regions. The SpacePointStatistics split the counts per muon station layer,
             *         i.e., BarrelInner, BarrelMiddle, EndCapInner, etc. are distinct categoriges. Each category
             *         is further subdivided into the indivudal stationEtas of the chambers and finally also into
             *         the technology type of the hit. */
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
                        unsigned measEtaPhi{0};
                        /** @brief Number of space points measuring eta only */
                        unsigned measEta{0};
                        /** @brief Number of space points measuring phi only*/
                        unsigned measPhi{0};
                        /** @brief Helper method returning the sum of the three
                         *         space point type counts */
                        unsigned allHits() const;
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
            
            /** @brief Abrivation of a MuonSapcePoint bucket vector */
            using SpacePointBucketVec = std::vector<SpacePointBucket>;
            /** @brief Retrieve an uncalibrated measurement container <ContType> and fill the hits into the
             *         presorted space point map. Per associated MuonChamber, hits from Tgc, Rpc, sTgcs are 
             *         grouped by their gasGap location and then divided into eta & phi measurements. If both
             *         are found, each eta measurement is combined with phi measurement into a SpacePoint. 
             *         In any other case, the measurements are just transformed into a SpacePoint.
             *  @param ctx: Event context of the current event
             *  @param key: ReadHandleKey to access the container of data type <ContType>
             *  @param fillContainer: Global container into which all space points are filled. */
            template <typename ContType> 
                StatusCode loadContainerAndSort(const EventContext& ctx,
                                                const SG::ReadHandleKey<ContType>& key,
                                                PreSortedSpacePointMap& fillContainer) const;
            /** @brief: Check whether the occupancy cuts of hits in a gasGap are surpassed.
             *          The method is specified for each of the 3 strip technologies, 
             *          Rpc, Tgc, sTgc and applies a technology-dependent upper bound on the 
             *          number of phi & eta hits. If the threshold is surpassed, only 1D space
             *          points are built intsead of 2D ones
             * @param etaHits: List of all presorted eta measurements in a gas gap
             * @param phiHits: List of all presorted phi measurements in a gas gap */
            template <typename PrdType>
                bool passOccupancy2D(const PrdVec_t<PrdType>& etaHits,
                                     const PrdVec_t<PrdType>& phiHits) const;
            /** @brief Splits the chamber hits of the viewer per gas gap
             *  @param viewer: Chamber viewer containing all hits in the chamber
             *  @return Vector of gas gap hit collections. Each entry contains 3 vectors:
             *          - eta hits
             *          - phi hits
             *          - 2D hits */
            template <typename ContType>
                EtaPhi2DHitsVec<typename ContType::const_value_type> splitHitsPerGasGap(xAOD::ChamberViewer<ContType>& viewer) const;
            /** @brief Transform the uncombined space prd measurements to space points
             *  @param gctx: Geometry context to fetch the transformation of the measurements
             *  @param sectorTrans: Transformation to go from the global -> sector frame
             *  @param prdsToFill: List of uncombined measurements to transform
             *  @param outColl: Reference to the mutable output collection to which the 
             *                  1D space points are appended. */
            template <typename PrdType> 
                void fillUncombinedSpacePoints(const ActsTrk::GeometryContext& gctx,
                                               const Acts::Transform3& sectorTrans,
                                               const PrdVec_t<const PrdType*>& prdsToFill,
                                               std::vector<SpacePoint>& outColl) const; 
            /** @brief Distribute the premade spacepoints per chamber into their individual SpacePoint
             *         buckets. A new bucket is created everytime if the hit to fill is along the z-axis 
             *         farther away from the first point in the bucket than the <spacePointWindowSize>.
             *         Hit in the previous bucket which are <spacePointOverlap> away from the first hit
             *         in the new bucket are also mirrored. The bucket formation starts with the eta
             *         Muon space points and then consumes the phi hits.
             * @param ctx: Event context of the current event
             * @param hitsPerChamber: List of all premade space points which have to be sorted
             * @param finalContainer: Output SpacePoint bucket container. */
            void distributePointsAndStore(SpacePointsPerChamber&& hitsPerChamber,
                                          SpacePointContainer& finalContainer) const;
            /** @brief Distributes the vector of primary eta or eta + phi space points and fills them into the
             *         buckets. The buckets are dynamically created based on the distance of the new space point
             *         to sort to the previous or the first space point in the bucket.
             *  @param spacePoints: Vector of space points to sort into the buckets
             *  @param splittedContainer: Output vector containing all defined bucket */
            void distributePrimaryPoints(std::vector<SpacePoint>&& spacePoints,
                                         SpacePointBucketVec& splittedContainer) const;
            /** @brief Distributs the vector phi space points into the buckets. In contrast to the primary distribution
             *         no new buckets are created and the points are distributed into the existing ones instead.
             *  @param spacePoint: Vecotr of phi space points to sort into the buckets
             *  @param splittedContainer: Output vector containing all defined bucket */
            void distributePhiPoints(std::vector<SpacePoint>&& spacePoints,
                                     SpacePointBucketVec& splittedContainer) const;

            /** @brief Returns whether the space point is beyond the bucket boundary.
             *  @param spacePoint: Space point candidate to add to the bucket
             *  @param sortedPoints: Container of all defined buckets in the chamber
             *  @param bucketParams: Parameters defining the bucket properties
             *  @return True if the space point should start a new bucket, false otherwise */
            bool splitBucket(const SpacePoint& spacePoint,
                             const double firstSpPos,
                             const SpacePointBucketVec& sortedPoints,
                             const BucketParameters& bucketParams) const;
            /** @brief Closes the current processed bucket and creates a new one. Space points of the previous bucket
             *         within the overlap region to the first space point of the new bucket are copied over
             * @param refSp: First new space point which will be added to the new bucket.
             * @param sortedPoints: List of all processed buckets in the chamber. The list is augmented by 1 element 
             * @param bucketParams: Parameters defining the bucket properties
             */
            void newBucket(const SpacePoint& refSp,
                           SpacePointBucketVec& sortedPoints,
                           const BucketParameters& bucketParams) const;

            using BucketPatternMap = std::map<std::string, double, std::less<>>;

            /** @brief Returns a string key for a chamber, based on the space point identifier.
             *  @param chamber: Chamber for which to generate the key
             *  @return A string representing the chamber key (e.g. EML_eta-1_phi3) */
            std::string chamberConfigKey(const MuonGMR4::Chamber& chamber) const;

            /** @brief Resolves the bucket parameters for a given chamber, based on the chamber key and the configured patterns.
             *  @param chamber: Chamber for which to resolve the bucket parameters
             *  @return A BucketParameters struct containing the resolved parameters for the chamber */
            std::optional<BucketParameters> resolveBucketParameters(const MuonGMR4::Chamber& chamber) const;
            /** @brief Resolves a specific parameter for a given chamber key, based on the configured patterns.
             *  @param chamberKey: Key (e.g. EML_eta-1_phi3) of the chamber
             *  @param defaultValue: Default value to use if no match is found
             *  @param patterns: Map of patterns to values
             *  @return The resolved parameter value */
            ResolvedParameter resolveParameter( const std::string_view chamberKey, const double defaultValue, const BucketPatternMap& patterns) const; 
            /** @brief Returns the bucket parameters for a given space point.
             *  @param spacePoint: Space point for which to get the bucket parameters
             *  @return A reference to the BucketParameters struct for the space point */
            const BucketParameters& getBucketParameters(const SpacePoint& spacePoint) const;

            SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_mdtKey{this, "MdtKey", "xMdtMeasurements",
                                                                      "Key to the uncalibrated Drift circle measurements"};
            
            SG::ReadHandleKey<xAOD::RpcMeasurementContainer> m_rpcKey{this, "RpcKey", "xRpcMeasurements",
                                                                "Key to the uncalibrated 1D rpc hits"};
            
            SG::ReadHandleKey<xAOD::TgcStripContainer> m_tgcKey{this, "TgcKey", "xTgcStrips",
                                                                "Key to the uncalibrated 1D tgc hits"};

            SG::ReadHandleKey<xAOD::MMClusterContainer> m_mmKey{this, "MmKey", "xAODMMClusters",
                                                                "Key to the uncalibrated 1D Mm hits"};

            SG::ReadHandleKey<xAOD::sTgcMeasContainer> m_stgcKey{this, "sTgcKey", "xAODsTgcMeasurements"};


            ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            
            SG::WriteHandleKey<SpacePointContainer> m_writeKey{this, "WriteKey", "MuonSpacePoints"};

            /** @brief Default space point window size (Max distance between the two eta hits). Can be overridden by chamber-specific patterns 
             * @param spacePointWindowSize: Default space point window size in meters */
            Gaudi::Property<double> m_spacePointWindow{this, "spacePointWindowSize", 0.8*Gaudi::Units::m,
                                                       "Maximal distance between consecutive hits in a bucket"};
            /** @brief Default maximum bucket length (the width of the bucket in local y coordinate). Can be overridden by chamber-specific patterns
             * @param maxBucketLength: Default maximum bucket length in meters */
            Gaudi::Property<double> m_maxBucketLength{this, "maxBucketLength", 2.*Gaudi::Units::m,
                                                       "Maximal size of a space point bucket"};
            
            /** @brief Default space point overlap (the margin around the edge of the bucket which is also coppied into another bucket). Can be overridden by chamber-specific patterns
             * @param spacePointOverlap: Default space point overlap in meters */
            Gaudi::Property<double> m_spacePointOverlap{this, "spacePointOverlap", 25.*Gaudi::Units::cm,
                                                        "Hits that are within <spacePointOverlap> of the bucket margin. "
                                                        "Are copied to the next bucket"};

            /** @brief Chamber-pattern dependent space point window. Example: cfg.getEventAlgo("MuonSpacePointMakerAlg").spacePointWindowPatterns = {"BIL_eta3_phi3": 0.2 * m, "BML*": 0.5 * m, "BOL_eta3*": 0.7 * m} */
            Gaudi::Property<BucketPatternMap> m_spacePointWindowPatterns{ this, "spacePointWindowPatterns", {},
                                                    "Chamber-pattern dependent space point window. " "Examples: BML*=0.5, BIL_eta3_phi3=0.2"};

            /** @brief Chamber-pattern dependent maximum bucket length. Example: cfg.getEventAlgo("MuonSpacePointMakerAlg").maxBucketLengthPatterns = {"BIL_eta3_phi3": 0.2 * m, "BML*": 0.5 * m} */
            Gaudi::Property<BucketPatternMap> m_maxBucketLengthPatterns{ this, "maxBucketLengthPatterns", {},
                                                    "Chamber-pattern dependent max bucket length. " "Examples: BML*=0.5, BIL_eta3_phi3=0.2"};
            
            /** @brief Chamber-pattern dependent space point overlap. Example: cfg.getEventAlgo("MuonSpacePointMakerAlg").spacePointOverlapPatterns = {"BIL_eta3_phi3": 0.2 * m, "BML*": 0.5 * m} */
            Gaudi::Property<BucketPatternMap> m_spacePointOverlapPatterns{ this, "spacePointOverlapPatterns", {},
                                                    "Chamber-dependent overrides of spacePointOverlap" };

            /** @brief Map of bucket parameters for each chamber */
            std::unordered_map<const MuonGMR4::Chamber*, BucketParameters> m_bucketParameters{};
            /** @brief Default bucket parameters used if no chamber-specific parameters are found, defaults into m_spacePointWindow, m_maxBucketLength, and m_spacePointOverlap */
            BucketParameters m_defaultBucketParameters{};
    
            Gaudi::Property<bool> m_doStat{this, "doStats", false, 
                                           "If enabled the algorithm keeps track how many hits have been made" };
            
            Gaudi::Property<unsigned> m_capacityBucket{this,"CapacityBucket" , 50};
            std::unique_ptr<SpacePointStatistics> m_statCounter ATLAS_THREAD_SAFE{};

            Gaudi::Property<double> m_maxOccRpcEta{this, "maxRpcEtaOccupancy", 0.1, 
                                                   "Maximum occpancy of Rpc eta hits in a gasGap"};
            Gaudi::Property<double> m_maxOccRpcPhi{this, "maxRpcPhiOccupancy", 0.1, 
                                                   "Maximum occpancy of Rpc phi hits in a gasGap"};

            Gaudi::Property<double> m_maxOccTgcEta{this, "maxTgcEtaOccupancy", 0.1, 
                                                   "Maximum occpancy of Tgc eta hits in a gasGap"};
            Gaudi::Property<double> m_maxOccTgcPhi{this, "maxTgcPhiOccupancy", 0.1, 
                                                   "Maximum occpancy of Tgc phi hits in a gasGap"};
    };
}


#endif
