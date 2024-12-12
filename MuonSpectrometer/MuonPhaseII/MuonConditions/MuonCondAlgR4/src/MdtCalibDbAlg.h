/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/

/**
   MdtCalibDbAlgR4 reads raw condition data and writes derived condition data to the condition store
*/

#ifndef MUONCALIBR4_MDTCALIBDBALGR4_H
#define MUONCALIBR4_MDTCALIBDBALGR4_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCalibITools/IIdToFixedIdTool.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include <nlohmann/json.hpp>



namespace MuonCalibR4 {
	class MdtCalibDbAlg : public AthReentrantAlgorithm {
		public:
			MdtCalibDbAlg(const std::string& name, ISvcLocator* pSvcLocator);
			virtual ~MdtCalibDbAlg() = default;

			virtual StatusCode initialize() override;
			virtual StatusCode execute(const EventContext& ctx) const override;
			virtual bool isReEntrant() const override {return false;}

			struct RtPayload {
				std::string station;
				int eta;
				int phi;
				int ml;
				int layer;
				int tube;
				bool appliedRT;
				std::vector<double> radii{};
				std::vector<double> times{};
				std::vector<double> resolutions{};
			};

			struct RtTubePayload {
				std::string station;
				int eta;
				int phi;
				double tZero;
				double meanAdc;
				int statusCode;
				int ml;
				int tube;
				int layer;
				bool appliedT0;
			};

		private:
			using RtRelationPtr = MuonCalib::MdtFullCalibData::RtRelationPtr;
			using TubeContainerPtr = MuonCalib::MdtFullCalibData::TubeContainerPtr;
			using RtMap = std::map<Identifier, RtRelationPtr>;

			StatusCode loadRt(const EventContext& ctx, MuonCalib::MdtCalibDataContainer& writeCdo) const;
			StatusCode loadTube(MuonCalib::MdtCalibDataContainer& writeCdo) const;
			StatusCode readRtPayload(const coral::AttributeList& attributeList, std::unordered_map<Identifier, RtPayload>& rtCalibration) const;
			StatusCode readTubePayload(const coral::AttributeList& attributeList, std::unordered_map<Identifier, RtTubePayload>& tubeCalibration) const;

			ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
			const MuonGMR4::MuonDetectorManager* m_r4detMgr{nullptr};
			SG::WriteCondHandleKey<MuonCalib::MdtCalibDataContainer> m_writeKey{this, "WriteKey",  "MdtCalibConstants",
																							"Conditions object containing the calibrations"};   
                                                    
			SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyRt{this, "ReadKeyRt", "/MDT/RTBLOB", "DB folder containing the RT calibrations"};

			SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyTube{this, "ReadKeyTube", "/MDT/T0BLOB", "DB folder containing the tube constants"};
			ToolHandle<MuonCalib::IIdToFixedIdTool> m_idToFixedIdTool{this, "IdToFixedIdTool", "MuonCalib::IdToFixedIdTool"};

			Gaudi::Property<double> m_prop_beta{this, "PropagationSpeedBeta", 1., "Speed of the signal propagation"};
			Gaudi::Property<unsigned int> m_maxOrder{this, "MaxOrder", 9, "Maximum order of the polynomial fit"};
	};
}
#endif
