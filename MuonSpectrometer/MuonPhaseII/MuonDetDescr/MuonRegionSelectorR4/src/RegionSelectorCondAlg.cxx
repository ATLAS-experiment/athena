/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "RegionSelectorCondAlg.h"


#include <StoreGate/WriteCondHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <AthenaKernel/IOVInfiniteRange.h>

#include "Acts/Surfaces/PlanarBounds.hpp"
#include "MuonTGC_Cabling/MuonTGC_CablingSvc.h"

#include "MuonNSWCommonDecode/NSWOfflineHelper.h"
#include "FourMomUtils/xAODP4Helpers.h"

namespace MuonR4{
    StatusCode RegionSelectorCondAlg::initialize() {
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_tableKey.initialize());
        ATH_CHECK(m_alignKey.initialize());
        ATH_CHECK(m_cablingMdtKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_cablingRpcKey.initialize(SG::AllowEmpty));
        return StatusCode::SUCCESS;
    }
     template <typename Key_t,
                typename... OtherKey_t>
    StatusCode RegionSelectorCondAlg::addDependency(const EventContext& ctx,
                                SG::WriteCondHandle<IRegSelLUTCondData>& writeHandle,
                               const SG::ReadCondHandleKey<Key_t>& key,
                               OtherKey_t... others) const {
        if (!key.empty()) {
            SG::ReadCondHandle readHandle{key, ctx};
            if (!readHandle.isValid()){
                ATH_MSG_FATAL("Failed to retrieve "<<key.fullKey()<<".");
                return StatusCode::FAILURE;
            }
            writeHandle.addDependency(readHandle);
        }
        if constexpr(sizeof...(others) > 0) {
            ATH_CHECK(addDependency(ctx, writeHandle, others...));
        }
        return StatusCode::SUCCESS;
    }

    const MuonIdHelper& RegionSelectorCondAlg::getIdHelper(const ActsTrk::DetectorType type) const {
        switch (type) {
            using enum ActsTrk::DetectorType;
            case Mdt: 
                return m_idHelperSvc->mdtIdHelper();
            case Rpc:
                return m_idHelperSvc->rpcIdHelper();
            case Tgc:
                return m_idHelperSvc->tgcIdHelper();
            case Mm:
                return m_idHelperSvc->mmIdHelper();
            case sTgc:
                return m_idHelperSvc->stgcIdHelper();
            default:
                THROW_EXCEPTION("Unknown detector type "<<ActsTrk::to_string(type));
        }
    }

    std::set<std::uint32_t> RegionSelectorCondAlg::getRobIDs(const EventContext& ctx,
                                                             const Identifier& moduleID) const {
        
        switch(m_idHelperSvc->technologyIndex(moduleID)) {
            using enum Muon::MuonStationIndex::TechnologyIndex;
            case MDT: {
                const MuonMDT_CablingMap* cabling{};
                if (!SG::get(cabling, m_cablingMdtKey, ctx).isSuccess()) {
                    THROW_EXCEPTION("Failed to load Mdt cabling "<<m_cablingMdtKey.fullKey());
                }
                return {cabling->getROBId(m_idHelperSvc->moduleHash(moduleID), msgStream())};
            } case RPC: {
                const Muon::RpcCablingMap* cabling{};
                if (!SG::get(cabling, m_cablingRpcKey, ctx).isSuccess()) {
                    THROW_EXCEPTION("Failed to load Rpc cabling "<<m_cablingRpcKey.fullKey());
                }
                return {cabling->getROBId(m_idHelperSvc->moduleHash(moduleID), msgStream())};
            } case TGC: {
                SmartIF<MuonTGC_CablingSvc> cabling{Gaudi::svcLocator()->service("MuonTGC_CablingSvc")};
                if (!cabling.isValid()) {
                    THROW_EXCEPTION("Failed to load Tgc cabling");
                }
                int subDetectorId{}, rodId{};
                cabling->getReadoutIDfromElementID(moduleID, subDetectorId, rodId);
                return {static_cast<std::uint32_t>(((0x0ff) & subDetectorId)<<16 | rodId)};
            } case MM:
              case STGC:{
                Muon::nsw::helper::NSWOfflineRobId robIdHelper{
                        m_idHelperSvc->stationNameString(moduleID),
                        static_cast<int8_t>(m_idHelperSvc->stationEta(moduleID)),
                        static_cast<uint8_t>(m_idHelperSvc->stationPhi(moduleID))};
                return std::set<std::uint32_t>(robIdHelper.get_ids().begin(), robIdHelper.get_ids().end());
            } default: {
                THROW_EXCEPTION("Invalid identifier "<<m_idHelperSvc->toStringChamber(moduleID));
            }
        }
        return {};
    }
            
    StatusCode RegionSelectorCondAlg::execute(const EventContext& ctx) const {
        SG::WriteCondHandle writeHandle{m_tableKey, ctx};
        if (writeHandle.isValid()) {
            ATH_MSG_DEBUG("Region selector "<<m_tableKey<<" is still valid");
            return StatusCode::SUCCESS;
        }
        writeHandle.addDependency(IOVInfiniteRange::infiniteRunLB());
        ATH_CHECK(addDependency(ctx, writeHandle, m_alignKey, m_cablingMdtKey, m_cablingRpcKey));

        auto writeCdo = std::make_unique<RegSelSiLUT>();
        const ActsTrk::DetectorAlignStore* alignDeltas{nullptr};
        ATH_CHECK(SG::get(alignDeltas, m_alignKey, ctx));

        ActsTrk::GeometryContext gctx{};
        gctx.setStore(MuonGMR4::copyDeltas(*alignDeltas));

        const MuonIdHelper& idHelper{getIdHelper(alignDeltas->detType)};

        /** @brief Temporary auxiliary struct to cache the readout element's 
         *         dimension in z, r & phi */
        struct TempSelTable {
            std::optional<double> centralPhi{};
            double zMin{std::numeric_limits<double>::max()}; 
            double zMax{-std::numeric_limits<double>::max()};
            double rMin{std::numeric_limits<double>::max()}; 
            double rMax{-std::numeric_limits<double>::max()};
            double dPhiMin{std::numeric_limits<double>::max()}; 
            double dPhiMax{-std::numeric_limits<double>::max()};

            void update(const Amg::Vector3D& v) {
                const double dPhi = xAOD::P4Helpers::deltaPhi(v.phi(), centralPhi.value_or(0.));
                rMin = std::min(rMin, v.perp());   rMax = std::max(rMax, v.perp());
                zMin = std::min(zMin, v.z());      zMax = std::max(zMax, v.z());
                dPhiMin = std::min(dPhiMin, dPhi); dPhiMax = std::max(dPhiMax, dPhi);
            }
    
        };
        std::vector<TempSelTable> luts(idHelper.module_hash_max());
        IdentifierHash modHash{};
        /** Fill the geometric part of the reg sel table */
        for (auto det_itr = idHelper.detectorElement_begin(); 
                  det_itr != idHelper.detectorElement_end(); ++det_itr) {
            
            const MuonGMR4::MuonReadoutElement* reEle = m_detMgr->getReadoutElement(*det_itr);
            if (!reEle) {
                ATH_MSG_WARNING(__LINE__<<" - Failed to fetch "<<m_idHelperSvc->toStringDetEl(*det_itr)<<".");
                continue;
            }
            const Acts::Surface& surface{reEle->surface()};

            const double halfTck = 0.5*reEle->thickness();

            std::vector<Amg::Vector3D> localVertices{};
            ATH_MSG_VERBOSE(__LINE__<<" - Fetch edge points from surface: "<<m_idHelperSvc->toStringDetEl(reEle->identify()));
            std::ranges::for_each(static_cast<const Acts::PlanarBounds&>(surface.bounds()).vertices(), 
                    [&](const Amg::Vector2D& v){
                        ATH_MSG_VERBOSE(__LINE__<<" - Local vertex: "<<Amg::toString(v)<<", half thickness: "<<halfTck);
                        localVertices.emplace_back(v.x(), v.y(), halfTck);
                        localVertices.emplace_back(v.x(), v.y(), -halfTck);
                    });
            ATH_MSG_VERBOSE(__LINE__<<" - Fetched "<<localVertices.size()<<" vertices.");
            const Amg::Transform3D& loc2Glob{surface.transform(gctx.context())};
            idHelper.get_module_hash(*det_itr, modHash);
            auto& lut = luts.at(modHash);
            if (!lut.centralPhi) {
                lut.centralPhi = loc2Glob.translation().phi();
            }
            for (const Amg::Vector3D& locVtx : localVertices) {
                const Amg::Vector3D globVtx = loc2Glob * locVtx;
                ATH_MSG_VERBOSE(__LINE__<<" - Expand LUT with vertex "
                        <<std::format("{:.2f}/{:.2f}/{:.2f}", globVtx.perp(), globVtx.z(), globVtx.phi()));
                lut.update(globVtx);
            }
        }
        const auto modCtx = idHelper.module_context();
        /** Now we need to combine it with the ROB id */
        for (auto module_itr = idHelper.module_begin();
                  module_itr != idHelper.module_end(); ++module_itr) {
            idHelper.get_module_hash(*module_itr, modHash);
            const auto& lut = luts.at(modHash);

            if(!lut.centralPhi) {
                continue;
            }

            ExpandedIdentifier exp_id{};
            if (idHelper.get_expanded_id(*module_itr, exp_id, &modCtx)) {
                ATH_MSG_DEBUG(__LINE__<<" - Failed retrieving ExpandedIdentifier for PRD Identifier = " 
                        << m_idHelperSvc->toString(*module_itr) << ". Skipping to the next PRD.");
              continue;
            }
            const int detid   = ( exp_id[2]<0 ? -1 : 1 );
            const int layerid = exp_id[1]+1;

            const double phiMin = lut.centralPhi.value_or(0) + lut.dPhiMin;
            const double phiMax = lut.centralPhi.value_or(0) + lut.dPhiMax;
            

            for (const std::uint32_t robID : getRobIDs(ctx, *module_itr)) {
                RegSelModule m{lut.zMin, lut.zMax, lut.rMin, lut.rMax, 
                               phiMin, phiMax, layerid, detid, robID, modHash};
                ATH_MSG_VERBOSE(__LINE__<<" - Insert new entry "<<m_idHelperSvc->chamberNameString(*module_itr)
                                <<", "<<m);
                writeCdo->addModule(m);
            }
        }

        if (m_printTable) {
            if (const auto *lut = dynamic_cast<const RegSelSiLUT*>(writeCdo.get())) {
                lut->write(std::format("{:}.map", name()));
            }
        }

        ATH_CHECK(writeHandle.record(std::make_unique<IRegSelLUTCondData>(std::move(writeCdo))));
        return StatusCode::SUCCESS;
    }
}