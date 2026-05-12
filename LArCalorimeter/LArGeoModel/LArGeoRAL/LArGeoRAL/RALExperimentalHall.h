/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 *   @class RALExperimentalHall
 *   @brief Access the experimental hall parameters from the geometry database.
 */

#ifndef LARGEORAL_RALEXPERIMENTALHALL_H
#define LARGEORAL_RALEXPERIMENTALHALL_H

#include "LArGeoCode/VDetectorParameters.h"

namespace LArGeo {

  class RALExperimentalHall : public VDetectorParameters {

  public:

    RALExperimentalHall();
    virtual ~RALExperimentalHall();

    virtual double GetValue(std::string_view, 
                            const int i0 = INT_MIN,
                            const int i1 = INT_MIN,
                            const int i2 = INT_MIN,
                            const int i3 = INT_MIN,
                            const int i4 = INT_MIN ) const override;

  private:


    class Clockwork;
    Clockwork *m_c;

    RALExperimentalHall (const RALExperimentalHall&);
    RALExperimentalHall& operator= (const RALExperimentalHall&);
  };

} // namespace LArGeo

#endif
