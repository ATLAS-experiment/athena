/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

#include "GaudiKernel/PhysicalConstants.h"
#include "MuonPRDTestR4/RpcDigitVariablesR4.h"
#include "StoreGate/ReadHandle.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
namespace MuonValR4 {
    RpcDigitVariablesR4::RpcDigitVariablesR4(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        TesterModuleBase(tree, "Digits_RPC", msglvl), m_key{container_name} {}

    bool RpcDigitVariablesR4::declare_keys() { return declare_dependency(m_key); } 
    
    namespace {
    using DigitIndexMap = std::unordered_map<uint32_t, std::vector<unsigned int>>;
    // function which groups Digit indices with same ID
    DigitIndexMap buildDigitIndexMap(const RpcDigitCollection& coll) {
        DigitIndexMap indexMap;
        for (unsigned int i = 0; i < coll.size(); ++i) {
            uint32_t key = coll[i]->identify().get_identifier32().get_compact();
            indexMap[key].push_back(i);
	}
        return indexMap;
    }
    const float c = Gaudi::Units::c_light;   // in mm/ns
    const float propVelocity = 0.5 * c;
    } // anonymous namespace

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
	    auto digitMap = buildDigitIndexMap(*coll);
            for (const RpcDigit* digit: *coll) {
                Identifier Id = digit->identify();
                ATH_MSG_DEBUG("RPC Digit Offline id:  " << idHelperSvc()->toString(Id));

                const MuonGMR4::RpcReadoutElement* rdoEl = MuonDetMgr->getRpcReadoutElement(Id);
                if (!rdoEl) {
                    ATH_MSG_ERROR("RPCDigitVariablesR4::fillVariables() - Failed to retrieve RPCReadoutElement for "<<idHelperSvc()->rpcIdHelper().print_to_string(Id).c_str());
                    return false;
                }

                const int RPClayer = int(idHelperSvc()->stationIndex(Id)); // RPC[0-3] --> 0:BI, 1:BM, 2:BO, 3:BE 
                const bool isBI = RPClayer == 0;
		
		const ActsTrk::GeometryContext& geoCtx = getGeoCtx(ctx);
                const Amg::Vector3D gpos{rdoEl->stripPosition(geoCtx, Id)};
		const Amg::Transform3D g2l = rdoEl->globalToLocalTransform(geoCtx, Id);
		const Amg::Vector3D lpos3D = g2l * gpos;
                // debug variables
		int idx0 = -1;
    		int idx1 = -1;

		if (isBI) {
		    const float stripHalfLength = 0.5 * rdoEl->stripEtaLength();
		    m_RPC_dig_stripHalfLength.push_back(stripHalfLength);
		    const float tof = gpos.norm() / c; // approximate time of flight, in ns
		    float timeL = tof, timeR = tof; // local frame s.t. y=-stripLength/2 (L side) and y=stripLength/2 (R side)
		    unsigned int nL{0}, nR{0};
		    const auto& group = digitMap[Id.get_identifier32().get_compact()];
		    
		    for (const unsigned int i : group) {
		        const RpcDigit* d = (*coll)[i];
        	        if (d->stripSide()) {
			    timeL = d->time();
			    ++nL;
        		} else {
           		    timeR = d->time();
			    ++nR;
			}
		    }
		    if (nL > 1 || nR > 1) {
	                ATH_MSG_WARNING("RPC digit same-side readout multiplicity > 1 for same Identifier. "
			    << "nL (stripSide==true) = " << nL
                            << ", nR (stripSide==false) = " << nR
                            << ", Run = " << ctx.eventID().run_number()
                            << ", Event = " << ctx.eventID().event_number());
		    }
		    const float dt = timeL - timeR; // in ns
		    m_RPC_dig_timeDiff.push_back(dt);

		    float dy{0};
		    if (nL && nR) {
    			dy = 0.5 * propVelocity * dt; // in mm
		    } else if (nL) {
    			dy = propVelocity * dt - stripHalfLength;
                    } else if (nR) {
                        dy = propVelocity * dt + stripHalfLength;
		    } // else: no timing info --> dy = 0
		    if (std::abs(dy) > stripHalfLength) {
                        ATH_MSG_WARNING("Computed dy outside strip bounds. "
                            << "|dy| = " << std::abs(dy)
                            << " mm, halfLength = " << stripHalfLength
                            << " mm, globalZ = " << gpos.z()
                            << " mm, globalPhi = " << std::atan2(gpos.y(), gpos.x())
                            << " rad, Run = " << ctx.eventID().run_number()
                            << ", Event = " << ctx.eventID().event_number()
                            << ". Clamping.");
                        dy = std::clamp(dy, -stripHalfLength, stripHalfLength);
                    }
   	            const Amg::Vector3D lpos3DFix(lpos3D.x(), lpos3D.y() + dy, lpos3D.z());
		    const Amg::Vector3D gposFix = g2l.inverse() * lpos3DFix;	    
		    m_RPC_dig_localPos3DFix.push_back(lpos3DFix);
		    m_RPC_dig_globalPosFix.push_back(gposFix);

		    // debug variables
		    if (!group.empty()) {
        		idx0 = group[0];
		    }
    		    if (group.size() >= 2) {
        		idx1 = group[1];
		    }

		} else {
		    m_RPC_dig_stripHalfLength.push_back(-999);
		    m_RPC_dig_timeDiff.push_back(-999);
		    m_RPC_dig_localPos3DFix.push_back(lpos3D);
                    m_RPC_dig_globalPosFix.push_back(gpos);
		}

		m_RPC_dig_globalPos.push_back(gpos);
		m_RPC_dig_localPos3D.push_back(lpos3D);
                // m_RPC_dig_localPos.push_back(lpos);
                m_RPC_dig_time.push_back(digit->time());
                m_RPC_tot.push_back(digit->ToT());
                m_RPC_dig_id.push_back(Id);
		m_RPC_secIndex.push_back(idHelperSvc()->sector(Id));
		m_RPC_stationName.push_back(idHelperSvc()->stationNameString(Id));
		m_RPC_layerIndex.push_back(RPClayer);
		// debug branches
		m_RPC_groupIndex0.push_back(idx0);
    		m_RPC_groupIndex1.push_back(idx1);

                ++n_digits;
            }
        }
        m_RPC_nDigits = n_digits;
        ATH_MSG_DEBUG(" finished fillRpcDigitVariablesR4()");
        return true;
    }
}  // namespace MuonValR4
