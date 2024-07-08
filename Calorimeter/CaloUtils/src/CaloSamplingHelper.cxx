/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


#include "CaloUtils/CaloSamplingHelper.h"
#include <algorithm>

using enum CaloSampling::CaloSample;

const std::map<std::string,CaloSampling::CaloSample> CaloSamplingHelper::m_lookUp{
  {"PreSamplerB",  PreSamplerB},
  {"EMB1",         EMB1},
  {"EMB2",         EMB2},
  {"EMB3",         EMB3},
  {"PreSamplerE",  PreSamplerE},
  {"EME1",         EME1},
  {"EME2",         EME2},
  {"EME3",         EME3},
  {"HEC0",         HEC0},
  {"HEC1",         HEC1},
  {"HEC2",         HEC2},
  {"HEC3",         HEC3},
  {"TileBar0",     TileBar0},
  {"TileBar1",     TileBar1},
  {"TileBar2",     TileBar2},
  {"TileGap1",     TileGap1},
  {"TileGap2",     TileGap2},
  {"TileGap3",     TileGap3},
  {"TileExt0",     TileExt0},
  {"TileExt1",     TileExt1},
  {"TileExt2",     TileExt2},
  {"FCal1",        FCAL0},
  {"FCal2",        FCAL1},
  {"FCal3",        FCAL2},
  {"MiniFCal0",    MINIFCAL0},
  {"MiniFCal1",    MINIFCAL1},
  {"MiniFCal2",    MINIFCAL2},
  {"MiniFCal3",    MINIFCAL3}
};

const std::string CaloSamplingHelper::m_unknown="Unknown";

const std::string& CaloSamplingHelper::getSamplingName(const CaloSampling::CaloSample theSample) {
  //Linear search for the value in the std::map m_lookUp. 
  //Slow, but acceptable because: 
  //  1. The map has only 28 entries 
  //  2. It's and integer-comparision 
  //  3. The method is only use for log-output (eg not too often)

  //Implementation uses a C++11 lambda function
  //[&theSample] defines the lambda function and says that it depends on the external variable theSampling (captured by reference)
  //(std::pair<std::string,CaloSample> i) is the parameter of the function
  //{return (i.second==theSample);} Is the code of the function
  auto it=std::find_if(m_lookUp.begin(),m_lookUp.end(),[&theSample](const std::pair<std::string,CaloSampling::CaloSample>& i) {return (i.second==theSample);});
  if (it==m_lookUp.end())
    return m_unknown;
  else
    return it->first;
}

CaloSampling::CaloSample 
CaloSamplingHelper::getSamplingId(const std::string& name)
{
  return m_lookUp.find(name) != m_lookUp.end()
    ? (*m_lookUp.find(name)).second
    : CaloSampling::Unknown;
}

bool CaloSamplingHelper::getSamplings(
    const std::vector<CaloCell_ID::SUBCALO>& theCalos,
    std::vector<CaloSampling::CaloSample>& theSamplings) {

  const size_t oldSize = theSamplings.size();
  for (auto fCalo : theCalos) {
    switch (fCalo) {
      case CaloCell_ID::LAREM:
        theSamplings.push_back(PreSamplerB);
        theSamplings.push_back(EMB1);
        theSamplings.push_back(EMB2);
        theSamplings.push_back(EMB3);
        theSamplings.push_back(PreSamplerE);
        theSamplings.push_back(EME1);
        theSamplings.push_back(EME2);
        theSamplings.push_back(EME3);
        break;
      case CaloCell_ID::LARHEC:
        theSamplings.push_back(HEC0);
        theSamplings.push_back(HEC1);
        theSamplings.push_back(HEC2);
        theSamplings.push_back(HEC3);
        break;
      case CaloCell_ID::LARFCAL:
        theSamplings.push_back(FCAL0);
        theSamplings.push_back(FCAL1);
        theSamplings.push_back(FCAL2);
        break;
      case CaloCell_ID::TILE:
        theSamplings.push_back(TileBar0);
        theSamplings.push_back(TileBar1);
        theSamplings.push_back(TileBar2);
        theSamplings.push_back(TileExt0);
        theSamplings.push_back(TileExt1);
        theSamplings.push_back(TileExt2);
        theSamplings.push_back(TileGap1);
        theSamplings.push_back(TileGap2);
        theSamplings.push_back(TileGap3);
        break;
      case CaloCell_ID::LARMINIFCAL:
        theSamplings.push_back(MINIFCAL0);
        theSamplings.push_back(MINIFCAL1);
        theSamplings.push_back(MINIFCAL2);
        theSamplings.push_back(MINIFCAL3);
        break;
      default:
        break;
    }
  }
  return oldSize < theSamplings.size();
}

bool CaloSamplingHelper::getSamplings( const CaloCell_ID::SUBCALO& theCalo, std::vector<CaloSampling::CaloSample>& theSamplings) {
  const std::vector<CaloCell_ID::SUBCALO> caloVector = {theCalo};
  return getSamplings(caloVector, theSamplings);
}

bool CaloSamplingHelper::getCalos(const std::vector<CaloSampling::CaloSample>& theSamplings, std::vector<CaloCell_ID::SUBCALO>& theCalos) {
  const size_t oldSize = theCalos.size();
  for (auto fSample : theSamplings) {
    CaloCell_ID::SUBCALO theCaloId = CaloCell_ID::NOT_VALID;
    switch (fSample) {
      case PreSamplerB:
      case EMB1:
      case EMB2:
      case EMB3:
      case PreSamplerE:
      case EME1:
      case EME2:
      case EME3:
        theCaloId = CaloCell_ID::LAREM;
        break;
      case HEC0:
      case HEC1:
      case HEC2:
      case HEC3:
        theCaloId = CaloCell_ID::LARHEC;
        break;
      case FCAL0:
      case FCAL1:
      case FCAL2:
        theCaloId = CaloCell_ID::LARFCAL;
        break;
      case TileBar0:
      case TileBar1:
      case TileBar2:
      case TileGap1:
      case TileGap2:
      case TileGap3:
      case TileExt0:
      case TileExt1:
      case TileExt2:
        theCaloId = CaloCell_ID::TILE;
        break;
      case MINIFCAL0:
      case MINIFCAL1:
      case MINIFCAL2:
      case MINIFCAL3:
        theCaloId = CaloCell_ID::LARMINIFCAL;
        break;
      default:
        break;
    }
    // check if not there yet (actually a std::set would be better than a
    // vector)
    if (std::find(theCalos.begin(), theCalos.end(), theCaloId) ==  theCalos.end()) {
      theCalos.push_back(theCaloId);
    }
  }
  return theCalos.size() > oldSize;
}
