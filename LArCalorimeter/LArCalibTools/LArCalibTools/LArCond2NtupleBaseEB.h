/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef LARCALIBTOOLS_LARCOND2NTUPLEBASEEB_H
#define LARCALIBTOOLS_LARCOND2NTUPLEBASEEB_H

//#include "GaudiKernel/Algorithm.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/ToolHandle.h"

#include "LArElecCalib/ILArFEBTempTool.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArCalibLineMapping.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

class HWIdentifier;
class LArOnlineID_Base;
class CaloCell_Base_ID;
class StoreGateSvc;
class LArEM_Base_ID;
class LArHEC_Base_ID;
class LArFCAL_Base_ID;
class MsgStream;
class CaloDetDescrManager_Base;

// Not reentrant --- derived classes fill tuples in execute().
class LArCond2NtupleBaseEB : public AthAlgorithm {

 public:
  LArCond2NtupleBaseEB(const std::string & name, ISvcLocator * pSvcLocator);
  ~LArCond2NtupleBaseEB() = default;

  //Standard algo methods
  StatusCode initialize();
  StatusCode execute(const EventContext&)    {return StatusCode::SUCCESS;}

  bool fillFromIdentifier(const HWIdentifier& id, int C); //returns true if connected

  const SG::ReadCondHandleKey<LArOnOffIdMapping>& cablingKey() const;

 private:
  bool m_initialized;


 protected:
  Gaudi::Property< bool > m_addBC{this, "AddBadChannelInfo", true, "dump BadChan info ?"};
  Gaudi::Property< bool > m_addFEBTemp{this, "AddFEBTempInfo", false, "dump FEB temperature info ?"};
  Gaudi::Property< bool > m_isSC{this, "isSC", false, "are we working with SC?"};
  Gaudi::Property< bool > m_isFlat{this, "isFlat", false, "are we working with Flat conditions ?"};
  Gaudi::Property< bool > m_OffId{this, "OffId", false, "dump also offline ID ?"};
  Gaudi::Property< bool > m_addHash{this, "AddHash", false, "add also ID hash info ?"};
  Gaudi::Property< bool > m_addCalib{this, "AddCalib", false, "add also calib line info info ?"};
  Gaudi::Property< bool > m_realgeom{this, "RealGeometry", false, "add real geometry values ?"};
  Gaudi::Property< bool > m_expandId{this,"ExpandId", true ,"add online Id decoded fields ?"};

  enum {NOT_VALID = -999};

  std::string m_ntpath, m_ntTitle;

  //Ntuple pointer
  NTuple::Tuple* m_nt;

  //Ntuple variables:
  NTuple::Array<double> m_detector, m_region, m_layer, m_eta, m_phi, m_onlChanId, m_oflChanId;
  NTuple::Array<double> m_pos_neg, m_barrel_ec, m_FT, m_slot, m_channel;
  NTuple::Array<double> m_calibLine,m_badChanWord;
  NTuple::Array<double> m_isConnected;
  NTuple::Array<double> m_chanHash, m_febHash, m_oflHash;

  NTuple::Array<float> m_reta, m_rphi;
  NTuple::Array<float> m_FEBTemp1, m_FEBTemp2;



  const int m_SC= 34048;

  StoreGateSvc* m_detStore;
  const LArEM_Base_ID* m_emId;
  const LArHEC_Base_ID* m_hecId;
  const LArFCAL_Base_ID* m_fcalId;
  const LArOnlineID_Base* m_onlineId;
  const CaloCell_Base_ID* m_caloId;
  ToolHandle<ILArFEBTempTool> m_FEBTempTool;

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingSCKey{this,"CablingSCKey","LArOnOffIdMapSC","SG Key of LArOnOffIdMapping object"};

  SG::ReadCondHandleKey<LArBadChannelCont> m_BCKey{this, "BadChanKey", "LArBadChannel", "SG bad channels key"};
  SG::ReadCondHandleKey<LArCalibLineMapping> m_calibMapKey{this,"CalibMapKey","LArCalibLineMap","SG Key of calib line mapping object"};
  SG::ReadCondHandleKey<LArCalibLineMapping> m_calibMapSCKey{this,"CalibMapSCKey","LArCalibIdMapSC","SG Key of calib line mapping object"};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this, "CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store"};
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey{this
      , "CaloSuperCellDetDescrManager"
      , "CaloSuperCellDetDescrManager"
      , "SG key of the resulting CaloSuperCellDetDescrManager" };

};
#endif
