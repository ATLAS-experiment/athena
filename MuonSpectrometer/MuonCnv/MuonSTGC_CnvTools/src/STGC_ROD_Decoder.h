/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSTGC_CNVTOOLS_STGC_ROD_DECODER_H
#define MUONSTGC_CNVTOOLS_STGC_ROD_DECODER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonSTGC_CnvTools/ISTGC_ROD_Decoder.h"
#include "MuonCondData/NswDcsDbData.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonCablingData/Nsw_CablingMap.h"

class sTgcIdHelper;

namespace Muon
{

class STGC_RawData;
class STGC_RawDataCollection;

class STGC_ROD_Decoder : virtual public ISTGC_ROD_Decoder, public AthAlgTool
{
  public:
    STGC_ROD_Decoder(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~STGC_ROD_Decoder() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode fillCollection(const EventContext& ctx,
                                      const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment&, 
                                      const std::vector<IdentifierHash>&, std::unordered_map<IdentifierHash, std::unique_ptr<STGC_RawDataCollection>>& rdo_map) const override;

  protected:
    const sTgcIdHelper* m_stgcIdHelper{nullptr};
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                            "Key of input MuonDetectorManager condition data"}; 
    SG::ReadCondHandleKey<NswDcsDbData> m_dscKey{this, "DcsKey", "NswDcsDbData",
        "Key of NswDcsDbData object containing DCS conditions data"};

    SG::ReadCondHandleKey<Nsw_CablingMap> m_cablingKey{this, "CablingMap", "","Key of Nsw_CablingMap"};


};

} // end of namespace

#endif // MUONSTGC_CNVTOOLS_STGC_ROD_DECODER_H
