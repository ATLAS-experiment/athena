/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLEXAMPLEDATA_EXAMPLEELECTRONAUXCONTAINER_V1_H
#define ATHENAPOOLEXAMPLEDATA_EXAMPLEELECTRONAUXCONTAINER_V1_H

#include "AthenaKernel/BaseInfo.h"
#include "xAODCore/AuxContainerBase.h"

namespace xAOD {

class ExampleElectronAuxContainer_v1 : public xAOD::AuxContainerBase {
 public:
  ExampleElectronAuxContainer_v1();

 private:
  AUXVAR_DECL(double, pt);
  AUXVAR_DECL(float, charge);
};

}  // namespace xAOD

// Declare inheritance to SG
SG_BASE(xAOD::ExampleElectronAuxContainer_v1, xAOD::AuxContainerBase);

#endif
