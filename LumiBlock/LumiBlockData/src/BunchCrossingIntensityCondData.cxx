#include "LumiBlockData/BunchCrossingIntensityCondData.h"
#include "AthenaKernel/getMessageSvc.h"
#include <algorithm>



float  BunchCrossingIntensityCondData::GetBeam1IntensityBCID(const bcid_type bcid, int channel) const {
  if (bcid>=m_MAX_BCID) [[unlikely]] return 0;
  if (channel==0)
    return m_beam1Intensity.at(bcid);
  else if (channel==1)
    return m_beam1Intensity_fBCT.at(bcid);
  else 
    return 0;
}


float  BunchCrossingIntensityCondData::GetBeam2IntensityBCID(const bcid_type bcid, int channel) const {
  if (bcid>=m_MAX_BCID) [[unlikely]] return 0;
  if (channel==0)
    return m_beam2Intensity.at(bcid);
  else if (channel==1)
    return m_beam2Intensity_fBCT.at(bcid);
  else 
    return 0;
}





const std::vector<float>&  BunchCrossingIntensityCondData::GetBeam1IntensityPerBCIDVector(int channel) const {
  if(channel==0)
    return m_beam1Intensity;
  // else if(channel==1)
  else
    return m_beam1Intensity_fBCT;

}


const std::vector<float>&  BunchCrossingIntensityCondData::GetBeam2IntensityPerBCIDVector(int channel) const {
  if(channel==0)
    return m_beam2Intensity;
  else //if(channel==1)
    return m_beam2Intensity_fBCT;

}


float BunchCrossingIntensityCondData::GetBeam1IntensityAll(int channel) const {
  if(channel==0)
    return m_beam1IntensityAll;
  else if(channel==1)
    return m_beam1IntensityAll_fBCT;
  else return 0;

}


float BunchCrossingIntensityCondData::GetBeam2IntensityAll(int channel) const {
  if(channel==0)
    return m_beam2IntensityAll;
  else if(channel==1)
    return m_beam2IntensityAll_fBCT;
  else return 0;

}


unsigned long long BunchCrossingIntensityCondData::GetRunLB() const {
  return m_RunLB;
}







void BunchCrossingIntensityCondData::setBeam1IntensityPerBCIDVector( std::vector<float>&& val,int channel) {
  if (channel==0)
   m_beam1Intensity = std::move(val);
  else if (channel==1)
    m_beam1Intensity_fBCT = std::move(val);
}


void BunchCrossingIntensityCondData::setBeam2IntensityPerBCIDVector( std::vector<float>&& val,int channel) {
  if (channel==0)
   m_beam2Intensity = std::move(val);
  else if (channel==1)
    m_beam2Intensity_fBCT = std::move(val);
}


void BunchCrossingIntensityCondData::SetBeam1IntensityAll( float  Beam1IntensityAll,int channel) {
   if(channel==0) 
    m_beam1IntensityAll = Beam1IntensityAll;
   else if(channel==1)
    m_beam1IntensityAll_fBCT = Beam1IntensityAll;
}


void BunchCrossingIntensityCondData::SetBeam2IntensityAll( float  Beam2IntensityAll,int channel) {
   if(channel==0) 
    m_beam2IntensityAll = Beam2IntensityAll;
   else if(channel==1)
    m_beam2IntensityAll_fBCT = Beam2IntensityAll;
}


void BunchCrossingIntensityCondData::SetRunLB( unsigned long long  RunLB) {
   m_RunLB = RunLB;
}

