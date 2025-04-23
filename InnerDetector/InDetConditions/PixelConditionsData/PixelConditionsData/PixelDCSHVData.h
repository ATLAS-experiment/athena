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

#include "AthenaKernel/CLASS_DEF.h"
#include <unordered_map>

#include "AthenaKernel/CondCont.h"

class PixelDCSHVData {
  public:
    void defaultVoltage(float v);
    void useDefault(bool b);
    void setChannelToDefault(int chanNum);
    void setBiasVoltage(int chanNum, float value);
    //
    float getBiasVoltage(const int chanNum) const;
    bool  useDefault() const {return m_alwaysUseDefault;}
    float defaultVoltage() const {return m_defaultVoltage;}

  private:
    typedef std::unordered_map<int, float> FloatConditions;
    static constexpr std::pair<float, float> m_valueLimits{-1000.f, 1000.f};
    float m_defaultVoltage{150.f};
    bool m_alwaysUseDefault{};
    FloatConditions  m_biasVoltage;
};

CLASS_DEF( PixelDCSHVData , 345932813 , 1 )

CONDCONT_DEF( PixelDCSHVData, 578988313 );

#endif
