/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODMuonSimHit/versions/MuonSimHitAuxContainer_v1.h"
namespace{
    static const std::string preFixStr{"MuSim_"};
}
#define SIM_AUXVARIABLE(VAR) \
   do { \
      static const std::string varName =preFixStr+#VAR; \
      static const auxid_t auxid = getAuxID(varName, VAR); \
      regAuxVar(auxid, varName, VAR); \
    } while (false);
namespace xAOD {
MuonSimHitAuxContainer_v1::MuonSimHitAuxContainer_v1()
    : AuxContainerBase() {
    AUX_MEASUREMENTVAR(localPosition, 3);
    AUX_MEASUREMENTVAR(localDirection, 3);
    SIM_AUXVARIABLE(stepLength);
    SIM_AUXVARIABLE(globalTime);
    SIM_AUXVARIABLE(pdgId);
    SIM_AUXVARIABLE(identifier);
    SIM_AUXVARIABLE(energyDeposit);
    SIM_AUXVARIABLE(kineticEnergy);    
    SIM_AUXVARIABLE(mcEventIndex);
    SIM_AUXVARIABLE(mcBarcode);
}
}  // namespace xAOD
#undef SIM_AUXVARIABLE