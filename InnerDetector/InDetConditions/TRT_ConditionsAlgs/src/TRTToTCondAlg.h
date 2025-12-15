/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTTOTCONDALG_H
#define TRTTOTCONDALG_H

#include <map>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListVec.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "Gaudi/Property.h"
#include "TRT_ConditionsData/TRTDedxcorrection.h"

class TRTToTCondAlg : public AthCondAlgorithm
{
 public:
  TRTToTCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TRTToTCondAlg() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;
  enum EDataBaseType {kOldDB,kNewDB,kNewDBOccCorr};
  StatusCode update1(TRTDedxcorrection& Dedxcorrection, const CondAttrListVec* channel_values) const;
  StatusCode update2(TRTDedxcorrection& Dedxcorrection, const CondAttrListCollection* attrListColl ) const;
 
 protected:
  static void updateOldDBParameters(TRTDedxcorrection& Dedxcollection, std::map<std::string,std::vector<float> > &result_dict) ;
  static void updateNewDBParameters(TRTDedxcorrection& Dedxcorrection, std::map<std::string,std::vector<float> > &result_dict) ;
  static void updateOccupancyCorrectionParameters(TRTDedxcorrection & Dedxcorrection, std::map<std::string,std::vector<float> > &result_dict) ;

 private:
  SG::ReadCondHandleKey<CondAttrListVec> m_VecReadKey{this,"ToTVecReadKey","/TRT/Calib/ToT/ToTVectors","ToTVec in-key"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_ValReadKey{this,"ToTValReadKey","/TRT/Calib/ToT/ToTValue","ToTVal in-key"};
  SG::WriteCondHandleKey<TRTDedxcorrection> m_WriteKey{this,"ToTWriteKey","Dedxcorrection","Dedxcorrection out-key"};

  static const std::vector<std::string> m_dictNamesOldDB;
  static const std::vector<std::string> m_dictNamesNewDB;
};
#endif
