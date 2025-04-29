/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsData/PixelDCSTempData.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date November, 2019
 * @brief Store pixel temperature data in PixelDCSTempData.
 */

#ifndef PIXELDCSTEMPDATA_H
#define PIXELDCSTEMPDATA_H

#include "PixelConditionsData/SingleConditionsDatum.h"

#include "AthenaKernel/CLASS_DEF.h"

#include "AthenaKernel/CondCont.h"
class PixelDCSTempData {
  public:
    void setTemperature(int chanNum, float value){m_impl.setValue(chanNum, value);}
    float getTemperature(int chanNum) const{ return m_impl.getValue(chanNum);}
  private:
    //class template parameters: lo limit, hi limit, default, invalid
    SingleConditionsDatum<float, -1000.f, 1000.f, -7.f, -7.f> m_impl;
};

CLASS_DEF( PixelDCSTempData , 345932822 , 1 )

CONDCONT_DEF( PixelDCSTempData, 578988322 );

#endif
