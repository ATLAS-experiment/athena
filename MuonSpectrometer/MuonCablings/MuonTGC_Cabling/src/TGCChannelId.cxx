/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelId.h"

namespace MuonTGC_Cabling {

bool TGCChannelId::operator==(const TGCChannelId& channelId) const {
    if ((this->getChannelIdType() == channelId.getChannelIdType()) &&
        (this->getSideType() == channelId.getSideType()) &&
        (this->getRegionType() == channelId.getRegionType()) &&
        (this->getSignalType() == channelId.getSignalType()) &&
        (this->getModuleType() == channelId.getModuleType()) &&
        (this->getSector() == channelId.getSector()) &&
        (this->getLayer() == channelId.getLayer()) &&
        (this->getChamber() == channelId.getChamber()) &&
        (this->getId() == channelId.getId()) &&
        (this->getBlock() == channelId.getBlock()) &&
        (this->getChannel() == channelId.getChannel())) {
        return true;
    }
    return false;
}

TGCChannelId::ChannelIdType TGCChannelId::getChannelIdType() const {
    return m_channelType;
}
int TGCChannelId::getLayer() const {
    return m_layer;
}
int TGCChannelId::getBlock() const {
    return m_block;
}
int TGCChannelId::getChannel() const {
    return m_channel;
}

int TGCChannelId::getGasGap() const {
    if (getLayer() == 0 || getLayer() == 3 || getLayer() == 5 ||
        getLayer() == 7) {
        return 1;
    }
    if (getLayer() == 1 || getLayer() == 4 || getLayer() == 6 ||
        getLayer() == 8) {
        return 2;
    }
    if (getLayer() == 2) {
        return 3;
    }
    return -1;
}

void TGCChannelId::setChannelIdType(ChannelIdType idtype) {
    m_channelType = idtype;
}

void TGCChannelId::setLayer(int v_layer) {
    m_layer = v_layer;
    if (m_layer >= 0 && m_layer <= 2) {
        setMultipletType(Triplet);
        setStation(0);
    }
    if (m_layer >= 3 && m_layer <= 4) {
        setMultipletType(Doublet);
        setStation(1);
    }
    if (m_layer >= 5 && m_layer <= 6) {
        setMultipletType(Doublet);
        setStation(2);
    }
    if (m_layer >= 7 && m_layer <= 8) {
        setMultipletType(Inner);
        setStation(3);
        if (m_sector != -1) {
            m_octant = m_sector / 3;
        }
    }
}

void TGCChannelId::setBlock(int block) {
    m_block = block;
}

void TGCChannelId::setChannel(int channel) {
    m_channel = channel;
}

bool TGCChannelId::isBackward() const {
    if (isEndcap()) {
        if (!isInner()) {
            if (isAside()) {
                return (m_sector % 2 == 1);
            } else {
                return (m_sector % 2 == 0);
            }
        } else {
            // EI
            // Special case of EI11
            if (m_sector == 15) {
                if (isAside()) {
                    return false;
                } else {
                    return true;
                }
            } else if (m_sector == 16) {
                if (isAside()) {
                    return true;
                } else {
                    return false;
                }
            } else {
                //  A-m_side phi0 F: phi1 F: phi2 B
                //  C-m_side phi0 B: phi1 B: phi2 F
                if (isAside()) {
                    return (m_sector % 3 == 2);
                } else {
                    return (m_sector % 3 != 2);
                }
            }
        }
    } else {
        if (isAside()) {
            return true;  // all Backward
        } else {
            return false;  // all Forward
        }
    }
}

}  // namespace MuonTGC_Cabling
