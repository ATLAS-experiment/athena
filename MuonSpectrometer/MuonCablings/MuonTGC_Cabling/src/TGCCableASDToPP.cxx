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

TGCCableASDToPP::TGCCableASDToPP(const std::string& filename,
                                 const std::string& diffFile)
    : TGCCable{TGCCable::ASDToPP} {
    initialize(filename, diffFile);
}

TGCCableASDToPP::~TGCCableASDToPP() = default;

void TGCCableASDToPP::initialize(const std::string& filename,
                                 const std::string& diffFile) {
    m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::WD] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FWD");
    m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::SD] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FSD");
    m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::WT] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FWT");
    m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::ST] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FST");

    m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::WD] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "EWD");
    m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::SD] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "ESD");
    m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::WT] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "EWT");
    m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::ST] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "EST");

    m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::WI] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FWI");
    m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::SI] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "FSI");
    m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::WI] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "EWI");
    m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::SI] =
        std::make_shared<TGCDatabaseASDToPP>(filename, "ESI");

    for (int side = 0; side < +TGCId::SideType::MaxSideType; side++) {
        for (int sector = 0; sector < TGCId::NUM_FORWARD_SECTOR; sector++) {
            m_FWDdb[side][sector] = m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::WD];
            m_FSDdb[side][sector] = m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::SD];
            m_FWTdb[side][sector] = m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::WT];
            m_FSTdb[side][sector] = m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::ST];
        }
        for (int sector = 0; sector < TGCId::NUM_ENDCAP_SECTOR; sector++) {
            m_EWDdb[side][sector] = m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::WD];
            m_ESDdb[side][sector] = m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::SD];
            m_EWTdb[side][sector] = m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::WT];
            m_ESTdb[side][sector] = m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::ST];
        }
        for (int sector = 0; sector < TGCId::NUM_INNER_SECTOR; sector++) {
            m_FWIdb[side][sector] = m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::WI];
            m_FSIdb[side][sector] = m_commonDb[+TGCId::RegionType::Forward][+TGCId::ModuleType::SI];
            m_EWIdb[side][sector] = m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::WI];
            m_ESIdb[side][sector] = m_commonDb[+TGCId::RegionType::Endcap][+TGCId::ModuleType::SI];
        }
    }
    updateDatabase(diffFile);
}

void TGCCableASDToPP::updateDatabase(const std::string& diffFile) {

    std::vector<std::string> fileContent{};
    std::ifstream inASDToPP;
    inASDToPP.open(diffFile);
    if (inASDToPP.bad()) {
        throw std::runtime_error("Failed to open " + diffFile);
        return;
    }

    std::string buf{};
    // Copy database into m_ASD2PP_DIFF_12
    while (getline(inASDToPP, buf)) {
        char letter = buf.at(0);
        // Truncation saves initialization CPU time of about 30 ms.
        if (letter == '/' || letter == '*') {
            continue;
        }

        fileContent.push_back(buf);
    }

    for (int side = 0; side < +TGCId::SideType::MaxSideType; side++) {
        for (int sector = 0; sector < TGCId::NUM_FORWARD_SECTOR; sector++) {
            updateIndividualDatabase(side, sector, fileContent, "FWD",
                                     m_FWDdb[side][sector]);
            updateIndividualDatabase(side, sector, fileContent, "FSD",
                                     m_FSDdb[side][sector]);
            updateIndividualDatabase(side, sector, fileContent, "FWT",
                                     m_FWTdb[side][sector]);
            updateIndividualDatabase(side, sector, fileContent, "FST",
                                     m_FSTdb[side][sector]);
        }
        for (int sector = 0; sector < TGCId::NUM_ENDCAP_SECTOR; sector++) {
            updateIndividualDatabase(side, sector, fileContent, "EWD",
                                     m_EWDdb[side][sector]);
            updateIndividualDatabase(side, sector, fileContent, "ESD",
                                     m_ESDdb[side][sector]);
            updateIndividualDatabase(side, sector, fileContent, "EWT",
                                     m_EWTdb[side][sector]);
            updateIndividualDatabase(side, sector, fileContent, "EST",
                                     m_ESTdb[side][sector]);
        }
        for (int sector = 0; sector < TGCId::NUM_INNER_SECTOR; sector++) {
            updateIndividualDatabase(side, sector, fileContent, "EWI",
                                     m_EWIdb[side][sector]);
            updateIndividualDatabase(side, sector, fileContent, "ESI",
                                     m_ESIdb[side][sector]);
            updateIndividualDatabase(side, sector, fileContent, "FWI",
                                     m_FWIdb[side][sector]);
            updateIndividualDatabase(side, sector, fileContent, "FSI",
                                     m_FSIdb[side][sector]);
        }
    }
}

std::vector<std::vector<int>> TGCCableASDToPP::getUpdateInfo(
    const int side, const int sector, const std::vector<std::string>& diffFile,

    const std::string& blockname) {
    // clear info
    std::vector<std::vector<int>> info{};

    std::vector<std::string>::const_iterator it = diffFile.begin();
    std::vector<std::string>::const_iterator it_e = diffFile.end();
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
    return info;
}

TGCDatabaseASDToPP* TGCCableASDToPP::getDatabase(const TGCId::SideType side,
                                                 const TGCId::RegionType region,
                                                 const int sector,
                                                 const TGCId::ModuleType module) const {
    if (side >= TGCId::SideType::MaxSideType) {
        return nullptr;
    }
    if (sector < 0) {
        return nullptr;
    }

    TGCDatabaseASDToPP* db = nullptr;
    if (region == TGCId::RegionType::Endcap) {
        switch (module) {
            case TGCId::ModuleType::WD:
                if (sector < TGCId::NUM_ENDCAP_SECTOR) {
                    db = m_EWDdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::SD:
                if (sector < TGCId::NUM_ENDCAP_SECTOR) {
                    db = m_ESDdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::WT:
                if (sector < TGCId::NUM_ENDCAP_SECTOR) {
                    db = m_EWTdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::ST:
                if (sector < TGCId::NUM_ENDCAP_SECTOR) {
                    db = m_ESTdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::WI:
                if (sector < TGCId::NUM_INNER_SECTOR) {
                    db = m_EWIdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::SI:
                if (sector < TGCId::NUM_INNER_SECTOR) {
                    db = m_ESIdb[+side][sector].get();
                }
                break;
            default:
                break;
        }
    } else if (region == TGCId::RegionType::Forward) {
        switch (module) {
            case TGCId::ModuleType::WD:
                if (sector < TGCId::NUM_FORWARD_SECTOR) {
                    db = m_FWDdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::SD:
                if (sector < TGCId::NUM_FORWARD_SECTOR) {
                    db = m_FSDdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::WT:
                if (sector < TGCId::NUM_FORWARD_SECTOR) {
                    db = m_FWTdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::ST:
                if (sector < TGCId::NUM_FORWARD_SECTOR) {
                    db = m_FSTdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::WI:
                if (sector < TGCId::NUM_INNER_SECTOR) {
                    db = m_FWIdb[+side][sector].get();
                }
                break;
            case TGCId::ModuleType::SI:
                if (sector < TGCId::NUM_INNER_SECTOR) {
                    db = m_FSIdb[+side][sector].get();
                }
                break;
            default:
                break;
        }
    }
    return db;
}

std::unique_ptr<TGCChannelId> TGCCableASDToPP::getChannel(
    const TGCChannelId& channelId, bool orChannel) const {
    if (channelId.getChannelIdType() == TGCChannelId::ChannelIdType::ASDOut) {
        return getChannelOut(channelId, orChannel);
    }
    if (channelId.getChannelIdType() == TGCChannelId::ChannelIdType::PPIn) {
        return getChannelIn(channelId, orChannel);
    }

    return nullptr;
}

std::unique_ptr<TGCChannelId> TGCCableASDToPP::getChannelIn(
    const TGCChannelId& ppin, bool orChannel) const {
    if (orChannel || !ppin.isValid()) {
        return nullptr;
    }

    TGCDatabaseASDToPP* databaseP =
        getDatabase(ppin.getSideType(), ppin.getRegionType(), ppin.getSector(),
                    ppin.getModuleType());

    if (databaseP == nullptr) {
        return nullptr;
    }

    int indexOut[TGCDatabaseASDToPP::NIndexOut] = {
        ppin.getId(), ppin.getBlock(), ppin.getChannel()};
    int i = databaseP->getIndexDBOut(indexOut);
    if (i < 0) {
        return nullptr;
    }

    // ASD2PP.db is Backward connection
    int layer = databaseP->getEntry(i, 0);
    if (ppin.isStrip()) {
        if (!ppin.isBackward()) {
            layer = s_stripForward[layer];
        }
    }
    int offset = (ppin.isWire()) ? 4 : 0;
    int channel = databaseP->getEntry(i, 2 + offset);

    // Endcap Triplet chamberId start from 1 in ASDOut
    int chamber = databaseP->getEntry(i, 1);
    if (ppin.isEndcap() && ppin.isTriplet()) {
        chamber = chamber + 1;
    }
    return std::make_unique<TGCChannelASDOut>(
        ppin.getSideType(), ppin.getSignalType(), ppin.getRegionType(),
        ppin.getSector(), layer, chamber, channel);
}

std::unique_ptr<TGCChannelId> TGCCableASDToPP::getChannelOut(
    const TGCChannelId& asdout, bool orChannel) const {
    if (orChannel || !asdout.isValid()) {
        return nullptr;
    }

    const bool asdoutisStrip = asdout.isStrip();
    const bool asdoutisBackward = asdout.isBackward();
    const bool asdoutisEndcap = asdout.isEndcap();
    const bool asdoutisTriplet = asdout.isTriplet();
    const int asdoutLayer = asdout.getLayer();
    const int asdoutChamber = asdout.getChamber();
    const int asdoutChannel = asdout.getChannel();

    TGCDatabaseASDToPP* databaseP =
        getDatabase(asdout.getSideType(), asdout.getRegionType(),
                    asdout.getSector(), asdout.getModuleType());

    if (databaseP == nullptr) {
        return nullptr;
    }

    const int MaxEntry = databaseP->getMaxEntry();
    for (int i = 0; i < MaxEntry; i++) {
        // ASD2PP.db is Backward connection
        int layer = asdoutLayer;
        if (asdoutisStrip && !asdoutisBackward) {
            layer = s_stripForward[layer];
        }

        int elecChannel = asdoutChannel;

        // Endcap Triplet chamberId start from 1 in ASDOut
        int chamber = asdoutChamber;
        if (asdoutisEndcap && asdoutisTriplet) {
            chamber = chamber - 1;
        }
        int offset = (asdout.isWire()) ? 4 : 0;
        if (databaseP->getEntry(i, 0) == layer &&
            databaseP->getEntry(i, 1) == chamber &&
            databaseP->getEntry(i, 2 + offset) == elecChannel) {
            int id = databaseP->getEntry(i, 3);
            int block = databaseP->getEntry(i, 4);
            int channel = databaseP->getEntry(i, 5);

            return std::make_unique<TGCChannelPPIn>(
                asdout.getSideType(), asdout.getStation(), asdout.getModuleType(),
                asdout.getRegionType(), asdout.getSector(), id, block, channel);
        }
    }
    return nullptr;
}

void TGCCableASDToPP::updateIndividualDatabase(
    const int side, const int sector, const std::vector<std::string>& diffFile,
    const std::string& blockname,
    std::shared_ptr<TGCDatabaseASDToPP>& database) {
    if (!database) {
        return;
    }
    std::vector<std::vector<int>> info =
        getUpdateInfo(side, sector, diffFile, blockname);

    if (info.empty()) {
        return;
    }

    if (database->isCommon()) {
        database = std::make_unique<TGCDatabaseASDToPP>(
            *database,
            false);  // false means this database is not commonly used.
    }

    for (auto& i : info) {
        database->update(i);
    }
}

}  // namespace MuonTGC_Cabling
