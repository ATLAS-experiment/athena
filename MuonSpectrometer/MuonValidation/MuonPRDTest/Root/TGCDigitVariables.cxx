/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/TGCDigitVariables.h"

#include "MuonReadoutGeometry/TgcReadoutElement.h"
namespace MuonPRDTest {
    TgcDigitVariables::TgcDigitVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        PrdTesterModule(tree, "Digits_TGC", msglvl), m_key{container_name} {}

    bool TgcDigitVariables::fill(const EventContext& ctx) {
        ATH_MSG_DEBUG("do fillTGCSimHitVariables()");
        const MuonGM::MuonDetectorManager* MuonDetMgr = getDetMgr(ctx);
        if (!MuonDetMgr) { return false; }
        SG::ReadHandle<TgcDigitContainer> TgcDigitContainer{m_key, ctx};
        if (!TgcDigitContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve digit container " << m_key.fullKey());
            return false;
        }

        ATH_MSG_DEBUG("retrieved TGC Digit Container with size " << TgcDigitContainer->digit_size());

        if (TgcDigitContainer->size() == 0) ATH_MSG_DEBUG(" TGC Digit Container empty ");
        unsigned int n_digits{0};
        for (const TgcDigitCollection* coll : *TgcDigitContainer) {
            ATH_MSG_DEBUG("processing collection with size " << coll->size());
            for (const TgcDigit* digit : *coll) {
                Identifier Id = digit->identify();
                ATH_MSG_DEBUG("TGC Digit Offline id:  " << idHelperSvc()->toString(Id));

                const MuonGM::TgcReadoutElement* rdoEl = MuonDetMgr->getTgcReadoutElement(Id);
                if (!rdoEl) {
                    ATH_MSG_ERROR("TGCDigitVariables::fillVariables() - Failed to retrieve TGCReadoutElement for "<<idHelperSvc()->tgcIdHelper().print_to_string(Id).c_str());
                    return false;
                }

                Amg::Vector3D gpos{Amg::Vector3D::Zero()};
                Amg::Vector2D lpos{Amg::Vector2D::Zero()};

                if (!rdoEl->stripPosition(Id, lpos)) {                   
                    continue;
                }
                rdoEl->surface(Id).localToGlobal(lpos, gpos, gpos);
                m_TGC_dig_globalPos.push_back(gpos);
                m_TGC_dig_localPos.push_back(lpos);
                m_TGC_dig_bcId.push_back(digit->bcTag());
                m_TGC_dig_id.push_back(Id);
                ++n_digits;
            }
        }
        m_TGC_nDigits = n_digits;
        ATH_MSG_DEBUG(" finished fillTgcDigitVariables()");
        return true;
    }
    bool TgcDigitVariables::declare_keys() { return  declare_dependency(m_key); }
}