/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFNEWP_H
#define TRKVKALVRTCORE_CFNEWP_H

namespace Trk {

void cfnewp(const long int ich, const double *parold, const double *ref,
            double *s, double *parnew, double *per);

}  // namespace Trk

#endif

