/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 *   @class RALEmb
 *   @brief Access the EMB parameters from the geometry database.
 */

#ifndef LARGEORAL_RALEMB_H
#define LARGEORAL_RALEMB_H

#include "LArGeoCode/VDetectorParameters.h"

namespace LArGeo {

  class RALEmb : public VDetectorParameters {

  public:

    RALEmb();
    virtual ~RALEmb();

    virtual double GetValue(std::string_view, 
                            const int i0 = INT_MIN,
                            const int i1 = INT_MIN,
                            const int i2 = INT_MIN,
                            const int i3 = INT_MIN,
                            const int i4 = INT_MIN ) const override;

  private:


    class Clockwork;
    Clockwork *m_c;
    bool m_oldDB{false};
    bool m_oldSagging{false};
    bool m_oldContract{false};

    RALEmb (const RALEmb&);
    RALEmb& operator= (const RALEmb&);
  };

} // namespace LArGeo

#endif
