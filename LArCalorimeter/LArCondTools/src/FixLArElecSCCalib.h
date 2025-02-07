/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FIXLARELECSCCALIB_H
#define FIXLARELECSCCALIB_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "Identifier/HWIdentifier.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArCalibLineMapping.h"
#include "LArRawConditions/LArMCSym.h"

#include <string>

class  LArOnline_SuperCellID;
class  CaloCell_SuperCell_ID;

#include "LArRawConditions/LArRampComplete.h"
#include "LArRawConditions/LArPedestalComplete.h"

// class to collect fixes, needed for SuperCells conditions
// mainly to record, what was done, and as example, if other problems will need a fix
class FixLArElecSCCalib : public AthAlgorithm
{
 public:
  FixLArElecSCCalib(const std::string& name,ISvcLocator* pSvcLocator);
  virtual ~FixLArElecSCCalib();

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override {return StatusCode::SUCCESS;}  
  virtual StatusCode stop() override;  

  void print (const HWIdentifier& hwid, const LArOnlineID_Base* onlineID, const CaloCell_Base_ID* caloCellID, 
              const Identifier *id = nullptr, std::vector<HWIdentifier>* calibIDs=nullptr,std::ostream& out=std::cout);

  StatusCode fix1();  
  StatusCode fix2(const LArOnOffIdMapping *cabling, const LArCalibLineMapping *cl);  
  StatusCode fix3(const LArOnOffIdMapping *cabling, const LArMCSym *sym);  

 private: 

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKeySC{this,"SCCablingKey","LArOnOffIdMapSC","SG Key of SC LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArCalibLineMapping>  m_CLKeySC{this, "SCCalibLineKey", "LArCalibLineMapSC", "SG calib line key"};
  SG::ReadCondHandleKey<LArMCSym> m_mcSymKey
  { this, "MCSymKey", "LArMCSym", "SG Key of LArMCSym object" };


  DoubleProperty   m_fixFactor{this, "FixFactor",  0.0, "which factor to apply"};
  IntegerProperty  m_fixFlag{this, "FixFlag", 1, "which fix to run"} ; 
  StringProperty   m_infile{this, "InputFile", "", "which file to read"}; 

  const LArEM_SuperCell_ID* m_sem_idhelper;
  const LArHEC_SuperCell_ID* m_shec_idhelper;
  const LArFCAL_SuperCell_ID* m_sfcal_idhelper;
  const LArOnline_SuperCellID* m_sonline_idhelper;
  const CaloCell_SuperCell_ID* m_scell_idhelper;
};

#endif // FixLArElecSCCalib
