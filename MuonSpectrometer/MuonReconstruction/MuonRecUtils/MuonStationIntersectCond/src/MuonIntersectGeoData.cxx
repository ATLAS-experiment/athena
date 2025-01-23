/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonStationIntersectCond/MuonIntersectGeoData.h"

#include <TString.h>  // for Form

#include "MuonCondData/MdtCondDbData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

namespace Muon {

    MuonIntersectGeoData::~MuonIntersectGeoData() = default;
    MuonIntersectGeoData::MuonIntersectGeoData() = default;
    MuonIntersectGeoData::MuonIntersectGeoData(MsgStream& log, const MuonGM::MuonDetectorManager* detMgr,
                                               const IMuonIdHelperSvc* idHelperSvc, const MdtCondDbData* dbData) :
        m_idHelperSvc{idHelperSvc}, m_detMgr{detMgr}, m_dbData{dbData} {
        m_geometry.resize(m_idHelperSvc->mdtIdHelper().detectorElement_hash_max());
        for (unsigned int n = 0; n < m_geometry.size(); ++n) {
            IdentifierHash id_hash{n};
            const MuonGM::MdtReadoutElement* mdt_ele = detMgr->getMdtReadoutElement(id_hash);
            // The Mdt intersectionGeometry relies on the first multi layer. We can skip the other layers
            if (!mdt_ele || mdt_ele->getMultilayer() != 1) continue;
            m_geometry[id_hash] = std::make_unique<Muon::MdtIntersectGeometry>(log, mdt_ele->identify(), idHelperSvc, detMgr, dbData);
        }    
    } 
    
    std::vector<std::shared_ptr<const Muon::MdtIntersectGeometry>> MuonIntersectGeoData::getStationGeometry(const Identifier& id) const {
        // get ids of geometry associated with identifier
        std::vector<Identifier> chambers = binPlusneighbours(id);

        // vector to store result
        std::vector<std::shared_ptr<const Muon::MdtIntersectGeometry>> stations;
        stations.reserve(chambers.size());
        // loop over bins, retrieve geometry
        for (const Identifier& chId : chambers) {
            if (m_dbData && !m_dbData->isGood(chId)) { continue; }
            std::shared_ptr<const MdtIntersectGeometry> chamb = getChamber(chId);
            if (chamb) stations.push_back(std::move(chamb));
        }
        return stations;
    }
    std::shared_ptr<const MdtIntersectGeometry> MuonIntersectGeoData::getChamber(const Identifier& chId) const {
         IdentifierHash hash{0};
         const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
         if (idHelper.get_detectorElement_hash(idHelper.multilayerID(chId), hash)) return nullptr;
         return hash < m_geometry.size() ? m_geometry[hash] : nullptr;
    }

    Muon::MuonStationIntersect MuonIntersectGeoData::tubesCrossedByTrack(const Identifier& id, const Amg::Vector3D& pos,
                                                                         const Amg::Vector3D& dir) const {
        std::vector<std::shared_ptr<const Muon::MdtIntersectGeometry>> stations = getStationGeometry(id);

        Muon::MuonStationIntersect::TubeIntersects tubeIntersects;
        for (std::shared_ptr<const Muon::MdtIntersectGeometry>& it : stations) {
            Muon::MuonStationIntersect intersect = it->intersection(pos, dir);
            tubeIntersects.insert(tubeIntersects.end(), intersect.tubeIntersects().begin(), intersect.tubeIntersects().end());
        }

        Muon::MuonStationIntersect intersection{std::move(tubeIntersects)};
        return intersection;
    }

    std::vector<Identifier> MuonIntersectGeoData::binPlusneighbours(const Identifier& id) const {
        std::vector<Identifier> chIds;
        int stName = m_idHelperSvc->mdtIdHelper().stationName(id);
        int stPhi = m_idHelperSvc->mdtIdHelper().stationPhi(id);
        int stEta = m_idHelperSvc->mdtIdHelper().stationEta(id);
        bool isBarrel = m_idHelperSvc->mdtIdHelper().isBarrel(id);
        auto [stEtaMin, stEtaMax] = m_idHelperSvc->mdtIdHelper().stationEtaMinMax(id);

        int chEtaLeft = stEta - 1;
        int chEtaRight = stEta + 1;

        

        // chamber with smallest eta
        if (chEtaLeft < stEtaMin) chEtaLeft = -999;

        // chamber with largest eta
        if (chEtaRight > stEtaMax) chEtaRight = -999;

        Muon::MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(id);

        // special treatment of EOS chambers
        if (chIndex == Muon::MuonStationIndex::EOS) {
            chEtaRight = -999;
            chEtaLeft = -999;
        }

        if (isBarrel) {
            // eta = 0 doesn't exist take next
            if (chEtaLeft == 0) chEtaLeft -= 1;
            if (chEtaRight == 0) chEtaRight += 1;
        } else {
            // do not combined positive and negative endcaps
            if (chEtaLeft == 0) chEtaLeft = -999;
            if (chEtaRight == 0) chEtaRight = -999;
        }

        // no neighbours for BIS8
        if (chIndex == Muon::MuonStationIndex::BIS && std::abs(stEta) == 8) {
            chEtaLeft = -999;
            chEtaRight = -999;
        }

        // BIS 8 never neighbour of a chamber
        if (chIndex == Muon::MuonStationIndex::BIS) {
            if (std::abs(chEtaLeft) == 8) chEtaLeft = -999;
            if (std::abs(chEtaRight) == 8) chEtaRight = -999;
        }
        
        if ((chIndex == Muon::MuonStationIndex::EIS || chIndex == Muon::MuonStationIndex::EIL) && !m_idHelperSvc->hasCSC()) {
            //Chambers can only be -5, -4, 4, 5
            if( std::abs(chEtaLeft) != 5 && std::abs(chEtaLeft) != 4 ) chEtaLeft = -999;
            if( std::abs(chEtaRight) != 5 && std::abs(chEtaRight) != 4 ) chEtaRight = -999;
        }

        if (chEtaLeft != -999 &&
            m_idHelperSvc->mdtIdHelper().validElement(m_idHelperSvc->mdtIdHelper().elementID(stName, chEtaLeft, stPhi)))
            chIds.push_back(m_idHelperSvc->mdtIdHelper().elementID(stName, chEtaLeft, stPhi));
        chIds.push_back(m_idHelperSvc->mdtIdHelper().elementID(id));
        if (chEtaRight != -999 &&
            m_idHelperSvc->mdtIdHelper().validElement(m_idHelperSvc->mdtIdHelper().elementID(stName, chEtaRight, stPhi)))
            chIds.push_back(m_idHelperSvc->mdtIdHelper().elementID(stName, chEtaRight, stPhi));

        // if (msgLvl(MSG::VERBOSE)) {
        //     ATH_MSG_VERBOSE(" returning chambers for id " << m_idHelperSvc->toString(id) << " ids " << chIds.size() << "   eta " <<
        //     chEtaLeft
        //                                                   << "     " << stEta << "     " << chEtaRight << " chambers: ");
        //     for (unsigned int i = 0; i < chIds.size(); ++i) ATH_MSG_VERBOSE(m_idHelperSvc->toString(chIds[i]));
        // }

        return chIds;
    }

}  // namespace Muon
