/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARGEORAL_RAL_H
#define LARGEORAL_RAL_H

#include "LArGeoCode/VDetectorParameters.h"
#include "LArGeoRAL/RALExperimentalHall.h"
#include "LArGeoRAL/RALEmec.h"
#include "LArGeoRAL/RALEmb.h"
#include "LArGeoRAL/RALHec.h"


namespace LArGeo {

  class RAL : public VDetectorParameters {

  public:
    RAL() = default;
    virtual ~RAL() = default;
    
    virtual double GetValue(const std::string&, 
                            const int i0 = INT_MIN,
                            const int i1 = INT_MIN,
                            const int i2 = INT_MIN,
                            const int i3 = INT_MIN,
                            const int i4 = INT_MIN ) const override;

  private:
    RALExperimentalHall m_ExpHall{};
    RALEmec             m_Emec{};
    RALEmb              m_Emb{};
    RALHec              m_Hec{};

  };

} // namespace LArGeo

#endif
