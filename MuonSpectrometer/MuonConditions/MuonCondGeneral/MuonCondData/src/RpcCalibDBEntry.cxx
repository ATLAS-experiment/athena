/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/RpcCalibData.h"
#include "MuonCondData/RpcCalibDBEntry.h"
#include "MuonCondData/RpcCondParType.h"

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <charconv>

namespace {

class NumberParser{
public:
  explicit NumberParser(std::string_view text)
    : m_text{text}
  {}

  template <class T>
  T next(){
    skipSpaces();

    T value{};
    const char* first = m_text.data();
    const char* last = m_text.data() + m_text.size();

    const auto [ptr, ec] = std::from_chars(first, last, value);
    if (ec != std::errc{} || ptr == first) {
      throw std::runtime_error("RpcCalibDBEntry: failed to parse numeric payload");
    }

    m_text.remove_prefix(static_cast<std::size_t>(ptr - first));
    return value;
  }

private:
  void skipSpaces(){
    const auto pos = m_text.find_first_not_of(" \t\n\r");
    if (pos == std::string_view::npos) {
      m_text = {};
    } else {
      m_text.remove_prefix(pos);
    }
  }

  std::string_view m_text;
};

}

namespace MuonCalib{


  RpcCalibDBEntry::RpcCalibDBEntry(Identifier gapID, std::string_view payLoad)
  : m_theGap(gapID)
{
  auto take_until = [&](std::string_view& sv) -> std::string_view {
    const std::string_view delim = "END ";
    const auto pos = sv.find(delim);
    if (pos == std::string_view::npos) {
      throw std::runtime_error("RpcCalibDBEntry: malformed payload (missing 'END ')");
    }
    auto out = sv.substr(0, pos);
    sv.remove_prefix(pos + delim.size()); // avoids overflow math
    return out;
  };

  const auto etaRec  = take_until(payLoad);
  const auto etaDet  = take_until(payLoad);
  const auto phiRec1 = take_until(payLoad);
  const auto phiRec2 = take_until(payLoad);
  const auto phiDet1 = take_until(payLoad);
  const auto phiDet2 = take_until(payLoad);

  initData(std::string(etaRec), std::string(etaDet),
           std::string(phiRec1), std::string(phiRec2),
           std::string(phiDet1), std::string(phiDet2));
}

  void
  RpcCalibDBEntry::initData(std::string_view etaRec, std::string_view etaDet,
    std::string_view phiRec1, std::string_view phiRec2, std::string_view phiDet1,
    std::string_view phiDet2){
    NumberParser etaRecParser{etaRec};
    NumberParser etaDetParser{etaDet};
    NumberParser phiRec1Parser{phiRec1};
    NumberParser phiRec2Parser{phiRec2};
    NumberParser phiDet1Parser{phiDet1};
    NumberParser phiDet2Parser{phiDet2};
  
    m_nRecEta = etaRecParser.next<int>();
    m_nDetEta = etaDetParser.next<int>();
    m_nRecPhi1 = phiRec1Parser.next<int>();
    m_nRecPhi2 = phiRec2Parser.next<int>();
    m_nDetPhi1 = phiDet1Parser.next<int>();
    m_nDetPhi2 = phiDet2Parser.next<int>();
  
    for (int k = 0; k < m_nRecEta; ++k) {
      const float eff = etaRecParser.next<float>();
      const float errEff = etaRecParser.next<float>();
      const float res1 = etaRecParser.next<float>();
      const float errRes1 = etaRecParser.next<float>();
      const float res2 = etaRecParser.next<float>();
      const float errRes2 = etaRecParser.next<float>();
      const float resX = etaRecParser.next<float>();
      const float errResX = etaRecParser.next<float>();
      const float time = etaRecParser.next<float>();
      const float errTime = etaRecParser.next<float>();
  
      const float noise = etaDetParser.next<float>();
      const float errNoise = etaDetParser.next<float>();
      const float noiseC = etaDetParser.next<float>();
      const float errNoiseC = etaDetParser.next<float>();
      const float cs = etaDetParser.next<float>();
      const float errCs = etaDetParser.next<float>();
  
      auto etaData = std::make_unique<RpcCalibData>();
  
      etaData->setId(k);
      etaData->setEff(eff);
      etaData->setErrEff(errEff);
      etaData->setRes1(res1);
      etaData->setRes2(res2);
      etaData->setResX(resX);
      etaData->setErrRes1(errRes1);
      etaData->setErrRes2(errRes2);
      etaData->setErrResX(errResX);
      etaData->setTime(time);
      etaData->setErrTime(errTime);
      etaData->setNoise(noise);
      etaData->setNoiseC(noiseC);
      etaData->setErrNoise(errNoise);
      etaData->setErrNoiseC(errNoiseC);
      etaData->setCs(cs);
      etaData->setErrCs(errCs);
  
      m_theEtaData.push_back(std::move(etaData));
    }
  
    for (int k = 0; k < m_nRecPhi1; ++k) {
      const float eff = phiRec1Parser.next<float>();
      const float res1 = phiRec1Parser.next<float>();
      const float res2 = phiRec1Parser.next<float>();
      const float resX = phiRec1Parser.next<float>();
      const float time = phiRec1Parser.next<float>();
  
      const float errEff = phiRec2Parser.next<float>();
      const float errRes1 = phiRec2Parser.next<float>();
      const float errRes2 = phiRec2Parser.next<float>();
      const float errResX = phiRec2Parser.next<float>();
      const float errTime = phiRec2Parser.next<float>();
  
      const float noise = phiDet1Parser.next<float>();
      const float errNoise = phiDet1Parser.next<float>();
      const float noiseC = phiDet1Parser.next<float>();
      const float errNoiseC = phiDet1Parser.next<float>();
      const float cs = phiDet1Parser.next<float>();
      const float errCs = phiDet1Parser.next<float>();
  
      auto phiData = std::make_unique<RpcCalibData>();
  
      phiData->setId(k);
      phiData->setEff(eff);
      phiData->setErrEff(errEff);
      phiData->setRes1(res1);
      phiData->setRes2(res2);
      phiData->setResX(resX);
      phiData->setErrRes1(errRes1);
      phiData->setErrRes2(errRes2);
      phiData->setErrResX(errResX);
      phiData->setTime(time);
      phiData->setErrTime(errTime);
      phiData->setNoise(noise);
      phiData->setNoiseC(noiseC);
      phiData->setErrNoise(errNoise);
      phiData->setErrNoiseC(errNoiseC);
      phiData->setCs(cs);
      phiData->setErrCs(errCs);
  
      m_thePhiData.push_back(std::move(phiData));
    }
  }
  // initialize from db columns
  
  RpcCalibDBEntry::RpcCalibDBEntry(Identifier gapID, const std::string& etaRec, const std::string& etaDet, const std::string& phiRec1, const std::string& phiRec2, const std::string& phiDet1, const std::string& phiDet2 ):m_nRecEta(0),m_nDetEta(0), m_nRecPhi1(0),m_nRecPhi2(0),m_nDetPhi1(0),m_nDetPhi2(0),m_theGap(gapID)
  {
    
    this->initData(etaRec, etaDet,phiRec1,phiRec2,phiDet1,phiDet2);


  }


  const Identifier& RpcCalibDBEntry::getGapID() const {    
    return m_theGap;
    
  }
  
  void RpcCalibDBEntry::getColumns(std::string &recEta, std::string &detEta,std::string &recPhi1, std::string &recPhi2, std::string &detPhi1,std::string &detPhi2) const{
    

    std::ostringstream recEta_str,detEta_str,recPhi1_str,recPhi2_str,detPhi1_str,detPhi2_str;

    float eff, errEff, res1, res2, resX, errRes1, errRes2, errResX, time, errTime, noise, errNoise, noiseC, errNoiseC, cs, errCs;
    
    recEta_str<<m_nRecEta<<' ';
    detEta_str<<m_nDetEta<<' ';

    recPhi1_str<<m_nRecPhi1<<' ';
    detPhi1_str<<m_nDetPhi1<<' ';

    recPhi2_str<<m_nRecPhi2<<' ';
    detPhi2_str<<m_nDetPhi2<<' ';



    for(int k=0;k<m_nRecEta;k++){
      
      const auto& theData = m_theEtaData[k];
      
      eff=theData->getEff();
      errEff=theData->getErrEff();
      res1=theData->getRes1();
      res2=theData->getRes2();
      resX=theData->getResX();
      errRes1=theData->getErrRes1();
      errRes2=theData->getErrRes2();
      errResX=theData->getErrResX();
      time=theData->getTime();
      errTime=theData->getErrTime();
      noise=theData->getNoise();
      noiseC=theData->getNoiseC();
      errNoise=theData->getErrNoise();
      errNoiseC=theData->getErrNoiseC();
      cs=theData->getCs();
      errCs=theData->getErrCs();

      recEta_str<< eff<< ' '<<errEff<< ' '<<res1<< ' '<<errRes1<< ' '<<res2<< ' '<<errRes2<< ' '<<resX<< ' '<<errResX<< ' '<<time<< ' '<<errTime<< ' ';
      detEta_str<<noise<< ' '<<errNoise<< ' '<<noiseC<< ' '<<errNoiseC<< ' '<<cs<< ' '<<errCs<< ' ';


    }

    for(int k=0;k<m_nRecPhi1;k++){
      
      const auto& theData = m_thePhiData[k];
      
      eff=theData->getEff();
      errEff=theData->getErrEff();
      res1=theData->getRes1();
      res2=theData->getRes2();
      resX=theData->getResX();
      errRes1=theData->getErrRes1();
      errRes2=theData->getErrRes2();
      errResX=theData->getErrResX();
      time=theData->getTime();
      errTime=theData->getErrTime();
      noise=theData->getNoise();
      noiseC=theData->getNoiseC();
      errNoise=theData->getErrNoise();
      errNoiseC=theData->getErrNoiseC();
      cs=theData->getCs();
      errCs=theData->getErrCs();

      recPhi1_str<<eff<<' ' <<res1<<' ' <<res2<<' ' <<resX<<' ' <<time<< ' ';
      recPhi2_str<<errEff<<' ' <<errRes1<<' ' <<errRes2<<' ' <<errResX<<' ' <<errTime<< ' ';
      detPhi1_str<<noise<<' ' <<errNoise<<' ' <<noiseC<<' ' <<errNoiseC<<' ' <<cs<<' ' <<errCs<< ' ';


    }

    // give back strings

    recEta=recEta_str.str();
    detEta=detEta_str.str();
    recPhi1=recPhi1_str.str();
    recPhi2=recPhi2_str.str();
    detPhi1=detPhi1_str.str();
    detPhi2=detPhi2_str.str();

  }

}// End namespace MuonCalib
