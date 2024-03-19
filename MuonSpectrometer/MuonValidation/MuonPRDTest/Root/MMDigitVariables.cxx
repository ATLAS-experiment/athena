/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/MMDigitVariables.h"

#include "MuonReadoutGeometry/MMReadoutElement.h"

namespace MuonPRDTest {
    MMDigitVariables::MMDigitVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        PrdTesterModule(tree, "Digits_MM", true, msglvl), m_key{container_name} {}

    bool MMDigitVariables::declare_keys() { return declare_dependency(m_key); }
    bool MMDigitVariables::fill(const EventContext& ctx) {
        ATH_MSG_DEBUG("do fillMMDigitHitVariables()");
        const MuonGM::MuonDetectorManager* MuonDetMgr = getDetMgr(ctx);
        if (!MuonDetMgr) { return false; }
        SG::ReadHandle<MmDigitContainer> MMDigitContainer{m_key, ctx};
        if (!MMDigitContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve digit container " << m_key.fullKey());
            return false;
        }

        ATH_MSG_DEBUG("retrieved MM Digit Container with size " << MMDigitContainer->digit_size());

        if (MMDigitContainer->size() == 0) ATH_MSG_DEBUG(" MM Digit Container empty ");
        unsigned int n_digits{0};
        for (const MmDigitCollection* coll : *MMDigitContainer) {
            ATH_MSG_DEBUG("processing collection with size " << coll->size());
            for (const MmDigit* digit : *coll) {
                Identifier Id = digit->identify();
                const MuonGM::MMReadoutElement* rdoEl = MuonDetMgr->getMMReadoutElement(Id);
                if (!rdoEl) {
                    ATH_MSG_ERROR("MMDigitVariables::fillVariables() - Failed to retrieve MMReadoutElement for "<<idHelperSvc()->mmIdHelper().print_to_string(Id).c_str());
                    return false;
                }
                Amg::Vector2D cr_strip_pos{Amg::Vector2D::Zero()};
                if ( !rdoEl->stripPosition(Id, cr_strip_pos) ) {
                    ATH_MSG_WARNING("MMDigitVariables: failed to associate a valid local position for (chip response) strip n. " 
                                    << "; associated positions will be set to 0.0.");
                    continue;
                }
                m_NSWMM_dig_stripLpos.push_back(cr_strip_pos);
                // asking the detector element to transform this local to the global position
                Amg::Vector3D cr_strip_gpos(Amg::Vector3D::Zero());
                rdoEl->surface(Id).localToGlobal(cr_strip_pos, Amg::Vector3D::Zero(), cr_strip_gpos);
                m_NSWMM_dig_stripGpos.push_back(cr_strip_gpos);
                ++n_digits;
            }
        }
        m_NSWMM_nDigits = n_digits;
        ATH_MSG_DEBUG(" finished fillMMDigitVariables()");
        return true;
    }
}