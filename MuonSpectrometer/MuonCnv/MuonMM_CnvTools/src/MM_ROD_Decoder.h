/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAM_MM_ROD_DECODER_H
#define MUONBYTESTREAM_MM_ROD_DECODER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonMM_CnvTools/IMM_ROD_Decoder.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "MuonCablingData/Nsw_CablingMap.h"
#include "MuonCondData/NswDcsDbData.h"

class MmIdHelper;

namespace Muon
{

class MM_RawData;
class MM_RawDataCollection;

class MM_ROD_Decoder : virtual public IMM_ROD_Decoder, public AthAlgTool 
{
  public: 
    MM_ROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent ) ;
    virtual ~MM_ROD_Decoder() = default;  
    virtual StatusCode initialize() override;
    virtual StatusCode fillCollection(const EventContext&, const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment&, const std::vector<IdentifierHash>&, std::unordered_map<IdentifierHash, std::unique_ptr<MM_RawDataCollection>>&) const override;

  private:
    const MmIdHelper* m_MmIdHelper = nullptr;
    SG::ReadCondHandleKey<Nsw_CablingMap> m_cablingKey{this, "CablingMap", "NswCabling","Key of Nsw_CablingMap"};

    SG::ReadCondHandleKey<NswDcsDbData> m_dscKey{this, "DcsKey", "NswDcsDbData",
        "Key of NswDcsDbData object containing DCS conditions data"};


};

}

#endif



