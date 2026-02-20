/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GepCellsHandlerALG_H
#define TRIGGEPPERF_GepCellsHandlerALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "CaloEvent/CaloCellContainer.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloConditions/CaloNoise.h"

#include "TrigGepPerf/GepCellMap.h"

#include <vector>
#include <map>
class CaloCell;
class CaloCell_ID;

class GepCellsHandlerAlg: public ::AthReentrantAlgorithm {

 public:

  GepCellsHandlerAlg( const std::string& name, ISvcLocator* pSvcLocator );
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) const override;

 private:

  StatusCode setNumberOfEnergyBits(int value) {

	// At the moment only have a detailed scheme for 6-10 bit readouts, thus rejecting any other value
	switch(value) {
		case 6: m_maxCellsPerFEB = 62; break;
		case 7: m_maxCellsPerFEB = 54; break;
		case 8: m_maxCellsPerFEB = 48; break;
		case 9: m_maxCellsPerFEB = 43; break;
		case 10: m_maxCellsPerFEB = 39; break;
                default: ATH_MSG_FATAL("A GEP energy encoding scheme with " << value << " energy bits is currently not defined");
                return StatusCode::FAILURE;
	}

        ATH_MSG_INFO("Setting GEP energy encoding to a " << value << "-bit scheme which allows for the sending of " << m_maxCellsPerFEB << " cells to GEP per FEB");
	m_nEnergyBits = value;

	return StatusCode::SUCCESS;
  }

  StatusCode setLeastSignificantBit(int value) {
	if (value < 1) {
		ATH_MSG_FATAL("The value of the least significant bit in the GEP energy encoding cannot be set to " << value);
                return StatusCode::FAILURE;
	}
        ATH_MSG_INFO("Setting the value for the least significant bit in the GEP energy encoding to " << value << " MeV");
        m_valLeastSigBit = value;

	return StatusCode::SUCCESS;
  }

  StatusCode setG(int value) {
	if (value < 1) {
                ATH_MSG_FATAL("The value of G in the GEP energy encoding cannot be set to " << value);
                return StatusCode::FAILURE;
        }
        ATH_MSG_INFO("Setting the value for G in the GEP energy encoding to " << value);
        m_valG = value;

	return StatusCode::SUCCESS;
  }
 
  // Values are set in the initialize function
  int m_nEnergyBits = -1; 
  int m_valLeastSigBit = -1;
  int m_valG = -1;
  int m_readoutRanges[5] = {-1,-1,-1,-1,-1};
  int m_stepsPerRange = -1;
  unsigned m_maxCellsPerFEB = -1;

  std::map<unsigned int,Gep::GepCaloCell> m_gepCellsBase = {};

  Gaudi::Property<std::string> m_GepEnergyEncodingScheme {this, "GEPEnergyEncodingScheme", "", 
    "String defining the GEP readout scheme according to number of readout bits + '-' + value of LSB in MeV + '-' + gain value"};

  Gaudi::Property<bool> m_doGepHardwareStyleEnergyEncoding {this, "HardwareStyleEnergyEncoding", false, 
    "Enabling or disabling the hardware-style energy encoding for the GEP"};

  Gaudi::Property<bool> m_doTruncationOfOverflowingFEBs {this, "TruncationOfOverflowingFEBs", false, 
    "Enabling or disabling the truncation of cells from FEBs with more than the maximum number of cells which can be send"};

  Gaudi::Property<bool> m_writeAllCells {this, "WriteAllCells", false, 
    "If true, all cells are considered regardless whether they are truncated or below the 2sigma threshold"};

  Gaudi::Property<bool> m_cleanOutputCells {this, "CleanOutputCells", false, 
    "If true, all output cells are included in the output, but those truncated or below 2sigma will have their energies set to 0; to be used only with WriteAllCells"};

  Gaudi::Property<std::string> m_LArCellMap {this, "LArCellMapFile", "UpgradePerformanceFunctions/LAr_Cell_Map_offlineID_0.csv", 
    "File associating LAr cells with readout FEBs and connection technology"};

  /** @brief Key of the CaloNoise Conditions data object. Typical values 
      are '"electronicNoise', 'pileupNoise', or '"totalNoise' (default) */

  SG::ReadCondHandleKey<CaloNoise> m_electronicNoiseKey{this, "electronicNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};
 
  SG::ReadCondHandleKey<CaloNoise> m_totalNoiseKey{this, "totalNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};

  SG::WriteHandleKey<Gep::GepCellMap> m_outputGepCellsKey{this, "outputGepCellsKey", "", "Key for GepCell map"};

  SG::ReadHandleKey<CaloCellContainer> m_caloCellsKey {this, "caloCells", "AllCalo", "key to read in a CaloCell constainer"};

  const CaloCell_ID* m_CaloCell_ID{nullptr};
  int getGepEnergy(float offline_et) const;
  std::vector<unsigned int> getNeighbours(const CaloCellContainer& allcells,
					  const CaloCell* acell,
					  const EventContext&) const;
  StatusCode removeCellsFromOverloadedFEB(std::vector<Gep::GepCaloCell> &cells) const;

};

#endif //> !TRIGGEPPERF_GEPCELLSHANDLERALG_H
