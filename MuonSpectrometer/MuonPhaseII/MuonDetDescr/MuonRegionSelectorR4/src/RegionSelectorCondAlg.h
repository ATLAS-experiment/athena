/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
 
#ifndef MuonRegionSelectorR4_RegionSelectorCondAlg_h
#define MuonRegionSelectorR4_RegionSelectorCondAlg_h


#include "AthenaBaseComps/AthCondAlgorithm.h"

#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"


#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "IRegionSelector/IRegSelLUTCondData.h"
#include "RegSelLUT/RegSelSiLUT.h"


#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonCablingData/RpcCablingMap.h"



namespace MuonR4 {
    /** @brief Conditions algorithm to create the region selector trigger tables from 
     *         the R4 readout geoemetry */
    class RegionSelectorCondAlg : public AthCondAlgorithm {

        public:
            using AthCondAlgorithm::AthCondAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;

 
        private:
            /** @brief Declare the dependency of the region selector table on the
             *         other keys set
             *  @param ctx: EventContext to load the conditions object
             *  @param writeHandle: Reference to the new writeHandle to be written to Storegate
             *  @param key: Primary key on which the dependency is declared
             *  @param others: Other keys for recursive dependency declaration */
            template <typename Key_t,
                      typename... OtherKey_t>
            StatusCode addDependency(const EventContext& ctx,
                                     SG::WriteCondHandle<IRegSelLUTCondData>& writeHandle,
                                     const SG::ReadCondHandleKey<Key_t>& key,
                                     OtherKey_t... others) const;
            /** @brief Retrieve the idHelper for the given detector technology
             *  @param type: Muon detector technology type */
            const MuonIdHelper& getIdHelper(const ActsTrk::DetectorType type) const;
            /** @brief Returns the list of ROB ids associated with the  module 
             *  @param ctx: EventContext to access the conditions store
             *  @param moduleID: Chamber module identifier */
            std::set<std::uint32_t> getRobIDs(const EventContext& ctx,
                                              const Identifier& moduleID) const;
            /** @brief Detector manager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            Gaudi::Property<bool>  m_printTable{this, "PrintTable", false};
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Region selector table written by the algorithm */
            SG::WriteCondHandleKey<IRegSelLUTCondData> m_tableKey{ this, "RegSelLUT", "", "Region Selector lookup table" };
            /** @brief Dependency on the alignment constants */
            SG::ReadCondHandleKey<ActsTrk::DetectorAlignStore> m_alignKey{this, "AlignKey", ""};
            /** @brief Dependency on the Mdt cabling map */
            SG::ReadCondHandleKey<MuonMDT_CablingMap> m_cablingMdtKey{this, "MdtCablingKey", "MuonMDT_CablingMap"};
            /** @brief Dependency on the phase II Rpc cabling map */
            SG::ReadCondHandleKey<Muon::RpcCablingMap> m_cablingRpcKey{this, "RpcCablingKey", "MuonNRPC_CablingMap"};
            /** @brief Instantiate a new transform cache to ensure lazy transform population in the event processing */
            Gaudi::Property<bool> m_splitTrfCache{this, "splitTrfCache", false, ""};

    };
}
#endif // MuonRegSelCondAlg_h

