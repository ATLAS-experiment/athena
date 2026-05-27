/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCHANNELSLBIN_HH
#define MUONTGC_CABLING_TGCCHANNELSLBIN_HH

#include "MuonTGC_Cabling/TGCChannelId.h"

namespace MuonTGC_Cabling {

class TGCChannelSLBIn : public TGCChannelId {
   public:
    TGCChannelSLBIn(TGCId::SideType side, TGCId::ModuleType module,
                    TGCId::RegionType region, int sector, int id, int channel);
    TGCChannelSLBIn(TGCId::SideType side, TGCId::StationType station,
                    TGCId::ModuleType module, TGCId::RegionType region,
                    int sector, int id, int channel);
    virtual ~TGCChannelSLBIn() = default;

    virtual std::unique_ptr<TGCModuleId> getModule() const override;

    virtual bool isValid() const override;

    // internal structure in 200 channel of SLBIn
   public:
    enum CellType {
        NoCellType = -1,
        CellTrig = 0,
        CellA = 1,
        CellB = 2,
        CellC = 3,
        CellD = 4,
        MaxCellType
    };

    static int convertChannelInCell(int channel);
    static CellType convertCellType(int channel);
    static int convertChannelInSLB(TGCId::ModuleType moduleType,
                                   CellType cellType, int channel);
    static int convertChannel(TGCId::ModuleType moduleType, CellType cellType,
                              int channelInSLB);
    static int getLengthOfCell(CellType cellType);
    static int getOffsetOfCell(CellType cellType);
    static int getLengthOfSLB(TGCId::ModuleType moduleType, CellType cellType);
    static int getAdjacentOfCell(CellType cellType);
    static int getAdjacentOfSLB(TGCId::ModuleType moduleType,
                                CellType cellType);

    virtual CellType getCellType() const { return m_cellType; }

    virtual int getChannelInCell() const;

    virtual int getChannelInSLB() const;

    virtual void setChannel(int channel) override;

   private:
    CellType m_cellType;
    int m_channelInCell = 0;
    int m_channelInSLB = 0;

    static const int s_lengthCell[];
    static const int s_offsetCell[];
    static const int s_lengthWD[];
    static const int s_lengthSD[];
    static const int s_lengthWT[];
    static const int s_lengthST[];
    static const int s_adjacentCell[];
    static const int s_adjacentWD[];
    static const int s_adjacentSD[];
    static const int s_adjacentWT[];
    static const int s_adjacentST[];

    TGCChannelSLBIn() {}
};

}  // namespace MuonTGC_Cabling

#endif
