/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONCHAMBERHOLERECOVERYTOOL_H
#define MUON_MUONCHAMBERHOLERECOVERYTOOL_H

#include <set>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMdtDriftCircleOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonHoleRecoveryTool.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkDetDescrInterfaces/ITrackingVolumesSvc.h"
#include "TrkTrack/Track.h"
#include "MuonStationIntersectCond/MuonIntersectGeoData.h"


namespace Muon {
    /**
       @brief tool to select tracks
    */
    class MuonChamberHoleRecoveryTool : public AthAlgTool, virtual public IMuonHoleRecoveryTool {
   
    public:
        MuonChamberHoleRecoveryTool(const std::string&, const std::string&, const IInterface*);
        virtual ~MuonChamberHoleRecoveryTool() = default;

        using NewTrackStates = std::vector<std::unique_ptr<const Trk::TrackStateOnSurface>>;

        StatusCode initialize() override;

        /** @brief returns a new track with holes recovered */
        std::unique_ptr<Trk::Track> recover(const Trk::Track& track, const EventContext& ctx) const override;

        // made public
        void createHoleTSOSsForClusterChamber(const Identifier& detElId, const EventContext& ctx, const Trk::TrackParameters& pars,
                                              std::set<Identifier>& layIds,
                                              NewTrackStates& states) const override;

    private:
        struct RecoveryState {
            RecoveryState(const Trk::Track& trk);

            Trk::TrackStates::const_iterator begin() const;
            Trk::TrackStates::const_iterator end() const;

            const Trk::TrackStateOnSurface* tsos() const;

            /// Increments the internal iterator 
            bool nextState();
            /// Switch indicating whether the track states are copied onto the global cache vector
            /// or onto the chamber cache vector. The chamber cache vector is sorted and moved onto the global
            //  vector once a new chamber is reached
            enum class CopyTarget {
                GlobalTrkStates,
                ChamberTrkStates
            };
            void copyState(CopyTarget target);

            /// List of all measurement layers on track
            std::set<Identifier> layersOnTrk{};
            /// Identifier of the current trackStateOnSurface
            Identifier tsosId{};

            /// Vector of the track states on surface in the current chamber
            NewTrackStates chamberStates{};
            /// Track parameters of the chamber. Usually that are the parameters of the 
            /// first TSOS in the chamberStates vector
            const Trk::TrackParameters* chamberPars() const;
            /// Sorts the hits accumulated in the current chamber using the first 
            /// track parameters in that chamber. The sorted hits are moved onto the
            /// final track state vector
            void finalizeChamber();

            /// Moves the recovered track states on surface onto a Trk::TrackStates
            /// to be piped into a new Trk::Track object
            std::unique_ptr<Trk::TrackStates> releaseStates();

        private:
            const Trk::Track& m_trk;
            Trk::TrackStates::const_iterator m_curr_itr{begin()};
            bool m_nextCalled{false};
            /// 
            NewTrackStates m_newStates{};            
            std::set<const Trk::TrackStateOnSurface*> m_copiedStates{};
            
        };

        /// Increments the internal iterator of the RecoveryState until the next muon measurement on track is found
        /// Measurements inside the MS volume or any scatterer are copied automatically copied onto the target vector
        bool getNextMuonMeasurement(RecoveryState& trkRecov, RecoveryState::CopyTarget target) const;

        //// Loops over all muon hits in a muon chamber and tries to find missed ones by 
        ///  checking each tracking layer for hits. If a new chamber is reached another 
        ///  iteration of the method is called
        void recoverHitsInChamber(const EventContext& ctx, RecoveryState& trkRecov) const;


        /** @brief calculate holes in a given chamber using local straight line extrapolation
                @param pars TrackParameters in the chamber
                @param chId Identifier of the chamber
                @param tubeIds set containing the Identifier of the hits that should not be counted as holes
                @return a vector of hole Identifiers
        */
        std::set<Identifier> holesInMdtChamber(const EventContext& ctx, const Amg::Vector3D& position, const Amg::Vector3D& direction, 
                                               const Identifier& chId, const std::set<Identifier>& tubeIds) const;

        void recoverMdtHits(const EventContext& ctx, 
                            const Identifier& chId,
                            const Trk::TrackParameters& pars,
                            NewTrackStates& newStates, 
                            std::set<Identifier>& knownLayers) const;

        
        void recoverClusterHits(const EventContext& ctx, 
                                const Identifier& chId,
                                const Trk::TrackParameters& chambPars,
                                NewTrackStates& newStates, 
                                std::set<Identifier>& knownLayers) const;



        /// Returns a set of all layer Identifiers of the associated track hits
        std::set<Identifier> layersOnTrkIds(const Trk::Track& track) const;
        
        /// Returns the detector element associated with the muon Identifier
        const Trk::TrkDetElementBase* getDetectorElement(const EventContext& ctx, const Identifier& id) const;

        /// Returns a set of all layer Identifiers (GasGap + channel orientation) 
        /// in a muon chamber of type X excluding the ones in the knownLayers set 
        std::set<Identifier> getHoleLayerIds(const Identifier& detElId, 
                                             const std::set<Identifier>& knownLayers) const;
        
        /// @brief Attempts to recover all missing hits in a chamber.
        template <class Prd> NewTrackStates recoverChamberClusters(const EventContext& ctx,
                                                                   const SG::ReadHandleKey<MuonPrepDataContainerT<Prd>>& prdKey,
                                                                   const Identifier& chambId,
                                                                   const Trk::TrackParameters& parsInChamb,
                                                                   std::set<Identifier>& knownLayers) const;
        
        template <class Prd> std::vector<const Prd*> loadPrepDataHits(const EventContext& ctx, 
                                                                      const SG::ReadHandleKey<MuonPrepDataContainerT<Prd>>& key, 
                                                                      const std::set<Identifier>& layerIds) const;
        
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{this, "edmHelper", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
                                                              "Handle to the service providing the IMuonEDMHelperSvc interface"};

        ServiceHandle<Trk::ITrackingVolumesSvc> m_trackingVolumesSvc{this, "TrackingVolumesSvc", "Trk::TrackingVolumesSvc/TrackingVolumesSvc"};



        
        PublicToolHandle<Muon::MuonEDMPrinterTool> m_printer{this, "EDMPrinter", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"};
        ToolHandle<Trk::IExtrapolator> m_extrapolator{this, "Extrapolator", "Trk::Extrapolator/MuonExtrapolator"};
        ToolHandle<Muon::IMdtDriftCircleOnTrackCreator> m_mdtRotCreator{this, "MdtRotCreator",
                                                                        "Muon::MdtDriftCircleOnTrackCreator/MdtDriftCircleOnTrackCreator",
                                                                        "IMdtDriftCircleOnTrackCreator full calibration"};
        ToolHandle<Muon::IMuonClusterOnTrackCreator> m_cscRotCreator{
            this, "CscRotCreator", "", "IMuonClusterOnTrackCreator for cscs"};
        ToolHandle<Muon::IMuonClusterOnTrackCreator> m_clusRotCreator{this, "ClusterRotCreator",
                                                                      "Muon::MuonClusterOnTrackCreator/MuonClusterOnTrackCreator",
                                                                      "IMuonClusterOnTrackCreator for RPC, TGC, NSW hits"};
        ToolHandle<Trk::IResidualPullCalculator> m_pullCalculator{this, "PullCalculator",
                                                                  "Trk::ResidualPullCalculator/ResidualPullCalculator"};

        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                "Key of input MuonDetectorManager condition data"};

        SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_key_mdt{this, "MdtPrepDataContainer", "MDT_DriftCircles", "MDT PRDs"};
        SG::ReadHandleKey<Muon::CscPrepDataContainer> m_key_csc{this, "CscPrepDataContainer", "CSC_Clusters", "CSC PRDS"};
        SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_key_tgc{this, "TgcPrepDataContainer", "TGC_Measurements", "TGC PRDs"};
        SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_key_rpc{this, "RpcPrepDataContainer", "RPC_Measurements", "RPC PRDs"};
        SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_key_stgc{this, "sTgcPrepDataContainer", "STGC_Measurements", "sTGC PRDs"};
        SG::ReadHandleKey<Muon::MMPrepDataContainer> m_key_mm{this, "MMPrepDataContainer", "MM_Measurements", "MM PRDs"};
        
        
        SG::ReadCondHandleKey<Muon::MuonIntersectGeoData> m_chamberGeoKey{this, "ChamberGeoKey", "MuonStationIntersects", "Pointer to hole search service"};
   
        Gaudi::Property<bool> m_addMeasurements{this, "AddMeasurements", true};
        Gaudi::Property<bool> m_detectBadSort{this, "DetectBadSorting", false};

        Gaudi::Property<double> m_associationPullCutEta{this, "AssociationPullCutEta", 3};
        Gaudi::Property<double> m_associationPullCutPhi{this, "AssociationPullCutPhi", 10};
        Gaudi::Property<double> m_adcCut{this, "AdcCut", 50};
    };

}  // namespace Muon

#endif
