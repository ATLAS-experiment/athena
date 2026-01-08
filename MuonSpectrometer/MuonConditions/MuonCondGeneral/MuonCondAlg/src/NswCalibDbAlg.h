/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_NSWCALIBDBALG_H
#define MUONCONDALG_NSWCALIBDBALG_H

// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

// Muon includes
#include "MuonCondData/NswCalibDbTimeChargeData.h"
#include "MuonCondData/NswCalibDbThresholdData.h"
#include "MuonCondData/NswT0Data.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCondData/Defs.h"


// Forward declarations
class CondAttrListCollection;
class TTree;

namespace Muon {
class NswCalibDbAlg: public AthCondAlgorithm {

public:

	using AthCondAlgorithm::AthCondAlgorithm;
	virtual ~NswCalibDbAlg() = default;
	virtual StatusCode initialize() override;
	virtual StatusCode execute (const EventContext&) const override;

 
private:

	using writeKeyTdoPdo_t = SG::WriteCondHandleKey<NswCalibDbTimeChargeData>;
	using writeKeyThr_t =  SG::WriteCondHandleKey<NswCalibDbThresholdData >;
	using writeKeyMmT0_t =  SG::WriteCondHandleKey<NswT0Data>;
	using readKey_t = SG::ReadCondHandleKey<CondAttrListCollection>;
	
	using writeHandleTdoPdo_t =  SG::WriteCondHandle<NswCalibDbTimeChargeData>;
	using writeHandleThr_t = SG::WriteCondHandle<NswCalibDbThresholdData >;
	using writeHandleT0_t = SG::WriteCondHandle<NswT0Data>;

	StatusCode processTdoPdoData(const EventContext& ctx) const;
	StatusCode processThrData   (const EventContext& ctx) const;
	StatusCode processNSWT0Data   (const EventContext& ctx) const;

	using TimeChargeType = NswCalibDbTimeChargeData::CalibDataType;
	using TimeChargeTech = MuonCond::CalibTechType;
	using ThresholdTech  = NswCalibDbThresholdData::ThrsldTechType;
	StatusCode loadTimeChargeData(const EventContext& ctx, const readKey_t& readKey, const TimeChargeTech,
	                              const TimeChargeType type, writeHandleTdoPdo_t& writeHandle, NswCalibDbTimeChargeData* writeCdo) const;
	StatusCode loadThresholdData (const EventContext&, const readKey_t&, const ThresholdTech, writeHandleThr_t   &, NswCalibDbThresholdData* ) const;

	using T0Tech = MuonCond::CalibTechType;
	StatusCode loadT0ToTree(const EventContext& ctx, const readKey_t& readKey, writeHandleT0_t& writeHandle, std::unique_ptr<TTree>& tree) const;
	StatusCode loadT0Data (const std::unique_ptr<TTree>& tree, NswT0Data* writeCdo, const T0Tech tech) const;

	bool buildChannelId(Identifier& channelId, unsigned int elinkId, unsigned int vmm, unsigned int channel) const;

	Gaudi::Property<bool> m_isData           {this, "isData"           , true, "Processing data"};
	Gaudi::Property<bool> m_processThresholds{this, "processThresholds", false, "Process threshold data"};
	
	ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
	
	writeKeyTdoPdo_t m_writeKey_tdopdo{this, "WriteKey_TdoPdo", "NswCalibDbTimeChargeData", "Key of output calibration data (TDOs and PDOs)" };
	writeKeyThr_t    m_writeKey_thr   {this, "WriteKey_Thr"   , "NswCalibDbThresholdData" , "Key of output calibration data (VMM thresholds)"};
	writeKeyMmT0_t   m_writeKey_nswT0   {this, "WriteKey_NswT0"   , "NswT0Data" , "Key of output calibration data (NSW T0s)"};
	
	readKey_t m_readKey_mm_sidea_tdo  {this, "ReadKey_MM_SIDEA_TDO"  , "/MDT/MM/TIME/SIDEA"   , "Key of input MM condition data for side A data TDO"};
	readKey_t m_readKey_mm_sidec_tdo  {this, "ReadKey_MM_SIDEC_TDO"  , "/MDT/MM/TIME/SIDEC"   , "Key of input MM condition data for side C data TDO"};
	readKey_t m_readKey_mm_sidea_pdo  {this, "ReadKey_MM_SIDEA_PDO"  , "/MDT/MM/CHARGE/SIDEA" , "Key of input MM condition data for side A data PDO"};
	readKey_t m_readKey_mm_sidec_pdo  {this, "ReadKey_MM_SIDEC_PDO"  , "/MDT/MM/CHARGE/SIDEC" , "Key of input MM condition data for side C data PDO"};
	readKey_t m_readKey_mm_sidea_thr  {this, "ReadKey_MM_SIDEA_THR"  , "/MDT/MM/THR/SIDEA"    , "Key of input MM condition data for side A data THR"};
	readKey_t m_readKey_mm_sidec_thr  {this, "ReadKey_MM_SIDEC_THR"  , "/MDT/MM/THR/SIDEC"    , "Key of input MM condition data for side C data THR"};
	readKey_t m_readKey_stgc_sidea_tdo{this, "ReadKey_STGC_SIDEA_TDO", "/TGC/NSW/TIME/SIDEA"  , "Key of input sTGC condition data for side A data TDO"};
	readKey_t m_readKey_stgc_sidec_tdo{this, "ReadKey_STGC_SIDEC_TDO", "/TGC/NSW/TIME/SIDEC"  , "Key of input sTGC condition data for side C data TDO"};
	readKey_t m_readKey_stgc_sidea_pdo{this, "ReadKey_STGC_SIDEA_PDO", "/TGC/NSW/CHARGE/SIDEA", "Key of input sTGC condition data for side A data PDO"};
	readKey_t m_readKey_stgc_sidec_pdo{this, "ReadKey_STGC_SIDEC_PDO", "/TGC/NSW/CHARGE/SIDEC", "Key of input sTGC condition data for side C data PDO"};
	readKey_t m_readKey_stgc_sidea_thr{this, "ReadKey_STGC_SIDEA_THR", "/TGC/NSW/THR/SIDEA"   , "Key of input sTGC condition data for side A data THR"};
	readKey_t m_readKey_stgc_sidec_thr{this, "ReadKey_STGC_SIDEC_THR", "/TGC/NSW/THR/SIDEC"   , "Key of input sTGC condition data for side C data THR"};

	Gaudi::Property<bool> m_loadMmT0Data {this, "loadMmT0Data", false, "Enable loading the sTgc T0Data"};
	Gaudi::Property<bool> m_loadsTgcT0Data {this, "loadsTgcT0Data", false, "Enable loading the sTgcT0Data"};
	readKey_t m_readKey_mm_t0{this, "ReadKey_MM_T0", ""   , "Key of input MM condition data for side A data T0"};
	readKey_t m_readKey_stgc_t0{this, "ReadKey_STGC_T0", ""   , "Key of input sTGC condition data for side C data T0"};
	Gaudi::Property<std::string> m_mmT0FilePath{this, "MmT0FileName", "", "Path to a file containing the MM T0 data, this will override the data from the conditions db"};
	Gaudi::Property<std::string> m_stgcT0FilePath{this, "sTgcT0FileName", "", "Path to a file containing the sTGC T0 data, this will override the data from the conditions db"};
 
};
}

#endif
