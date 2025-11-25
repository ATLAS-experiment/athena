/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOINTERFACE_ICALOCLUSTERMLCALIBTOOLLITE_H
#define CALOINTERFACE_ICALOCLUSTERMLCALIBTOOLLITE_H


#include "GaudiKernel/IAlgTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h" //typedef
#include <vector>

class ICaloClusterMLCalibToolLite : public virtual IAlgTool
{
public:
  DeclareInterfaceID(ICaloClusterMLCalibToolLite, 1, 0);

  virtual StatusCode inference(const xAOD::CaloClusterContainer &clusters,
                               int nPrimVtx,
                               float avgMu,
                               std::vector<double> &clusterE_ML,
                               std::vector<double> &clusterE_ML_Unc) const = 0;
};
#endif // CALOINTERFACE_ICALOCLUSTERMLCALIBTOOLLITE_H
