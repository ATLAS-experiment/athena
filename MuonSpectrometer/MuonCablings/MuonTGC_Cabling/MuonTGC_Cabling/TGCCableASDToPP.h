/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLEASDTOPP_H
#define MUONTGC_CABLING_TGCCABLEASDTOPP_H

#include <array>
#include <string>
#include <vector>

#include "MuonTGC_Cabling/TGCCable.h"
#include "MuonTGC_Cabling/TGCDatabaseASDToPP.h"
#include "MuonTGC_Cabling/TGCId.h"

namespace MuonTGC_Cabling {

class TGCCableASDToPP : public TGCCable {
   public:
    TGCCableASDToPP(const std::string& fileName, const std::string& diffFile);
    virtual ~TGCCableASDToPP();

    std::unique_ptr<TGCChannelId> getChannel(const TGCChannelId& channelId,
                                             bool orChannel = false) const;

   private:
    void initialize(const std::string& filename, const std::string& diffFile);

    void updateDatabase(const std::string& diffFile);

    std::unique_ptr<TGCChannelId> getChannelIn(const TGCChannelId& ppin,
                                               bool orChannel = false) const;
    std::unique_ptr<TGCChannelId> getChannelOut(const TGCChannelId& asdout,
                                                bool orChannel = false) const;

    std::vector<std::vector<int> > getUpdateInfo(
        const int side, const int sector,
        const std::vector<std::string>& diffFile, const std::string& blockname);

    TGCDatabaseASDToPP* getDatabase(const int side, const int region,
                                    const int sector, const int module) const;

    void updateIndividualDatabase(
        const int side, const int sector,
        const std::vector<std::string>& diffFile, const std::string& blockname,
        std::shared_ptr<TGCDatabaseASDToPP>& database);

   private:
    // reverse layers in Forward sector

    static constexpr std::array<int, 9> s_stripForward{2, 1, 0, 4, 3,
                                                       6, 5, 8, 7};

   private:
    using ForwardSectorDB =
        std::array<std::array<std::shared_ptr<TGCDatabaseASDToPP>,
                              TGCId::NUM_FORWARD_SECTOR>,
                   TGCId::MaxSideType>;
    using InnerSectorDB =
        std::array<std::array<std::shared_ptr<TGCDatabaseASDToPP>,
                              TGCId::NUM_INNER_SECTOR>,
                   TGCId::MaxSideType>;
    using EndcapSectorDB =
        std::array<std::array<std::shared_ptr<TGCDatabaseASDToPP>,
                              TGCId::NUM_ENDCAP_SECTOR>,
                   TGCId::MaxSideType>;

    ForwardSectorDB m_FWDdb{};
    ForwardSectorDB m_FSDdb{};
    ForwardSectorDB m_FWTdb{};
    ForwardSectorDB m_FSTdb{};
    InnerSectorDB m_FWIdb{};
    InnerSectorDB m_FSIdb{};

    EndcapSectorDB m_EWDdb{};
    EndcapSectorDB m_ESDdb{};
    EndcapSectorDB m_EWTdb{};
    EndcapSectorDB m_ESTdb{};
    InnerSectorDB m_EWIdb{};
    InnerSectorDB m_ESIdb{};

    /** Pointers of common databases are recorded in this array */
    using CommonDB = std::array<
        std::array<std::shared_ptr<TGCDatabaseASDToPP>, TGCId::MaxModuleType>,
        TGCId::MaxRegionType>;
    CommonDB m_commonDb{{{nullptr}}};
};

}  // namespace MuonTGC_Cabling

#endif
