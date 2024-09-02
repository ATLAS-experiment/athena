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

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"


namespace MuonCalibR4 {
	class MdtCalibDbAlg : public AthReentrantAlgorithm {
		public:
			MdtCalibDbAlg(const std::string& name, ISvcLocator* pSvcLocator);
			virtual ~MdtCalibDbAlg() = default;

			virtual StatusCode initialize() override;
			virtual StatusCode execute(const EventContext& ctx) const override;
			virtual bool isReEntrant() const override {return false;}

		private:
			using RtRelationPtr = MuonCalib::MdtFullCalibData::RtRelationPtr;
			using TubeContainerPtr = MuonCalib::MdtFullCalibData::TubeContainerPtr;

			StatusCode loadRt(MuonCalib::MdtCalibDataContainer& writeCdo) const;
			StatusCode loadTube(MuonCalib::MdtCalibDataContainer& writeCdo) const;

			ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
			const MuonGMR4::MuonDetectorManager* m_r4detMgr{nullptr};
			SG::WriteCondHandleKey<MuonCalib::MdtCalibDataContainer> m_writeKey{this, "WriteKey",  "MdtCalibConstants",
																							"Conditions object containing the calibrations"};                                                            

			Gaudi::Property<double> m_defaultT0{this, "defaultT0", 0., "default T0 value to be used in absence of DB information"};
			Gaudi::Property<double> m_prop_beta{this, "PropagationSpeedBeta", 1., "Speed of the signal propagation"};
	};
}
#endif
