/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IRECMUONCOMBINEDINDETEXTENSIONTOOL_H
#define IRECMUONCOMBINEDINDETEXTENSIONTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "MuonCombinedEvent/InDetCandidateCollection.h"
#include "MuonCombinedEvent/InDetCandidateToTagMap.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "TrkSegment/SegmentCollection.h"
#include "TrkTrack/TrackCollection.h"

namespace MuonCombined {

    /** @class IMuonCombinedInDetExtensionTool
        @brief interface for tools buildingmuons from ID candidates

        @author Niels van Eldik
     */

    class IMuonCombinedInDetExtensionTool : virtual public IAlgTool {
    public:
        struct MuonPrdData {
            const Muon::MdtPrepDataContainer* mdtPrds{nullptr};
            const Muon::RpcPrepDataContainer* rpcPrds{nullptr};
            const Muon::TgcPrepDataContainer* tgcPrds{nullptr};
            const Muon::CscPrepDataContainer* cscPrds{nullptr};
            const Muon::sTgcPrepDataContainer* stgcPrds{nullptr};
            const Muon::MMPrepDataContainer* mmPrds{nullptr};
        };

        DeclareInterfaceID(MuonCombined::IMuonCombinedInDetExtensionTool, 1, 0);
        /**IMuonCombinedInDetExtensionTool interface: build combined muons from ID candidates */
        virtual void extend(const InDetCandidateCollection& inDetCandidates, 
                            InDetCandidateToTagMap* tagMap, 
                            TrackCollection* combTracks,
                            TrackCollection* meTracks, 
                            Trk::SegmentCollection* segments, 
                            const EventContext& ctx) const = 0;

        /*New interface including PRDs for MuGirl segment-finding*/
        virtual void extendWithPRDs(const InDetCandidateCollection& inDetCandidates, 
                                    InDetCandidateToTagMap* tagMap,
                                    IMuonCombinedInDetExtensionTool::MuonPrdData prdData, 
                                    TrackCollection* combTracks,
                                    TrackCollection* meTracks, 
                                    Trk::SegmentCollection* segments, 
                                    const EventContext& ctx) const = 0;

        virtual ~IMuonCombinedInDetExtensionTool() = default;
    };

}  // namespace MuonCombined

#endif
