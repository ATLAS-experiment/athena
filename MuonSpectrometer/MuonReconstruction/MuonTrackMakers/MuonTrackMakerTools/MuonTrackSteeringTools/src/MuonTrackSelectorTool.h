/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONTRACKSELECTOR_H
#define MUON_MUONTRACKSELECTOR_H

#include <atomic>
#include <set>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkToolInterfaces/ITrackSummaryHelperTool.h"
#include "TrkTrack/Track.h"

namespace Trk {
    class Track;
}

namespace Muon {

    /**
       @brief tool to select tracks

    */
    class MuonTrackSelectorTool : public AthAlgTool {
    public:
        /** @brief constructor */
        MuonTrackSelectorTool(const std::string&, const std::string&, const IInterface*);

        /** @brief destructor */
        virtual ~MuonTrackSelectorTool() = default;

        /** @brief AlgTool initilize */
        StatusCode initialize();

        /** @brief AlgTool finalize */
        StatusCode finalize();

        /** @brief calculate holes in a given chamber using local straight line extrapolation
            @param pars TrackParameters in the chamber
            @param chId Identifier of the chamber
            @param tubeIds set containing the Identifier of the hits that should not be counted as holes
            @return a vector of hole Identifiers
        */
        std::vector<Identifier> holesInChamber(const Trk::TrackParameters& pars, const Identifier& chId,
                                               const std::set<Identifier>& tubeIds) const;

        /** @brief returns true if the track satisfies the selection criteria else false */
        bool decision(Trk::Track& track) const;

    private:
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{
            this, "edmHelper", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
            "Handle to the service providing the IMuonEDMHelperSvc interface"};  //!< EDM Helper tool
        PublicToolHandle<Muon::MuonEDMPrinterTool> m_printer{this, "EDMPrinter", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool",
                                                       "helper to nicely print out tracks"};
        ToolHandle<Trk::ITrackSummaryHelperTool> m_trackSummaryTool{this, "TrackSummaryHelperTool",
                                                                    "Muon::MuonTrackSummaryHelperTool/MuonTrackSummaryHelperTool"};

        Gaudi::Property<double> m_holeHitRatioCutPerStation{this, "HolesToHitsRatioCutPerStation", 1.1};
        Gaudi::Property<double> m_chi2NDofCut{this, "Chi2NDofCut", 20};
        Gaudi::Property<unsigned int> m_minMdtHitsPerStation{this, "MinimumNumberOfMdtHitsPerStation", 3};
        Gaudi::Property<unsigned int> m_maxMdtHolesPerTwoStationTrack{this, "MaxMdtHolesOnTwoStationTrack", 5};
        Gaudi::Property<unsigned int> m_maxMdtHolesPerTrack{this, "MaxMdtHolesOnTrack", 5};
        Gaudi::Property<unsigned int> m_minCscHitsPerStation{this, "MinimumNumberOfCscHitsPerStation", 3 };

        Gaudi::Property<bool> m_useRPCHoles{this, "UseRPCHoles", true};
        Gaudi::Property<bool> m_useTGCHoles{this, "UseTGCHoles", true};
        Gaudi::Property<bool> m_useCSCHoles{this, "UseCSCHoles", true};
        Gaudi::Property<bool> m_useMDTHoles{this, "UseMDTHoles", true};
        Gaudi::Property<bool> m_ignoreTriggerHolesInLayersWithHits{this, "IgnoreTriggerHolesInChamberWithHits", true};
        Gaudi::Property<bool> m_useRPCTimeWindow{this, "ApplyRPCTimeWindow", false};
        Gaudi::Property<bool> m_removeTwoStationTrackWithoutTriggerHits{this, "RemoveTwoStationTrackWithoutTriggerHits", false};
        Gaudi::Property<bool> m_countMdtOutliersAsHoles{this, "CountMDTOutlierAsHoles", false};
        Gaudi::Property<bool> m_removeSingleStationTracks{this, "RemoveSingleStationTracks", false};
        Gaudi::Property<bool> m_tightSingleStationCuts{this, "TightSingleStationCuts", false};

        Gaudi::Property<bool> m_requireSanePerigee{this,"RequireSanePerigee", true, 
                                                  "Ensures that the covariance of the perigee parameters has a positive trace"};

        /** internal data structure */
        struct StationData {
            StationData() = default;
            bool isMdt{false};
            bool isCsc{false};
            bool isNSW{false};
            bool isTrigger{false};
            bool mdtHasHitsinBothMl() const { return mdtHasHitsinMl1 && mdtHasHitsinMl2; }
            bool mdtHasHitsinMl1{false};
            bool mdtHasHitsinMl2{false};
            unsigned int netaHits{0};
            unsigned int nphiHits{0};
            unsigned int netaHoles{0};
            unsigned int nphiHoles{0};

            unsigned int netaTrigHits{0};
            unsigned int nphiTrigHits{0};
            unsigned int netaTrigHoles{0};
            unsigned int nphiTrigHoles{0};
        };

        /** counter for statistics */
        mutable std::atomic_uint m_ntotalTracks{0};
        mutable std::atomic_uint m_failedChi2NDofCut{0};
        mutable std::atomic_uint m_failedSingleStationCut{0};
        mutable std::atomic_uint m_failedRPCAveMinTimeCut{0};
        mutable std::atomic_uint m_failedRPCAveMaxTimeCut{0};
        mutable std::atomic_uint m_failedRPCSpreadTimeCut{0};
        mutable std::atomic_uint m_failedTwoStationsCut{0};
        mutable std::atomic_uint m_failedTwoStationsMaxMDTHoleCut{0};
        mutable std::atomic_uint m_failedTwoStationsMaxHoleCut{0};
        mutable std::atomic_uint m_failedTwoStationsGoodStationCut{0};
        mutable std::atomic_uint m_failedTriggerStationCut{0};
        mutable std::atomic_uint m_failedMaxMDTHoleCut{0};
        mutable std::atomic_uint m_failedMaxHoleCut{0};
    };

}  // namespace Muon

#endif
