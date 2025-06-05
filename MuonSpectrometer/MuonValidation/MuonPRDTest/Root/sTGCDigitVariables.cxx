/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/sTGCDigitVariables.h"

#include "MuonReadoutGeometry/sTgcReadoutElement.h"

namespace MuonPRDTest {
    sTgcDigitVariables::sTgcDigitVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        PrdTesterModule(tree, "Digits_sTGC", msglvl), m_key{container_name} {}

    bool sTgcDigitVariables::declare_keys() { return declare_dependency(m_key); }
    bool sTgcDigitVariables::fill(const EventContext& ctx) {
        ATH_MSG_DEBUG("do fillsTGCDigitHitVariables()");
        const MuonGM::MuonDetectorManager* MuonDetMgr = getDetMgr(ctx);
        if (!MuonDetMgr) { return false; }
        SG::ReadHandle<sTgcDigitContainer> sTgcDigitContainer{m_key, ctx};
        if (!sTgcDigitContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve digit container " << m_key.fullKey());
            return false;
        }

        ATH_MSG_DEBUG("retrieved sTGC Digit Container with size " << sTgcDigitContainer->digit_size());

        if (sTgcDigitContainer->size() == 0) ATH_MSG_DEBUG(" sTGC Digit Container empty ");
        unsigned int n_digits{0};
        for (const sTgcDigitCollection* coll : *sTgcDigitContainer) {
            ATH_MSG_DEBUG("processing collection with size " << coll->size());
            for (unsigned int digitNum = 0; digitNum < coll->size(); digitNum++) {
                const sTgcDigit* digit = coll->at(digitNum);
                Identifier Id = digit->identify();

                const MuonGM::sTgcReadoutElement* rdoEl = MuonDetMgr->getsTgcReadoutElement(Id);
                if (!rdoEl) {
                    ATH_MSG_ERROR("sTGCDigitVariables::fillVariables() - Failed to retrieve sTGCReadoutElement for "<<idHelperSvc()->toString(Id));
                    return false;
                }
                m_NSWsTGC_dig_id.push_back(Id);
                Amg::Vector3D gpos{Amg::Vector3D::Zero()};
                Amg::Vector2D lpos(Amg::Vector2D::Zero());

                rdoEl->stripPosition(Id, lpos);
                rdoEl->surface(Id).localToGlobal(lpos, gpos, gpos);

                m_NSWsTGC_dig_globalPos.push_back(gpos);
                m_NSWsTGC_dig_localPos.push_back(lpos);
                
                m_NSWsTGC_dig_channelNumber.push_back(rdoEl->stripNumber(lpos, Id));
                if(idHelperSvc()->stgcIdHelper().channelType(Id) == sTgcIdHelper::Pad ) {
        
                    std::array<Amg::Vector2D, 4> local_pad_corners{make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
                    rdoEl->padCorners(Id,local_pad_corners);
                 
                    for(const Amg::Vector2D& local_corner : local_pad_corners) {
                        Amg::Vector3D global_corner{Amg::Vector3D::Zero()};
                        rdoEl->surface(Id).localToGlobal(local_corner, global_corner, global_corner);
                        m_NSWsTGC_dig_PadglobalCornerPos.push_back(global_corner);
                    }
                }

                m_NSWsTGC_dig_bctag.push_back(digit->bcTag());
                m_NSWsTGC_dig_time.push_back(digit->time());
                m_NSWsTGC_dig_charge.push_back(digit->charge());
                m_NSWsTGC_dig_isDead.push_back(digit->isDead());
                m_NSWsTGC_dig_isPileup.push_back(digit->isPileup());

                ++n_digits;
            }
        }
        m_NSWsTGC_nDigits = n_digits;
        ATH_MSG_DEBUG(" finished fillsTgcDigitVariables()");
        return true;
    }
}