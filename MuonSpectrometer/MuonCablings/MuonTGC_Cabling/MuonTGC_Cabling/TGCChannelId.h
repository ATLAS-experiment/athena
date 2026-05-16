/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCHANNELID_H
#define MUONTGC_CABLING_TGCCHANNELID_H



#include "MuonTGC_Cabling/TGCId.h"
#include "MuonTGC_Cabling/TGCModuleId.h"
#include <memory>
#include <cstddef>//std::size_t

namespace MuonTGC_Cabling {

class TGCModuleId;

class TGCChannelId : public TGCId {
   public:
    enum class ChannelIdType {
        NoChannelIdType,
        ASDIn,
        ASDOut,
        PPIn,
        PPOut,
        SLBIn,
        SLBOut,
        HPBIn
    };

   public:
    TGCChannelId(ChannelIdType type = ChannelIdType::NoChannelIdType)
        : TGCId(IdType::Channel) {
        m_channelType = type;
    }
    virtual ~TGCChannelId() = default;

    virtual std::unique_ptr<TGCModuleId> getModule() const { return nullptr; }

    virtual bool operator==(const TGCChannelId& channelId) const;

    virtual bool isValid() const { return true; }

    ChannelIdType getChannelIdType() const;
    int getLayer() const;
    int getBlock() const;
    int getChannel() const;
    virtual int getGasGap() const;

    void setChannelIdType(ChannelIdType type);
    void setLayer(int layer);
    virtual void setBlock(int block);
    virtual void setChannel(int channel);

    bool isBackward() const;
    
    

   protected:
    ChannelIdType m_channelType{ChannelIdType::NoChannelIdType};
    int m_layer{-1};
    int m_block{-1};
    int m_channel{-1};
};

}  // namespace MuonTGC_Cabling

#endif
