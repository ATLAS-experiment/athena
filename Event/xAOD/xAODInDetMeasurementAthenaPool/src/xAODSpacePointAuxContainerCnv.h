/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_SPACEPOINT_AUXCONTAINER_CNV_H
#define XAODINDETMEASUREMENT_SPACEPOINT_AUXCONTAINER_CNV_H

// Gaudi/Athena include(s):
#include "AthenaPoolCnvSvc/T_AthenaPoolAuxContainerCnv.h"

// EDM include(s):
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

typedef T_AthenaPoolAuxContainerCnv<xAOD::SpacePointAuxContainer> xAODSpacePointAuxContainerCnvBase;

class xAODSpacePointAuxContainerCnv :
  public xAODSpacePointAuxContainerCnvBase
{
public:
  xAODSpacePointAuxContainerCnv( ISvcLocator* svcLoc );

private:
  friend class CnvFactory< xAODSpacePointAuxContainerCnv >;

  virtual StatusCode initialize() override;

protected:
  virtual xAOD::SpacePointAuxContainer*
  createPersistentWithKey( xAOD::SpacePointAuxContainer* trans,
			   const std::string& key ) override;
};

#endif
