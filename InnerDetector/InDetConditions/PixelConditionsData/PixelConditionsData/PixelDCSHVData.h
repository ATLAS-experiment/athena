/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsData/PixelDCSHVData.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date November, 2019
 * @brief Store pixel HV data in PixelDCSHVData.
 */

#ifndef PIXELDCSHVDATA_H
#define PIXELDCSHVDATA_H

#include "PixelConditionsData/SingleConditionsDatum.h"
#include "AthenaKernel/CLASS_DEF.h"

#include "AthenaKernel/CondCont.h"
//class template parameters: lo limit, hi limit, default, invalid
class PixelDCSHVData { 
  public:
    void defaultVoltage(float v){m_impl.defaultValue(v);}
    void useDefault(bool b){m_impl.useDefaultValue(b);}
    void setChannelToDefault(int chanNum){m_impl.setChanToDefault(chanNum);}
    void setBiasVoltage(int chanNum, float value){ m_impl.setValue(chanNum,value);}
    //
    float getBiasVoltage(const int chanNum) const{ return m_impl.getValue(chanNum);}
    bool  useDefault() const {return m_impl.useDefaultValue();}
    float defaultVoltage() const {return m_impl.defaultValue();}
  private:
  //class template parameters: lo limit, hi limit, default, invalid
   SingleConditionsDatum<float,-1000.f, 1000.f, 150.f, 0.f> m_impl;
};

CLASS_DEF( PixelDCSHVData , 345932813 , 1 )

CONDCONT_DEF( PixelDCSHVData, 578988313 );

#endif
