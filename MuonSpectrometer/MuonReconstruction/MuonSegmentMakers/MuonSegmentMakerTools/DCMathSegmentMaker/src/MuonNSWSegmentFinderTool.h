/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONROADFINDERTOOL_H
#define MUONROADFINDERTOOL_H

#include <utility>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "CxxUtils/ArrayHelper.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonTrackCleaner.h"
#include "MuonRecToolInterfaces/IMuonTrackToSegmentTool.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonSegmentMakerToolInterfaces/IMuonNSWSegmentFinderTool.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkToolInterfaces/ITrackAmbiguityProcessorTool.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
namespace Muon {

    class MuonNSWSegmentFinderTool;
    struct NSWSeed {

        /// Struct caching the MuonClusterOnTrack and providing the
        /// orientation of the strip in addtion
        struct SeedMeasurement{
            SeedMeasurement() = default;
            explicit SeedMeasurement(const Muon::MuonClusterOnTrack* cl) ;
            /// Accomodations of the interface to make the changes as transparent as possible
            const Muon::MuonClusterOnTrack* get() const{return m_cl;}
            const Muon::MuonClusterOnTrack* operator->() const{return get();}
            operator const Muon::MuonClusterOnTrack* () const { return get(); }
          
            bool operator==(const SeedMeasurement& other) const {return get() == other.get();}
            bool operator!() const {return !get();}
            operator bool() const {return get();}
                     
            const Amg::Vector3D& dir() const {return m_dir;}
            const Amg::Vector3D& pos() const {return m_cl->globalPosition();}

            double dirDot(const SeedMeasurement& other) const {return dirDot(other.dir());}
            double dirDot(const Amg::Vector3D& v) const { return dir().dot(v); }
        
            void setDistance(double d){m_distance = d;}
            double distance() const {return m_distance;}          
            
        private:
            const Muon::MuonClusterOnTrack* m_cl{nullptr};
            Amg::Vector3D m_dir {Amg::Vector3D::Zero()};
            double m_distance{0.};           
        };

        NSWSeed() = default;

        //// Constructor used for the three micro mega stereo seeds
        NSWSeed(const MuonNSWSegmentFinderTool* parent, const std::array<SeedMeasurement, 4>& seed,
                const std::array<double, 2>& lengths);

        //// Constructor to build seeds from 2 eta/phi strips
        NSWSeed(const MuonNSWSegmentFinderTool* parent,  const SeedMeasurement& first, const SeedMeasurement& second);
        /// Constructor to build a seed from an existing segment
        NSWSeed(const MuonNSWSegmentFinderTool* parent, const Muon::MuonSegment& seg);
        //// Constructor to build a seed to create 3D segments
        NSWSeed(const MuonNSWSegmentFinderTool* parent, const Amg::Vector3D& pos, const Amg::Vector3D& dir);

        double chi2() const {return m_chi2;}
        /// Returns the number of measurements
        size_t size() const { return m_size; }
        /// Returns the position of the seed
        const Amg::Vector3D& pos() const { return m_pos; }
        /// Returns the direction of the seed
        const Amg::Vector3D& dir() const { return m_dir; }

        /// Returns the contained measurements
        using MeasVec = std::vector<SeedMeasurement>;
        MeasVec measurements() const;
        /// Tries to add the measurement to the seeds. Returns false if the measurement is incompatible with the seed or the seed is invalid
        bool add(SeedMeasurement meas, double max_uncert);
        /// Adds a calibrated cluster to the garbage collection
        const Muon::MuonClusterOnTrack* newCalibClust(std::unique_ptr<const Muon::MuonClusterOnTrack> new_clust);

        enum class SeedOR { NoOverlap, Same, SubSet, SuperSet };
        SeedOR overlap(const NSWSeed& other) const;

    private:
        /// Returns the channel of the measurement on the layer
        int channel(const SeedMeasurement& meas) const;
        /// Checks whether the measurement is already part of the seed
        bool find(const SeedMeasurement& meas) const;
        /// Calculates the minimal distance between seed and measurement
        double distance(const SeedMeasurement& meas) const;
        bool insert(const Muon::MuonClusterOnTrack* cl);

        bool insert(SeedMeasurement meas);

        const MuonNSWSegmentFinderTool* m_parent{nullptr};
        /// Helper pair to cache the measurements with the respective distances
        using SeedMeasCache = std::array<SeedMeasurement, 16>;
        /// Cache the eta measurements
        SeedMeasCache m_measurements{};
        /// Cache the phi measurements
        SeedMeasCache m_phiMeasurements{};
        /// Cache the sTGC pad measurements
        SeedMeasCache m_padMeasurements{};
        /// Starting position of the seed
        Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
        /// Seed direction
        Amg::Vector3D m_dir{Amg::Vector3D::Zero()};
        /// seed width
        double m_width{0.};
        /// Chi2
        double m_chi2{0.};
        /// Added measurements on track
        size_t m_size{0};
        /// Garbage container per seed
        std::set<std::shared_ptr<const Muon::MuonClusterOnTrack>> m_calibClust{};
    };

    class MuonNSWSegmentFinderTool : virtual public IMuonNSWSegmentFinderTool, public AthAlgTool {
    public:
        /** default constructor */
        MuonNSWSegmentFinderTool(const std::string& type, const std::string& name, const IInterface* parent);
        /** destructor */
        virtual ~MuonNSWSegmentFinderTool() = default;

        virtual StatusCode initialize() override;

    private:
        ServiceHandle<IMuonIdHelperSvc> m_idHelperSvc{
            this,
            "MuonIdHelperSvc",
            "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
        };
        ServiceHandle<IMuonEDMHelperSvc> m_edmHelperSvc{
            this,
            "edmHelper",
            "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
            "Handle to the service providing the IMuonEDMHelperSvc interface",
        };  //<! Id helper tool

        ToolHandle<Trk::ITrackAmbiguityProcessorTool> m_ambiTool{
            this,
            "SegmentAmbiguityTool",
            "Trk::SimpleAmbiguityProcessorTool/MuonAmbiProcessor",
        };  //!< Tool for ambiguity solving
        ToolHandle<Trk::ITrackFitter> m_slTrackFitter{
            this,
            "SLFitter",
            "Trk::GlobalChi2Fitter/MCTBSLFitter",
        };  //<! fitter, always use straightline
        ToolHandle<IMuonTrackToSegmentTool> m_trackToSegmentTool{
            this,
            "TrackToSegmentTool",
            "Muon::MuonTrackToSegmentTool/MuonTrackToSegmentTool",
        };  //<! track to segment converter
        PublicToolHandle<MuonEDMPrinterTool> m_printer{
            this,
            "Printer",
            "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool",
        };
        ToolHandle<IMuonTrackCleaner> m_trackCleaner{
            this,
            "TrackCleaner",
            "Muon::MuonTrackCleaner/MuonTrackCleaner",
        };
        ToolHandle<Trk::ITrackSummaryTool> m_trackSummary{
            this,
            "TrackSummaryTool",
            "Trk::TrackSummaryTool/MuidTrackSummaryTool",
        };

        ToolHandle<IMuonClusterOnTrackCreator> m_muonClusterCreator{this, "MuonClusterCreator", ""};

        Gaudi::Property<bool> m_ipConstraint{this, "IPConstraint", true};  // use a ip perigee(0,0) constraint in the segment fit
        Gaudi::Property<double> m_maxClustDist{this, "ClusterDistance", 5.};
        Gaudi::Property<int> m_nOfSeedLayers{this, "NOfSeedLayers", 1};


        Gaudi::Property<bool> m_useStereoSeeding{this, "SeedMMStereos", true};
        Gaudi::Property<bool> m_usesTGCSeeding{this, "SeedWithsTGCS", true};

        /// Protection against slobbering Micromega events
        Gaudi::Property<unsigned int> m_ocupMmBinWidth{this, "MmOccupancyBinWidth", 100, 
        "Size of the channel window to group the MicroMegaCluster"};
        Gaudi::Property<unsigned int> m_ocupMmNumPerBin{this, "MmOccupancySingleCut", 5, 
        "Cut on the number of MicroMega clusters in a particular occupancy bin"};
        Gaudi::Property<unsigned int> m_ocupMmNumPerPair{this, "MmOccupancyPairCut", 7,
        "Cut on the number of MicroMega clusters in two neighbouring occupancy bins"};
    public:
        using SeedMeasurement = NSWSeed::SeedMeasurement;
        using MeasVec = NSWSeed::MeasVec;
        using LayerMeasVec = std::vector<MeasVec>;
      

        // find segments given a list of MuonCluster
        // segments can be added directly to a SegmentCollection, if they are to be written to SG, or returned in a list for
        // further processing
        void find(const EventContext& ctx, SegmentMakingCache& cache) const override;

        int wedgeNumber(const Muon::MuonClusterOnTrack* cluster) const;
        int layerNumber(const Muon::MuonClusterOnTrack* cluster) const;
        /// Returns the channel of the measurement on the layer
        int channel(const Muon::MuonClusterOnTrack* cluster) const;
        
        const IMuonIdHelperSvc* idHelper() const;
        bool isPad(const Muon::MuonClusterOnTrack* clust) const;
        bool isStrip(const Muon::MuonClusterOnTrack* clust) const;
        bool isWire(const Muon::MuonClusterOnTrack* clust) const;
        template <size_t N>
        std::string printSeed(const std::array<SeedMeasurement, N>& seed) const;
        std::string print(const SeedMeasurement& meas) const;
        std::string print(const MeasVec& clusters) const;
        std::string print(const LayerMeasVec& sortedVec) const;
        

        
    private:
        enum HitType { Eta = 1, Phi = 1 << 1, Wire = 1 << 2, Pad = 1 << 3 };

        /// @brief  Runs the NSW segment maker by combining 4 Micromega layers to a stereo seed. Subsequentally, 
        ///         the hits on the hit road are added to the seeed and the segment is fitted
        /// @param ctx EventContext
        /// @param allClusts Collection of all EventContext objects
        /// @param singleWedge 0 (use all 4 multiplets) 2/3
        /// @return 
        std::vector<std::unique_ptr<Muon::MuonSegment>> findStereoSegments(const EventContext& ctx,
                                                                           const std::vector<const Muon::MuonClusterOnTrack*>& allClusts,
                                                                           int singleWedge = 0) const;

        /// @brief Combines 2 sTgc strip layers to find 2D segments constraining the muon in the eta direction
        std::vector<std::unique_ptr<Muon::MuonSegment>> findStgcPrecisionSegments(const EventContext& ctx, 
                                                                                  const std::vector<const Muon::MuonClusterOnTrack*>& MuonClusters, 
                                                                                  int singleWedge = 0) const;
        // recontruct 3D segments
        std::vector<std::unique_ptr<Muon::MuonSegment>> find3DSegments(const EventContext& ctx,
                                                                       const std::vector<const Muon::MuonClusterOnTrack*>& MuonClusters,
                                                                       std::vector<std::unique_ptr<Muon::MuonSegment>>& etaSegs,
                                                                       int singleWedge = 0) const;
        // clean the clusters
        MeasVec cleanClusters(const std::vector<const Muon::MuonClusterOnTrack*>& MuonClusters,
                              int hit_sel, int singleWedge) const;

        // classify clusters by layer (one vector per layer)
        LayerMeasVec classifyByLayer(const MeasVec& clusters, int hit_sel) const;

        // find segment seeds
        std::vector<NSWSeed> segmentSeedFromStgc(const LayerMeasVec& orderedClusters,
                                                 bool usePhi) const;

        std::vector<NSWSeed> segmentSeedFromMM(const LayerMeasVec& orderedClusters) const;
        std::vector<NSWSeed> segmentSeedFromMM(const LayerMeasVec& orderedClusters, 
                                               std::array<unsigned int,4> selLayers,
                                               unsigned int& trial_counter) const;

        
        std::vector<NSWSeed> segmentSeedFromPads(const LayerMeasVec& orderedClusters,
                                                 const Muon::MuonSegment& etaSeg) const;

        std::vector<std::pair<double, double>> getPadPhiOverlap(const std::vector<std::vector<const Muon::sTgcPrepData*>>& pads) const;

        // associate clusters to the segment seeds
        int getClustersOnSegment(const LayerMeasVec& orderedclusters, NSWSeed& seed,
                                 const std::set<unsigned int>& exclude, bool useStereo=true ) const;

        // get the clusters after calibration
        MeasVec getCalibratedClusters(NSWSeed& seed) const;

        // fit eta and phi hits and fill the track collection
        bool hitsToTrack(const EventContext& ctx, const MeasVec& etaHitVec,
                         const MeasVec& phiHitVec, const Trk::TrackParameters& startpar,
                         TrackCollection& segTrkColl) const;

        /// creates the IP constraint
        std::unique_ptr<Trk::PseudoMeasurementOnTrack> ipConstraint(const EventContext& ctx) const;

        std::vector<NSWSeed> resolveAmbiguities(std::vector<NSWSeed>&& unresolved) const;
        std::vector<std::unique_ptr<Muon::MuonSegment>> resolveAmbiguities(const EventContext& ctx, 
                                                                           const TrackCollection& segColl,
                                                                           const Trk::Segment::Author a) const;
        std::unique_ptr<Trk::Track> fit(const EventContext& ctx, const std::vector<const Trk::MeasurementBase*>& fit_meas,
                                        const Trk::TrackParameters& perigee) const;

        
        enum class ChannelConstraint{
            InWindow,
            TooNarrow,
            TooWide

        };
        /// Checks whether the two measurements are compatible within the IP constraint        
        ChannelConstraint compatiblyFromIP(const SeedMeasurement& meas1, const SeedMeasurement& meas2) const;
        /// @brief Returns the minimal & maximal radial distance of a measurement
        std::pair<double, double> coveredRadii(const SeedMeasurement& meas) const;
        
        /// @brief  Removes clusters from high activity areas in the detector
        MeasVec vetoBursts( MeasVec && clustInLay ) const;

    };

}  // namespace Muon
#endif  // MUONROADFINDERTOOL_H
