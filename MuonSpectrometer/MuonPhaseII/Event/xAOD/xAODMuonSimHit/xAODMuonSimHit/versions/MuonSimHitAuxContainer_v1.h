/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHIT_VERSION_MUONSIMHITAUXCONTAINER_V1
#define XAODMUONSIMHIT_VERSION_MUONSIMHITAUXCONTAINER_V1

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "AthLinks/ElementLink.h"
#include "GeneratorObjects/McEventCollection.h"
#include <array>
namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class MuonSimHitAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    MuonSimHitAuxContainer_v1();
    virtual ~MuonSimHitAuxContainer_v1() = default;
   private:
    /// @name Defining Mdt Drift Circle parameters
    /// @{
    std::vector<std::array<float, 3>> localPosition{};
    std::vector<std::array<float, 3>> localDirection{};
    std::vector<float> mass{};
    std::vector<float> globalTime{};
    std::vector<int> pdgId{};
    std::vector<Identifier::value_type> identifier{};
    std::vector<float> energyDeposit{};
    std::vector<float> kineticEnergy{};
    std::vector<float> stepLength{};
    
    /// Information needed to save the HEPMC particle link
    std::vector<unsigned short> mcEventIndex{};
    std::vector<unsigned int>  uniqueID{};
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::MuonSimHitAuxContainer_v1, xAOD::AuxContainerBase);
#endif
