/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWT0DATA_H
#define MUONCONDDATA_NSWT0DATA_H



// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 
// STL includes
#include <vector>

class Identifier;

namespace Muon{
  class IMuonIdHelperSvc;
}

class NswT0Data {


public:
    NswT0Data(const Muon::IMuonIdHelperSvc* idHelperSvc);
     ~NswT0Data() = default;

    // setting functions
    void setData(const Identifier& channelId, const float channelT0);

    // retrieval functions
    bool getT0 (const Identifier&  channelId, float& channelT0) const;
 
private:
    unsigned int identToModuleIdx(const Identifier& chan_id) const;

    // ID helpers
    const Muon::IMuonIdHelperSvc* m_idHelperSvc{};
    // containers
    using ChannelArray = std::vector<std::vector<float>>;
    ChannelArray m_data_mmg{};
    ChannelArray m_data_stg{};


};

CLASS_DEF( NswT0Data ,148122593  , 1 )
CLASS_DEF( CondCont<NswT0Data> , 69140433 , 1 )

#endif
