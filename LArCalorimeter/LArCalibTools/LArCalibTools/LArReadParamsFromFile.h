//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


/** This algorithm reads an ascii file and fill a paremeters
    structure into the detector store.
   * @author M. Fanti
   * 20.10.2005


*/

#ifndef LARREADPARAMSFROMFILE_H
#define LARREADPARAMSFROMFILE_H
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "CaloIdentifier/CaloCell_ID.h"

#include <fstream>
#include <string>

#include "LArIdentifier/LArOnlineID.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"

#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArCalibTools/LArParamsProperties.h"

template <class DATA>
class LArReadParamsFromFile : public AthReentrantAlgorithm
{
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~LArReadParamsFromFile();

  //standard algorithm methods
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) const override {return StatusCode::SUCCESS;}
  virtual StatusCode stop ATLAS_NOT_THREAD_SAFE () override;

 private:
  const LArOnlineID* m_onlineHelper = nullptr;
  const LArEM_ID*   m_emId = nullptr;
  const LArHEC_ID*  m_hecId = nullptr;
  const LArFCAL_ID* m_fcalId = nullptr;
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  int m_groupingType = LArConditionsContainerBase::SingleGroup;

  // assign grouping type (only for LArConditionsContainer-based classes)
  StringProperty m_groupingName { this, "GroupingType", "Unknown" };
  // file name to be read
  StringProperty m_file { this, "File", "" };
  // choose whether use offline ID (default is online)
  BooleanProperty m_useOfflineIdentifier { this, "UseOfflineIdentifier", false };
  StringProperty m_chIdType { this, "ChannelIdType", "UNKNOWN" };
  StringProperty m_customKey { this, "CustomKey", "" };

  bool m_useCalibLines = false;
  DATA * m_dataclass = nullptr;

  StatusCode readFile() ;

  // define 'set' method for each foreseen data class:
  //---------------------------------------------------

  StatusCode set(LArCaliPulseParamsComplete* complete, HWIdentifier chid, int gain, const std::vector<float>& data) {
    complete->set(chid, gain, data[0], data[1], data[2], data[3], (short)data[4]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArDetCellParamsComplete* complete,   HWIdentifier chid, int gain, const std::vector<float>& data) {
    complete->set(chid, gain, data[0], data[1]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArPhysCaliTdiffComplete* complete,   HWIdentifier chid, int gain, const std::vector<float>& data) {
    complete->set(chid, gain, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArTdriftComplete* complete,          HWIdentifier chid, int /*gain*/, const std::vector<float>& data) {
    complete->set(chid, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArMphysOverMcalComplete* complete,   HWIdentifier chid, int gain, const std::vector<float>& data) {
    complete->set(chid, gain, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArRinjComplete* complete,   HWIdentifier chid, int /*gain*/, const std::vector<float>& data) {
    complete->set(chid, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArTshaperComplete* complete,   HWIdentifier chid, int /*gain*/, const std::vector<float>& data) {
    complete->set(chid, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArEMEC_CphiComplete* complete,   HWIdentifier chid, int /*gain*/, const std::vector<float>& data) {
    complete->set(chid, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArEMEC_HValphaComplete* complete,   HWIdentifier chid, int /*gain*/, const std::vector<float>& data) {
    complete->set(chid, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArEMEC_HVbetaComplete* complete,   HWIdentifier chid, int /*gain*/, const std::vector<float>& data) {
    complete->set(chid, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArCableLengthComplete* complete,   HWIdentifier chid, int /*gain*/, const std::vector<float>& data) {
    complete->set(chid, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
  StatusCode set(LArCableAttenuationComplete* complete,   HWIdentifier chid, int /*gain*/, const std::vector<float>& data) {
    complete->set(chid, data[0]) ;
    return StatusCode::SUCCESS ;
  } ;
};

#include "LArCalibTools/LArReadParamsFromFile.icc"

#endif
