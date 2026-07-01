///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCONDPHYSALGS_CALOCELLCALCENERGYCORR_H
#define CALOCONDPHYSALGS_CALOCELLCALCENERGYCORR_H 1
// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"

#include "CaloIdentifier/CaloIdManager.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "LArRecConditions/LArHVIdMapping.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/CondHandleKeyArray.h"


class CaloCellCalcEnergyCorr: public AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  CaloCellCalcEnergyCorr( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~CaloCellCalcEnergyCorr(); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) override;
  virtual StatusCode  finalize() override;
  virtual StatusCode  stop() override;

private:
  struct HVData;

  // Properties
  Gaudi::Property<std::string> m_folder{this, "Folder", "/LAR/CellCorrOfl/EnergyCorr"};
  Gaudi::Property<std::vector<int>> m_calosample{this, "CaloSamples", {-1}};  // which CaloSamples to be filled (numbers from CaloSample enum in CaloCell_Base_ID
  Gaudi::Property<std::vector<float>> m_value{this, "SampleValues", {1.0}};     // value to fill for every sample
  Gaudi::Property<std::vector<int>> m_hvlines{this, "HVLines", {-1}};        // which hvlines should be changed in addition
    Gaudi::Property<std::vector<float>> m_hvvalue{this, "HVvalues", {1.0}};    // value to fill for every hvline

  SG::ReadCondHandleKey<LArHVIdMapping> m_hvCablingKey
    {this, "LArHVIdMapping", "LArHVIdMap", "SG key for HV ID mapping"};
  SG::ReadCondHandleKeyArray<CondAttrListCollection>  m_DCSFolderKeys
    {this, "DCSFolderNames", {"/LAR/DCS/HV/BARREl/I16", "/LAR/DCS/HV/BARREL/I8"},
     "DCS folders with HV values"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey 
    {this, "CaloDetDescrManager", "CaloDetDescrManager",
     "SG Key for CaloDetDescrManager in the Condition Store" };

  // Private members
  const LArEM_ID* m_larem_id{};
  const LArHEC_ID* m_larhec_id{};
  const LArFCAL_ID* m_larfcal_id{};

  std::vector<int> GetHVLines(const HVData& hvdata, const Identifier& id, const CaloDetDescrManager* caloMgr) ;
}; 

#endif //> !CALOCONDPHYSALGS_CALOCELLCALCENERGYCORR_H
