/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 *   @class RALEmec
 *   @brief Access the EMEC parameters from the geometry database.
 */

#ifndef LARGEORAL_RALEMEC_H
#define LARGEORAL_RALEMEC_H

#include "LArGeoCode/VDetectorParameters.h"

namespace LArGeo {

  class RALEmec : public VDetectorParameters {

  public:

    RALEmec();
    virtual ~RALEmec();

    virtual double GetValue(const std::string&, 
                            const int i0 = INT_MIN,
                            const int i1 = INT_MIN,
                            const int i2 = INT_MIN,
                            const int i3 = INT_MIN,
                            const int i4 = INT_MIN ) const override;

  private:


    class Clockwork;
    Clockwork *m_c;

    RALEmec (const RALEmec&);
    RALEmec& operator= (const RALEmec&);
  };

} // namespace LArGeo

#endif
