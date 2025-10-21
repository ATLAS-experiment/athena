
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPATTERNCNV_MUONPATTERNCNVALG_H
#define MUONPATTERNCNV_MUONPATTERNCNVALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRecToolInterfaces/HoughDataPerSec.h"
#include "MuonPattern/MuonPatternCombinationCollection.h"
#include "MuonPatternEvent/MuonPatternContainer.h"
namespace MuonR4{
    /** @brief The MuonPatternCnvAlg converts the SegmentSeeds produced by the R4 pattern recognition chain
     *         into the segment seeds that can be consumed by the legacy muon segment maker
     */
    class PatternCnvAlg : public AthReentrantAlgorithm{
        public:

            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            StatusCode initialize() override final;

            StatusCode execute(const EventContext& ctx) const override final;
    
        private:
            /*** @brief Fetches a MuonPrepData object from the PrepData container by matching the parsed Identifier.
               *        Nullptr is returned if the object does not exist and an error message is printed
               * @param prdId: Identifier of the measurement to fetch
               * @param prdContainer: Pointer to the MuonPrepData container to fetch the object from.  
               */
            template <class PrdType> 
                const PrdType* fetchPrd(const Identifier& prdId,
                                        const Muon::MuonPrepDataContainerT<PrdType>* prdContainer) const;
            
            /** @brief Converts each segment seed first into a PatternCombination - serving as input for the
             *         legacy segment builder chain - and then to an entry of the hough data per sector.
             *  @param ctx: The event context of the current event
             *  @param seedContainer: Segment seed container to convert
             *  @param patternContainer: Output patternCombination collection container
             *  @param houghDataSec: Hough input data per sector.
            */
            StatusCode convertSeed(const EventContext& ctx,
                                   const SegmentSeedContainer& seedContainer,
                                   ::MuonPatternCombinationCollection& patternContainer,
                                   Muon::HoughDataPerSectorVec& houghDataSec) const;

            /** @brief: Converts the maximum back into the HoughDataPersector */        
            void convertMaximum(const Muon::MuonPatternChamberIntersect& intersect,
                                Muon::HoughDataPerSectorVec& houghDataSec)const;
            
            


            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Prep data container keys */
            SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_keyTgc{this, "TgcKey", "TGC_MeasurementsAllBCs"};
            SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_keyRpc{this, "RpcKey", "RPC_Measurements"};
            SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_keyMdt{this, "MdtKey", "MDT_DriftCircles"};
            SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_keysTgc{this, "sTgcKey", "STGC_Measurements"};
            SG::ReadHandleKey<Muon::MMPrepDataContainer> m_keyMM{this, "MmKey", "MM_Measurements"};

            SG::ReadHandleKeyArray<SegmentSeedContainer> m_readKeys{this, "Patterns", {"MuonHoughStationSegmentSeeds"}};
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            SG::WriteHandleKey<::MuonPatternCombinationCollection> m_combiKey{this, "PatternCombiKey", "MuonLayerHoughCombis"};
            SG::WriteHandleKey<Muon::HoughDataPerSectorVec> m_dataPerSecKey{this, "HoughDataPerSecKey",
                                                                            "HoughDataPerSectorVec", "HoughDataPerSectorVec key"};

    };

}

#endif