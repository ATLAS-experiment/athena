/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCALOREC_HLTCALOGLOBALCELLMAKER_H
#define TRIGCALOREC_HLTCALOGLOBALCELLMAKER_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <CaloEvent/CaloConstCellContainer.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

class CaloCell_ID;
class LArOnlineID; 
class LArOnOffIdMapping;
class CaloNoise;

class HLTCaloGlobalCellMaker : public AthReentrantAlgorithm {
  public:
    HLTCaloGlobalCellMaker(std::string const& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(EventContext const& context) const override;

  private:
    SG::ReadHandleKey<CaloConstCellContainer> m_inputCellContainerKey {
      this, "InputCellKey", "HLTCaloCellInput", "" };

    SG::WriteHandleKey<CaloConstCellContainer> m_outputCellContainerKey {
      this, "OutputCellKey", "HLTCaloCellOutput", "" };

    Gaudi::Property<float> m_NumberOfSigma { this, "NumberOfSigma", 2.0, "Number of Sigma Above Noise" };
    Gaudi::Property<uint32_t> m_MaxNCellsPerFEB { this, "MaxNCellsPerFEB", 30, "Maximum Number Cells Per FEB" };

    // We need to have the online and offline and the cabling
    // we also need to have the noise tables somewhere.
    // For Tile, all cells avaiable will be copied (for the moment)

    const CaloCell_ID*  m_caloCell_ID{nullptr};
    const LArOnlineID*  m_onlineId{nullptr};
    SG::ReadCondHandleKey<LArOnOffIdMapping> m_onOffIdMappingKey {this, "CablingKey", "LArOnOffIdMap", "SG Key for LArOnOffIdMapping"} ;
    SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this,"CaloNoiseKey","totalNoise","SG Key of CaloNoise data object"};
    

};

#endif /* TRIGCALOREC_HLTCALOGLOBALCELL_H */
