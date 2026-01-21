/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_NSWDCSDBALG_H
#define MUONCONDALG_NSWDCSDBALG_H

// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

// Muon includes
#include "MuonCondData/NswDcsDbData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"


// Forward declarations
class CondAttrListCollection;

namespace Muon{

class NswDcsDbAlg: public AthCondAlgorithm{

public:

	using AthCondAlgorithm::AthCondAlgorithm;
	virtual ~NswDcsDbAlg() = default;
	virtual StatusCode initialize() override;
	virtual StatusCode execute (const EventContext&) const override;

 
private:

	using writeKey_t = SG::WriteCondHandleKey<NswDcsDbData>;
	using readKey_t  = SG::ReadCondHandleKey<CondAttrListCollection>;
	
	using writeHandleDcs_t = SG::WriteCondHandle<NswDcsDbData>;

	StatusCode processHvData(const EventContext& ctx) const;

	using DcsTechType = NswDcsDbData::DcsTechType;
	using DcsDataType = NswDcsDbData::DcsDataType;
	StatusCode loadHvData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, 
	                      writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const;
	StatusCode loadTDaqData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, 
	                      writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const;
	StatusCode loadELTXData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, 
	                      writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const;

	bool buildChannelIdForHv(Identifier& channelId, const DcsTechType tech0, const std::string& chanName, bool& isOK) const;
	bool buildChannelIdForTDaq(Identifier& channelId, uint& elink, const DcsTechType tech0, const std::string& chanName, bool& isOK) const;
        bool buildChannelIdForEltx(Identifier& channelId ,const DcsTechType tech0, const std::string& chanName, bool& isOK) const;

	ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
	
	writeKey_t m_writeKey{this, "WriteKey", "NswDcsDbData", "Key of output data object" };
	
	readKey_t m_readKey_mmg_hv {this, "ReadKey_MMG_HV", "/MMG/DCS/HV", "Key of input MMG condition data for HV"};
	readKey_t m_readKey_stg_hv {this, "ReadKey_STG_HV", "/STG/DCS/HV", "Key of input STG condition data for HV"};

	readKey_t m_readKey_mmg_tdaq{this, "ReadKey_MMG_TDAQ", "", "Key of input MMG condition data for TDAQ"};
	readKey_t m_readKey_stg_tdaq{this, "ReadKey_STG_TDAQ", "", "Key of input STG condition data for TDAQ"};
	
	readKey_t m_readKey_mmg_eltx{this, "ReadKey_MMG_ELTX", "", "Key of input MMG condition data for  SCA status"};
	readKey_t m_readKey_stg_eltx{this, "ReadKey_STG_ELTX", "", "Key of input STG condition data for  SCA status"};

	Gaudi::Property<bool> m_loadTdaq{this, "LoadTdaq",false,"enable the processing of Elinks in the NswDcsDbAlg"};
	Gaudi::Property<bool> m_loadEltx{this, "LoadEltx",false,"enable the processing of SCAs in the NswDcsDbAlg"};

	SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muonManagerKey{
        this, "DetectorManagerKey", "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data"};
    const MuonGM::MuonDetectorManager* m_detManager{nullptr}; 
};

}
#endif
