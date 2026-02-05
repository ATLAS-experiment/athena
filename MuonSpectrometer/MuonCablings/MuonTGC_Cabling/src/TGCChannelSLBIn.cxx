/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelSLBIn.h"

#include <stdexcept>
#include <string>

#include "MuonTGC_Cabling/TGCModuleSLB.h"

namespace {
void checkCellType(MuonTGC_Cabling::TGCChannelSLBIn::CellType cellType,
                   const std::string& funcName) {
    if (cellType == MuonTGC_Cabling::TGCChannelSLBIn::CellType::NoCellType or
        cellType == MuonTGC_Cabling::TGCChannelSLBIn::CellType::MaxCellType) {
        const std::string msg = "cellType out of range in " + funcName;
        throw std::out_of_range(msg);
    }
}

}  // namespace

namespace MuonTGC_Cabling {

// Constructor
TGCChannelSLBIn::TGCChannelSLBIn(TGCId::SideType vside,
                                 TGCId::ModuleType vmodule,
                                 TGCId::RegionType vregion, int vsector,
                                 int vid, int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::SLBIn) {
    setSideType(vside);
    setModuleType(vmodule);
    setRegionType(vregion);
    setSector(vsector);
    setId(vid);
    TGCChannelSLBIn::setChannel(vchannel);
}

TGCModuleId* TGCChannelSLBIn::getModule(void) const {
    return (new TGCModuleSLB(getSideType(), getModuleType(), getRegionType(),
                             getSector(), getId()));
}

bool TGCChannelSLBIn::isValid(void) const {
    if ((getSideType() > TGCId::NoSideType) &&
        (getSideType() < TGCId::MaxSideType) &&
        (getModuleType() > TGCId::NoModuleType) &&
        (getModuleType() < TGCId::MaxModuleType) &&
        (getRegionType() > TGCId::NoRegionType) &&
        (getRegionType() < TGCId::MaxRegionType) && (getOctant() >= 0) &&
        (getOctant() < 8) && (getId() >= 0) && (getChannel() >= 0) &&
        (m_channelInSLB < getLengthOfSLB(getModuleType(), getCellType())) &&
        (m_channelInSLB >= 0)) {
        return true;
    }
    return false;
}

//                                           Trig  A   B   C   D
const int TGCChannelSLBIn::s_lengthCell[] = {40, 36, 36, 44, 44};
const int TGCChannelSLBIn::s_offsetCell[] = {0, 40, 76, 112, 156};
const int TGCChannelSLBIn::s_lengthWD[] = {40, 36, 36, 44, 44};
const int TGCChannelSLBIn::s_lengthSD[] = {40, 32, 32, 32, 32};
const int TGCChannelSLBIn::s_lengthWT[] = {40, 36, 36, 36, 36};
const int TGCChannelSLBIn::s_lengthST[] = {40, 32, 32, 32, 32};
const int TGCChannelSLBIn::s_adjacentCell[] = {0, 2, 2, 6, 6};
const int TGCChannelSLBIn::s_adjacentWD[] = {0, 2, 2, 6, 6};
const int TGCChannelSLBIn::s_adjacentSD[] = {0, 0, 0, 0, 0};
const int TGCChannelSLBIn::s_adjacentWT[] = {0, 2, 2, 2, 2};
const int TGCChannelSLBIn::s_adjacentST[] = {0, 0, 0, 0, 0};

int TGCChannelSLBIn::getLengthOfCell(CellType cellType) {
    checkCellType(cellType, __func__);
    return s_lengthCell[cellType];
}

int TGCChannelSLBIn::getOffsetOfCell(CellType cellType) {
    checkCellType(cellType, __func__);
    return s_offsetCell[cellType];
}

int TGCChannelSLBIn::getLengthOfSLB(TGCId::ModuleType moduleType,
                                    CellType cellType) {
    checkCellType(cellType, __func__);
    switch (moduleType) {
        case TGCId::WD:
            return s_lengthWD[cellType];
        case TGCId::SD:
            return s_lengthSD[cellType];
        case TGCId::WT:
            return s_lengthWT[cellType];
        case TGCId::ST:
            return s_lengthST[cellType];
        case TGCId::WI:
            return s_lengthST[cellType];
        case TGCId::SI:
            return s_lengthST[cellType];
        default:
            break;
    }
    return -1;
}

int TGCChannelSLBIn::getAdjacentOfCell(CellType cellType) {
    checkCellType(cellType, __func__);
    return s_adjacentCell[cellType];
}

int TGCChannelSLBIn::getAdjacentOfSLB(TGCId::ModuleType moduleType,
                                      CellType cellType) {
    checkCellType(cellType, __func__);
    switch (moduleType) {
        case TGCId::WD:
            return s_adjacentWD[cellType];
        case TGCId::SD:
            return s_adjacentSD[cellType];
        case TGCId::WT:
            return s_adjacentWT[cellType];
        case TGCId::ST:
            return s_adjacentST[cellType];
        case TGCId::WI:
            return s_adjacentST[cellType];
        case TGCId::SI:
            return s_adjacentST[cellType];
        default:
            break;
    }
    return -1;
}

int TGCChannelSLBIn::convertChannelInCell(int channel) {
    if (channel >= getOffsetOfCell(CellTrig) &&
        channel < getOffsetOfCell(CellTrig) + getLengthOfCell(CellTrig)) {
        return channel - getOffsetOfCell(CellTrig);
    }
    if (channel >= getOffsetOfCell(CellA) &&
        channel < getOffsetOfCell(CellA) + getLengthOfCell(CellA)) {
        return channel - getOffsetOfCell(CellA);
    }
    if (channel >= getOffsetOfCell(CellB) &&
        channel < getOffsetOfCell(CellB) + getLengthOfCell(CellB)) {
        return channel - getOffsetOfCell(CellB);
    }
    if (channel >= getOffsetOfCell(CellC) &&
        channel < getOffsetOfCell(CellC) + getLengthOfCell(CellC)) {
        return channel - getOffsetOfCell(CellC);
    }
    if (channel >= getOffsetOfCell(CellD) &&
        channel < getOffsetOfCell(CellD) + getLengthOfCell(CellD)) {
        return channel - getOffsetOfCell(CellD);
    }
    return -1;
}

TGCChannelSLBIn::CellType TGCChannelSLBIn::convertCellType(int channel) {
    if (channel >= getOffsetOfCell(CellTrig) &&
        channel < getOffsetOfCell(CellTrig) + getLengthOfCell(CellTrig)) {
        return CellTrig;
    }
    if (channel >= getOffsetOfCell(CellA) &&
        channel < getOffsetOfCell(CellA) + getLengthOfCell(CellA)) {
        return CellA;
    }
    if (channel >= getOffsetOfCell(CellB) &&
        channel < getOffsetOfCell(CellB) + getLengthOfCell(CellB)) {
        return CellB;
    }
    if (channel >= getOffsetOfCell(CellC) &&
        channel < getOffsetOfCell(CellC) + getLengthOfCell(CellC)) {
        return CellC;
    }
    if (channel >= getOffsetOfCell(CellD) &&
        channel < getOffsetOfCell(CellD) + getLengthOfCell(CellD)) {
        return CellD;
    }
    return NoCellType;
}

int TGCChannelSLBIn::convertChannelInSLB(TGCId::ModuleType moduleType,
                                         CellType cellType, int channel) {
    const int adjacentSLB = getAdjacentOfSLB(moduleType, cellType);
    if (adjacentSLB == -1) {
        throw std::out_of_range(
            "adjacentSLB is -1 in TGCChannelSLBIn::convertChannelInSLB");
    }
    int offset = getAdjacentOfCell(cellType) - adjacentSLB;
    return channel - offset;
}

int TGCChannelSLBIn::convertChannel(TGCId::ModuleType moduleType,
                                    CellType cellType, int channelInSLB) {
    const int adjacentSLB = getAdjacentOfSLB(moduleType, cellType);
    if (adjacentSLB == -1) {
        throw std::out_of_range(
            "adjacentSLB is -1 in TGCChannelSLBIn::convertChannel");
    }
    int offset = getAdjacentOfCell(cellType) - adjacentSLB;
    return getOffsetOfCell(cellType) + offset + channelInSLB;
}

int TGCChannelSLBIn::getChannelInCell(void) const {
    return m_channelInCell;
}

int TGCChannelSLBIn::getChannelInSLB(void) const {
    return m_channelInSLB;
}

void TGCChannelSLBIn::setChannel(int vchannel) {
    TGCChannelId::setChannel(vchannel);
    m_cellType = convertCellType(vchannel);
    m_channelInCell = convertChannelInCell(vchannel);
    m_channelInSLB =
        convertChannelInSLB(getModuleType(), getCellType(), getChannelInCell());
}

}  // namespace MuonTGC_Cabling
