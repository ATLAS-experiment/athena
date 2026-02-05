/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCableASDToPP.h"

#include <fstream>
#include <sstream>

#include "GaudiKernel/StatusCode.h"
#include "MuonTGC_Cabling/TGCChannelASDOut.h"
#include "MuonTGC_Cabling/TGCChannelPPIn.h"
#include "MuonTGC_Cabling/TGCDatabaseASDToPP.h"

namespace MuonTGC_Cabling {

TGCCableASDToPP::TGCCableASDToPP(const std::string& filename)
    : TGCCable(TGCCable::ASDToPP), m_tgcCablingDbTool("TGCCablingDbTool") {
    initialize(filename);
}

TGCCableASDToPP::~TGCCableASDToPP() {}

void TGCCableASDToPP::initialize(const std::string& filename) {
    m_commonDb[TGCId::Forward][TGCId::WD] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FWD");
    m_commonDb[TGCId::Forward][TGCId::SD] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FSD");
    m_commonDb[TGCId::Forward][TGCId::WT] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FWT");
    m_commonDb[TGCId::Forward][TGCId::ST] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FST");

    m_commonDb[TGCId::Endcap][TGCId::WD] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "EWD");
    m_commonDb[TGCId::Endcap][TGCId::SD] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "ESD");
    m_commonDb[TGCId::Endcap][TGCId::WT] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "EWT");
    m_commonDb[TGCId::Endcap][TGCId::ST] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "EST");

    m_commonDb[TGCId::Forward][TGCId::WI] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FWI");
    m_commonDb[TGCId::Forward][TGCId::SI] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FSI");
    m_commonDb[TGCId::Endcap][TGCId::WI] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "EWI");
    m_commonDb[TGCId::Endcap][TGCId::SI] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "ESI");

    for (int side = 0; side < TGCId::MaxSideType; side++) {
        for (int sector = 0; sector < TGCId::NUM_FORWARD_SECTOR; sector++) {
            m_FWDdb[side][sector] = m_commonDb[TGCId::Forward][TGCId::WD];
            m_FSDdb[side][sector] = m_commonDb[TGCId::Forward][TGCId::SD];
            m_FWTdb[side][sector] = m_commonDb[TGCId::Forward][TGCId::WT];
            m_FSTdb[side][sector] = m_commonDb[TGCId::Forward][TGCId::ST];
        }
        for (int sector = 0; sector < TGCId::NUM_ENDCAP_SECTOR; sector++) {
            m_EWDdb[side][sector] = m_commonDb[TGCId::Endcap][TGCId::WD];
            m_ESDdb[side][sector] = m_commonDb[TGCId::Endcap][TGCId::SD];
            m_EWTdb[side][sector] = m_commonDb[TGCId::Endcap][TGCId::WT];
            m_ESTdb[side][sector] = m_commonDb[TGCId::Endcap][TGCId::ST];
        }
        for (int sector = 0; sector < TGCId::NUM_INNER_SECTOR; sector++) {
            m_FWIdb[side][sector] = m_commonDb[TGCId::Forward][TGCId::WI];
            m_FSIdb[side][sector] = m_commonDb[TGCId::Forward][TGCId::SI];
            m_EWIdb[side][sector] = m_commonDb[TGCId::Endcap][TGCId::WI];
            m_ESIdb[side][sector] = m_commonDb[TGCId::Endcap][TGCId::SI];
        }
    }
}

StatusCode TGCCableASDToPP::updateDatabase() {
    if (m_tgcCablingDbTool.retrieve().isFailure()) {
        return StatusCode::FAILURE;
    }

    StatusCode sc = m_tgcCablingDbTool->readASD2PP_DIFF_12FromText();
    if (!sc.isSuccess()) {
        return StatusCode::SUCCESS;
    }

    std::vector<std::string>* tmp_ASD2PP_DIFF_12 =
        m_tgcCablingDbTool->giveASD2PP_DIFF_12();
    if (!tmp_ASD2PP_DIFF_12) {
        return StatusCode::FAILURE;
    }

    m_ASD2PP_DIFF_12.clear();
    // Truncation saves initialization CPU time of about 30 ms.
    for (const std::string& s : *tmp_ASD2PP_DIFF_12) {
        char letter = s.at(0);
        if (letter == '/' || letter == '*') {
            continue;
        }
        m_ASD2PP_DIFF_12.push_back(s);
    }
    delete tmp_ASD2PP_DIFF_12;
    tmp_ASD2PP_DIFF_12 = nullptr;

    for (int side = 0; side < TGCId::MaxSideType; side++) {
        for (int sector = 0; sector < TGCId::NUM_FORWARD_SECTOR; sector++) {
            StatusCode sc = updateIndividualDatabase(side, sector, "FWD",
                                                     m_FWDdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }

            sc = updateIndividualDatabase(side, sector, "FSD",
                                          m_FSDdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
            sc = updateIndividualDatabase(side, sector, "FWT",
                                          m_FWTdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
            sc = updateIndividualDatabase(side, sector, "FST",
                                          m_FSTdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
        }
        for (int sector = 0; sector < TGCId::NUM_ENDCAP_SECTOR; sector++) {
            StatusCode sc = updateIndividualDatabase(side, sector, "EWD",
                                                     m_EWDdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
            sc = updateIndividualDatabase(side, sector, "ESD",
                                          m_ESDdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
            sc = updateIndividualDatabase(side, sector, "EWT",
                                          m_EWTdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
            sc = updateIndividualDatabase(side, sector, "EST",
                                          m_ESTdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
        }
        for (int sector = 0; sector < TGCId::NUM_INNER_SECTOR; sector++) {
            StatusCode sc = updateIndividualDatabase(side, sector, "EWI",
                                                     m_EWIdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
            sc = updateIndividualDatabase(side, sector, "ESI",
                                          m_ESIdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
            sc = updateIndividualDatabase(side, sector, "FWI",
                                          m_FWIdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
            sc = updateIndividualDatabase(side, sector, "FSI",
                                          m_FSIdb[side][sector]);
            if (!sc.isSuccess()) {
                return sc;
            }
        }
    }

    return StatusCode::SUCCESS;
}

StatusCode TGCCableASDToPP::getUpdateInfo(const int side, const int sector,
                                          const std::string& blockname,
                                          std::vector<std::vector<int>>& info) {
    // clear info
    info.clear();

    std::vector<std::string>::const_iterator it = m_ASD2PP_DIFF_12.begin();
    std::vector<std::string>::const_iterator it_e = m_ASD2PP_DIFF_12.end();
    int size = 0;

    // search block name
    while (it != it_e) {
        const std::string& buf = (*it);
        ++it;
        char firstl = buf.at(0);
        if (firstl == '/' || firstl == '*') {
            continue;
        }
        if (buf.compare(0, blockname.size(), blockname) == 0) {
            std::istringstream line(buf);
            std::string temp;
            line >> temp >> size;
            break;
        }
    }

    // loop over entries of specified block
    while (it != it_e) {
        const std::string& buf = (*it);
        ++it;
        char firstl = buf.at(0);
        if (firstl == '/' || firstl == '*') {
            continue;
        }
        if (firstl == 'E' || firstl == 'F') {
            break;
        }
        std::istringstream line(buf);
        std::vector<int> entry;
        int t_side, t_sector;
        line >> t_side;
        line >> t_sector;
        bool isOK = false;
        if ((t_side == side) && (t_sector == sector)) {
            for (int i = 2; i < 8; i++) {
                int temp = -1;
                if (line >> temp) {
                    entry.push_back(temp);
                } else {
                    break;
                }
                isOK = (i == 7);
            }
            if (isOK) {
                info.push_back(std::move(entry));
            }
        }
    }

    return StatusCode::SUCCESS;
}

TGCDatabaseASDToPP* TGCCableASDToPP::getDatabase(const int side,
                                                 const int region,
                                                 const int sector,
                                                 const int module) const {
    if (side < 0 || side >= TGCId::MaxSideType) {
        return nullptr;
    }
    if (sector < 0) {
        return nullptr;
    }

    TGCDatabaseASDToPP* db = nullptr;
    if (region == TGCId::Endcap) {
        switch (module) {
            case TGCId::WD:
                if (sector < TGCId::NUM_ENDCAP_SECTOR) {
                    db = m_EWDdb[side][sector].get();
                }
                break;
            case TGCId::SD:
                if (sector < TGCId::NUM_ENDCAP_SECTOR) {
                    db = m_ESDdb[side][sector].get();
                }
                break;
            case TGCId::WT:
                if (sector < TGCId::NUM_ENDCAP_SECTOR) {
                    db = m_EWTdb[side][sector].get();
                }
                break;
            case TGCId::ST:
                if (sector < TGCId::NUM_ENDCAP_SECTOR) {
                    db = m_ESTdb[side][sector].get();
                }
                break;
            case TGCId::WI:
                if (sector < TGCId::NUM_INNER_SECTOR) {
                    db = m_EWIdb[side][sector].get();
                }
                break;
            case TGCId::SI:
                if (sector < TGCId::NUM_INNER_SECTOR) {
                    db = m_ESIdb[side][sector].get();
                }
                break;
            default:
                break;
        }
    } else if (region == TGCId::Forward) {
        switch (module) {
            case TGCId::WD:
                if (sector < TGCId::NUM_FORWARD_SECTOR) {
                    db = m_FWDdb[side][sector].get();
                }
                break;
            case TGCId::SD:
                if (sector < TGCId::NUM_FORWARD_SECTOR) {
                    db = m_FSDdb[side][sector].get();
                }
                break;
            case TGCId::WT:
                if (sector < TGCId::NUM_FORWARD_SECTOR) {
                    db = m_FWTdb[side][sector].get();
                }
                break;
            case TGCId::ST:
                if (sector < TGCId::NUM_FORWARD_SECTOR) {
                    db = m_FSTdb[side][sector].get();
                }
                break;
            case TGCId::WI:
                if (sector < TGCId::NUM_INNER_SECTOR) {
                    db = m_FWIdb[side][sector].get();
                }
                break;
            case TGCId::SI:
                if (sector < TGCId::NUM_INNER_SECTOR) {
                    db = m_FSIdb[side][sector].get();
                }
                break;
            default:
                break;
        }
    }
    return db;
}

// reverse layers in Forward sector
const int TGCCableASDToPP::s_stripForward[] = {2, 1, 0, 4, 3, 6, 5, 8, 7};

TGCChannelId* TGCCableASDToPP::getChannel(const TGCChannelId* channelId,
                                          bool orChannel) const {
    if (channelId) {
        if (channelId->getChannelIdType() ==
            TGCChannelId::ChannelIdType::ASDOut) {
            return getChannelOut(channelId, orChannel);
        }
        if (channelId->getChannelIdType() ==
            TGCChannelId::ChannelIdType::PPIn) {
            return getChannelIn(channelId, orChannel);
        }
    }
    return nullptr;
}

TGCChannelId* TGCCableASDToPP::getChannelIn(const TGCChannelId* ppin,
                                            bool orChannel) const {
    if (orChannel) {
        return nullptr;
    }
    if (ppin->isValid() == false) {
        return nullptr;
    }

    TGCDatabaseASDToPP* databaseP =
        getDatabase(ppin->getSideType(), ppin->getRegionType(),
                    ppin->getSector(), ppin->getModuleType());

    if (databaseP == nullptr) {
        return nullptr;
    }

    int indexOut[TGCDatabaseASDToPP::NIndexOut] = {
        ppin->getId(), ppin->getBlock(), ppin->getChannel()};
    int i = databaseP->getIndexDBOut(indexOut);
    if (i < 0) {
        return nullptr;
    }

    // ASD2PP.db is Backward connection
    int layer = databaseP->getEntry(i, 0);
    if (ppin->isStrip()) {
        if (!(ppin->isBackward())) {
            layer = s_stripForward[layer];
        }
    }
    int offset = (ppin->isWire()) ? 4 : 0;
    int channel = databaseP->getEntry(i, 2 + offset);

    // Endcap Triplet chamberId start from 1 in ASDOut
    int chamber = databaseP->getEntry(i, 1);
    if (ppin->isEndcap() && ppin->isTriplet()) {
        chamber = chamber + 1;
    }
    TGCChannelASDOut* asdout = new TGCChannelASDOut(
        ppin->getSideType(), ppin->getSignalType(), ppin->getRegionType(),
        ppin->getSector(), layer, chamber, channel);

    return asdout;
}

TGCChannelId* TGCCableASDToPP::getChannelOut(const TGCChannelId* asdout,
                                             bool orChannel) const {
    if (orChannel) {
        return nullptr;
    }
    if (asdout->isValid() == false) {
        return nullptr;
    }

    const bool asdoutisStrip = asdout->isStrip();
    const bool asdoutisBackward = asdout->isBackward();
    const bool asdoutisEndcap = asdout->isEndcap();
    const bool asdoutisTriplet = asdout->isTriplet();
    const int asdoutLayer = asdout->getLayer();
    const int asdoutChamber = asdout->getChamber();
    const int asdoutChannel = asdout->getChannel();

    TGCDatabaseASDToPP* databaseP =
        getDatabase(asdout->getSideType(), asdout->getRegionType(),
                    asdout->getSector(), asdout->getModuleType());

    if (databaseP == nullptr) {
        return nullptr;
    }

    TGCChannelPPIn* ppin = nullptr;
    const int MaxEntry = databaseP->getMaxEntry();
    for (int i = 0; i < MaxEntry; i++) {
        // ASD2PP.db is Backward connection
        int layer = asdoutLayer;
        if (asdoutisStrip) {
            if (!asdoutisBackward) {
                layer = s_stripForward[layer];
            }
        }

        int elecChannel = asdoutChannel;

        // Endcap Triplet chamberId start from 1 in ASDOut
        int chamber = asdoutChamber;
        if (asdoutisEndcap && asdoutisTriplet) {
            chamber = chamber - 1;
        }
        int offset = (asdout->isWire()) ? 4 : 0;
        if (databaseP->getEntry(i, 0) == layer &&
            databaseP->getEntry(i, 1) == chamber &&
            databaseP->getEntry(i, 2 + offset) == elecChannel) {
            int id = databaseP->getEntry(i, 3);
            int block = databaseP->getEntry(i, 4);
            int channel = databaseP->getEntry(i, 5);

            ppin = new TGCChannelPPIn(asdout->getSideType(),
                                      asdout->getModuleType(),
                                      asdout->getRegionType(),
                                      asdout->getSector(), id, block, channel);
            break;
        }
    }
    return ppin;
}

StatusCode TGCCableASDToPP::updateIndividualDatabase(
    const int side, const int sector, const std::string& blockname,
    std::shared_ptr<TGCDatabaseASDToPP>& database) {
    if (!database) {
        return StatusCode::FAILURE;
    }
    std::vector<std::vector<int>> info;
    StatusCode sc = getUpdateInfo(side, sector, blockname, info);
    if (!sc.isSuccess()) {
        return sc;
    }

    size_t info_size = info.size();
    if (!info_size) {
        return StatusCode::SUCCESS;
    }

    if (database->isCommon()) {
        database.reset(new TGCDatabaseASDToPP(
            *database,
            false));  // false means this database is not commonly used.
        if (!database) {
            return StatusCode::FAILURE;
        }
    }

    for (size_t i = 0; i < info_size; i++) {
        database->update(info[i]);
    }

    return StatusCode::SUCCESS;
}

}  // namespace MuonTGC_Cabling
