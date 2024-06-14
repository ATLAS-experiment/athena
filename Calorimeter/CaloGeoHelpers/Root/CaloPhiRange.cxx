/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Liquid Argon detector description package
 -----------------------------------------
 ***************************************************************************/


#include "CaloGeoHelpers/CaloPhiRange.h"

double
CaloPhiRange::fix ( double phi )
{
  if (phi < m_phi_min) return (phi+m_twopi);
  if (phi > m_phi_max) return (phi-m_twopi);
  return phi;
}

double
CaloPhiRange::diff ( double phi1, double phi2 )
{
  double res = fix(phi1) - fix(phi2);
  return fix(res);
}