/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonABLineJsonDumpAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "nlohmann/json.hpp"
#include <fstream>

MuonABLineJsonDumpAlg::MuonABLineJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator):
      AthAlgorithm{name, pSvcLocator} {}

StatusCode MuonABLineJsonDumpAlg::initialize() {
    ATH_CHECK(m_readALineKey.initialize());
    ATH_CHECK(m_readBLineKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
  }
StatusCode MuonABLineJsonDumpAlg::execute(){
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadCondHandle<ALineContainer> aLineContainer{m_readALineKey, ctx};
  if (!aLineContainer.isValid()){
    ATH_MSG_FATAL("Failed to load ALine container "<<m_readALineKey.fullKey());
    return StatusCode::FAILURE;
  }
  SG::ReadCondHandle<BLineContainer> bLineContainer{m_readBLineKey, ctx};
  if (!aLineContainer.isValid()){
    ATH_MSG_FATAL("Failed to load BLine container "<<m_readBLineKey.fullKey());
    return StatusCode::FAILURE;
  }
  
  std::ofstream ostr{m_jsonFile};
  if (!ostr.good()) {
     ATH_MSG_FATAL("Failed to create output file "<<m_jsonFile);
     return StatusCode::FAILURE;
  }
  ostr<<"[\n";
  unsigned int nLines{0};
  for (const ALinePar& aLine : **aLineContainer) {
    ++nLines;
    ostr<<"    {\n";
    /// Identifier of the A Line
    ostr<<"     \"typ\": \""<<aLine.AmdbStation()<<"\",\n";
    ostr<<"     \"jzz\": "<<aLine.AmdbEta()<<", \n";
    ostr<<"     \"jff\": "<<aLine.AmdbPhi()<<", \n";
    ostr<<"     \"job\": "<<aLine.AmdbJob()<<", \n";
    /// ALine parameter
    using APar = ALinePar::Parameter;
    ostr<<"     \"svalue\": "<<aLine.getParameter(APar::transS)<<", \n";
    ostr<<"     \"zvalue\": "<<aLine.getParameter(APar::transZ)<<", \n";
    ostr<<"     \"tvalue\": "<<aLine.getParameter(APar::transT)<<", \n";
    ostr<<"     \"tsv\": "<<aLine.getParameter(APar::rotS)<<", \n";
    ostr<<"     \"tzv\": "<<aLine.getParameter(APar::rotZ)<<", \n";
    ostr<<"     \"ttv\": "<<aLine.getParameter(APar::rotT);
    /// BLine parameter
    BLineContainer::const_iterator itr = bLineContainer->find(aLine.identify());
    if (itr == bLineContainer->end()) {
      /// Check that the last entry does not have a comma
      ostr<<"\n    }"<< (nLines != aLineContainer->size() ? "," : "")<<"\n";
      continue;
    }
    ostr<<",\n";
    using BPar = BLinePar::Parameter;
    const BLinePar& bLine = (*itr);
    ostr<<"     \"bz\": "<<bLine.getParameter(BPar::bz)<<",\n";
    ostr<<"     \"bp\": "<<bLine.getParameter(BPar::bp)<<",\n";
    ostr<<"     \"bn\": "<<bLine.getParameter(BPar::bn)<<",\n";
    ostr<<"     \"sp\": "<<bLine.getParameter(BPar::sp)<<",\n";
    ostr<<"     \"sn\": "<<bLine.getParameter(BPar::sn)<<",\n";
    ostr<<"     \"tw\": "<<bLine.getParameter(BPar::tw)<<",\n";
    ostr<<"     \"pg\": "<<bLine.getParameter(BPar::pg)<<",\n";
    ostr<<"     \"tr\": "<<bLine.getParameter(BPar::tr)<<",\n";
    ostr<<"     \"eg\": "<<bLine.getParameter(BPar::eg)<<",\n";
    ostr<<"     \"ep\": "<<bLine.getParameter(BPar::ep)<<",\n";
    ostr<<"     \"en\": "<<bLine.getParameter(BPar::en)<<",\n";
    /// No idea what xAtlas & yAtlas are in this parameter book
    ostr<<"     \"xAtlas\": 0 ,\n";
    ostr<<"     \"yAtlas\": 0 ,\n";
    ostr<<"     \"hwElement\": \"";
    ostr<<m_idHelperSvc->stationNameString(bLine.identify());
    const int stEta = m_idHelperSvc->stationEta(bLine.identify());
    ostr<<std::abs(stEta)<<(stEta > 0 ? "A" : "C");
    ostr<<m_idHelperSvc->stationPhi(bLine.identify());
    ostr<<"\"\n";
    ostr<<"    }"<< (nLines != aLineContainer->size() ? "," : "")<<"\n";
  }
  ostr<<"]"<<std::endl;
  ostr.close();
  
  
  return StatusCode::SUCCESS;
}
