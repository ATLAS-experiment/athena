/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCLUSTERCORRECTION_ICALOCLUSTERMLCALIBTOOLLITE_H
#define CALOCLUSTERCORRECTION_ICALOCLUSTERMLCALIBTOOLLITE_H

#include "GaudiKernel/IAlgTool.h"

class ICaloClusterMLCalibToolLite : public virtual IAlgTool
{
public:
    DeclareInterfaceID(ICaloClusterMLCalibToolLite, 1, 0);

    virtual StatusCode inference(const xAOD::CaloClusterContainer &clusters, const int &nPrimVtx, const double &avgMu, std::vector<double> &clusterE_ML, std::vector<double> &clusterE_ML_Unc) const = 0;
};
#endif // CALOCLUSTERCORRECTION_ICALOCLUSTERMLCALIBTOOLLITE_H
