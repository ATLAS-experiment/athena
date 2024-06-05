/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_TRACKSTATE_AUXCONTAINER_CNV_H
#define XAODINDETMEASUREMENT_TRACKSTATE_AUXCONTAINER_CNV_H

// Gaudi/Athena include(s):
#include "AthenaPoolCnvSvc/T_AthenaPoolAuxContainerCnv.h"

// EDM include(s):
#include "xAODTracking/TrackStateAuxContainer.h"

typedef T_AthenaPoolAuxContainerCnv<xAOD::TrackStateAuxContainer> xAODTrackStateAuxContainerCnvBase;

class xAODTrackStateAuxContainerCnv :
  public xAODTrackStateAuxContainerCnvBase
{
public:
  xAODTrackStateAuxContainerCnv( ISvcLocator* svcLoc );

private:
  friend class CnvFactory< xAODTrackStateAuxContainerCnv >;

  virtual StatusCode initialize() override;

protected:
  virtual xAOD::TrackStateAuxContainer*
  createPersistentWithKey( xAOD::TrackStateAuxContainer* trans,
                           const std::string& key ) override;
};

#endif
