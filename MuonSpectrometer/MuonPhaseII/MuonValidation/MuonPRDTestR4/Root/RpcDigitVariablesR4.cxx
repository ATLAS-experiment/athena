/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTestR4/RpcDigitVariablesR4.h"
#include "StoreGate/ReadHandle.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
namespace MuonValR4 {
    RpcDigitVariablesR4::RpcDigitVariablesR4(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        TesterModuleBase(tree, "Digits_RPC", msglvl), m_key{container_name} {}

    bool RpcDigitVariablesR4::declare_keys() { return declare_dependency(m_key); }
    bool RpcDigitVariablesR4::fill(const EventContext& ctx) {
        ATH_MSG_DEBUG("do fillMDTSimHitVariables()");
        const MuonGMR4::MuonDetectorManager* MuonDetMgr = getDetMgr();
        if (!MuonDetMgr) { return false; }
        SG::ReadHandle<RpcDigitContainer> RpcDigitContainer{m_key, ctx};
        if (!RpcDigitContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve digit container " << m_key.fullKey());
            return false;
        }

        ATH_MSG_DEBUG("retrieved RPC Digit Container with size " << RpcDigitContainer->digit_size());

        if (RpcDigitContainer->size() == 0) ATH_MSG_DEBUG(" RPC Digit Container empty ");
        unsigned int n_digits{0};
        for (const RpcDigitCollection* coll : *RpcDigitContainer) {
            ATH_MSG_DEBUG("processing collection with size " << coll->size());
            for (const RpcDigit* digit: *coll) {
                Identifier Id = digit->identify();

                ATH_MSG_DEBUG("RPC Digit Offline id:  " << idHelperSvc()->toString(Id));

                const MuonGMR4::RpcReadoutElement* rdoEl = MuonDetMgr->getRpcReadoutElement(Id);
                if (!rdoEl) {
                    ATH_MSG_ERROR("RPCDigitVariablesR4::fillVariables() - Failed to retrieve PRCReadoutElement for "<<idHelperSvc()->rpcIdHelper().print_to_string(Id).c_str());
                    return false;
                }
		
		const ActsTrk::GeometryContext& geoCtx = getGeoCtx(ctx);
                const Amg::Vector3D gpos{rdoEl->stripPosition(geoCtx, Id)};
                // Amg::Vector2D lpos{Amg::Vector2D::Zero()};
		// const Acts::Surface& surf = rdoEl->surface(rdoEl->measurementHash(Id));                
		// surf.globalToLocal(gpos, Amg::Vector3D::Zero(), lpos);
		const Amg::Vector3D lpos3D = rdoEl->globalToLocalTransform(geoCtx, Id) * gpos;
		const Amg::Vector2D lpos(lpos3D.x(), lpos3D.y());


		m_RPC_dig_globalPos.push_back(gpos);
		m_RPC_dig_localPos3D.push_back(lpos3D);
                m_RPC_dig_localPos.push_back(lpos);
                m_RPC_dig_time.push_back(digit->time());
                m_RPC_tot.push_back(digit->ToT());
                m_RPC_dig_id.push_back(Id);
		m_RPC_secIndex.push_back(idHelperSvc()->sector(Id));
		m_RPC_secName.push_back(idHelperSvc()->stationNameString(Id));

                ++n_digits;
            }
        }
        m_RPC_nDigits = n_digits;
        ATH_MSG_DEBUG(" finished fillRpcDigitVariables()");
        return true;
    }
}  // namespace MuonValR4
