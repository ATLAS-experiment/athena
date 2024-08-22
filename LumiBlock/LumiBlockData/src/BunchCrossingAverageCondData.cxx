#include "LumiBlockData/BunchCrossingAverageCondData.h"
#include "AthenaKernel/getMessageSvc.h"
#include <algorithm>






float BunchCrossingAverageCondData::GetBeam1IntensityAll(int channel) const {
  // if (ATH_UNLIKELY(bcid>=m_MAX_BCID)) return false;
  if(channel==0)
    return m_beam1IntensityAll;
  else if(channel==1)
    return m_beam1IntensityAll_fBCT;
  else if(channel==2)
    return m_beam1IntensityAll_DCCT;
  else if(channel==3)
    return m_beam1IntensityAll_DCCT24;
  else return 0;

}


float BunchCrossingAverageCondData::GetBeam2IntensityAll(int channel) const {
  // if (ATH_UNLIKELY(bcid>=m_MAX_BCID)) return false;
  if(channel==0)
    return m_beam2IntensityAll;
  else if(channel==1)
    return m_beam2IntensityAll_fBCT;
  else if(channel==2)
    return m_beam2IntensityAll_DCCT;
  else if(channel==3)
    return m_beam2IntensityAll_DCCT24;
  else return 0;

}


float BunchCrossingAverageCondData::GetBeam1Intensity(int channel) const {
  // if (ATH_UNLIKELY(bcid>=m_MAX_BCID)) return false;
  if(channel==0)
    return m_beam1Intensity;
  else if(channel==1)
    return m_beam1Intensity_fBCT;
  else if(channel==2)
    return m_beam1Intensity_DCCT;
  else if(channel==3)
    return m_beam1Intensity_DCCT24;
  else return 0;

}


float BunchCrossingAverageCondData::GetBeam2Intensity(int channel) const {
  // if (ATH_UNLIKELY(bcid>=m_MAX_BCID)) return false;
  if(channel==0)
    return m_beam2Intensity;
  else if(channel==1)
    return m_beam2Intensity_fBCT;
  else if(channel==2)
    return m_beam2Intensity_DCCT;
  else if(channel==3)
    return m_beam2Intensity_DCCT24;
  else return 0;

}


// errors



float BunchCrossingAverageCondData::GetBeam1IntensityAllSTD(int channel) const {
  // if (ATH_UNLIKELY(bcid>=m_MAX_BCID)) return false;
  if(channel==0)
    return m_beam1IntensityAllSTD;
  else if(channel==1)
    return m_beam1IntensityAllSTD_fBCT;
  else if(channel==2)
    return m_beam1IntensityAllSTD_DCCT;
  else if(channel==3)
    return m_beam1IntensityAllSTD_DCCT24;
  else return 0;

}


float BunchCrossingAverageCondData::GetBeam2IntensityAllSTD(int channel) const {
  // if (ATH_UNLIKELY(bcid>=m_MAX_BCID)) return false;
  if(channel==0)
    return m_beam2IntensityAllSTD;
  else if(channel==1)
    return m_beam2IntensityAllSTD_fBCT;
  else if(channel==2)
    return m_beam2IntensityAllSTD_DCCT;
  else if(channel==3)
    return m_beam2IntensityAllSTD_DCCT24;
  else return 0;

}


float BunchCrossingAverageCondData::GetBeam1IntensitySTD(int channel) const {
  // if (ATH_UNLIKELY(bcid>=m_MAX_BCID)) return false;
  if(channel==0)
    return m_beam1IntensitySTD;
  else if(channel==1)
    return m_beam1IntensitySTD_fBCT;
  else if(channel==2)
    return m_beam1IntensitySTD_DCCT;
  else if(channel==3)
    return m_beam1IntensitySTD_DCCT24;
  else return 0;

}


float BunchCrossingAverageCondData::GetBeam2IntensitySTD(int channel) const {
  // if (ATH_UNLIKELY(bcid>=m_MAX_BCID)) return false;
  if(channel==0)
    return m_beam2IntensitySTD;
  else if(channel==1)
    return m_beam2IntensitySTD_fBCT;
  else if(channel==2)
    return m_beam2IntensitySTD_DCCT;
  else if(channel==3)
    return m_beam2IntensitySTD_DCCT24;
  else return 0;

}

unsigned long long BunchCrossingAverageCondData::GetRunLB() const {
  // if (ATH_UNLIKELY(bcid>=m_MAX_BCID)) return false;
  return m_RunLB;
}








void BunchCrossingAverageCondData::SetBeam1IntensityAll( float  Beam1IntensityAll,int channel) {
   if(channel==0) 
    m_beam1IntensityAll = Beam1IntensityAll;
   else if(channel==1)
    m_beam1IntensityAll_fBCT = Beam1IntensityAll;
   else if(channel==2)
    m_beam1IntensityAll_DCCT = Beam1IntensityAll;
   else if(channel==3)
    m_beam1IntensityAll_DCCT24= Beam1IntensityAll;

}


void BunchCrossingAverageCondData::SetBeam2IntensityAll( float  Beam2IntensityAll,int channel) {
   if(channel==0) 
    m_beam2IntensityAll = Beam2IntensityAll;
   else if(channel==1)
    m_beam2IntensityAll_fBCT = Beam2IntensityAll;
   else if(channel==2)
    m_beam2IntensityAll_DCCT = Beam2IntensityAll;
   else if(channel==3)
    m_beam2IntensityAll_DCCT24= Beam2IntensityAll;

}



void BunchCrossingAverageCondData::SetBeam1Intensity( float  Beam1Intensity,int channel) {
   if(channel==0) 
    m_beam1Intensity = Beam1Intensity;
   else if(channel==1)
    m_beam1Intensity_fBCT = Beam1Intensity;
   else if(channel==2)
    m_beam1Intensity_DCCT = Beam1Intensity;
   else if(channel==3)
    m_beam1Intensity_DCCT24= Beam1Intensity;

}


void BunchCrossingAverageCondData::SetBeam2Intensity( float  Beam2Intensity,int channel) {
   if(channel==0) 
    m_beam2Intensity = Beam2Intensity;
   else if(channel==1)
    m_beam2Intensity_fBCT = Beam2Intensity;
   else if(channel==2)
    m_beam2Intensity_DCCT = Beam2Intensity;
   else if(channel==3)
    m_beam2Intensity_DCCT24= Beam2Intensity;

}

// errors

void BunchCrossingAverageCondData::SetBeam1IntensityAllSTD( float  Beam1IntensityAllSTD,int channel) {
   if(channel==0) 
    m_beam1IntensityAllSTD = Beam1IntensityAllSTD;
   else if(channel==1)
    m_beam1IntensityAllSTD_fBCT = Beam1IntensityAllSTD;
   else if(channel==2)
    m_beam1IntensityAllSTD_DCCT = Beam1IntensityAllSTD;
   else if(channel==3)
    m_beam1IntensityAllSTD_DCCT24= Beam1IntensityAllSTD;

}


void BunchCrossingAverageCondData::SetBeam2IntensityAllSTD( float  Beam2IntensityAllSTD,int channel) {
   if(channel==0) 
    m_beam2IntensityAllSTD = Beam2IntensityAllSTD;
   else if(channel==1)
    m_beam2IntensityAllSTD_fBCT = Beam2IntensityAllSTD;
   else if(channel==2)
    m_beam2IntensityAllSTD_DCCT = Beam2IntensityAllSTD;
   else if(channel==3)
    m_beam2IntensityAllSTD_DCCT24= Beam2IntensityAllSTD;

}



void BunchCrossingAverageCondData::SetBeam1IntensitySTD( float  Beam1IntensitySTD,int channel) {
   if(channel==0) 
    m_beam1IntensitySTD = Beam1IntensitySTD;
   else if(channel==1)
    m_beam1IntensitySTD_fBCT = Beam1IntensitySTD;
   else if(channel==2)
    m_beam1IntensitySTD_DCCT = Beam1IntensitySTD;
   else if(channel==3)
    m_beam1IntensitySTD_DCCT24= Beam1IntensitySTD;

}


void BunchCrossingAverageCondData::SetBeam2IntensitySTD( float  Beam2IntensitySTD,int channel) {
   if(channel==0) 
    m_beam2IntensitySTD = Beam2IntensitySTD;
   else if(channel==1)
    m_beam2IntensitySTD_fBCT = Beam2IntensitySTD;
   else if(channel==2)
    m_beam2IntensitySTD_DCCT = Beam2IntensitySTD;
   else if(channel==3)
    m_beam2IntensitySTD_DCCT24= Beam2IntensitySTD;

}
void BunchCrossingAverageCondData::SetRunLB( unsigned long long  RunLB) {
   m_RunLB = RunLB;
}


