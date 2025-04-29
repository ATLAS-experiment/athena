/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef IDTRACKCALODEPOSITSDECORATORALG_H_
#define IDTRACKCALODEPOSITSDECORATORALG_H_

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include "ICaloTrkMuIdTools/ITrackDepositInCaloTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "xAODMuon/MuonContainer.h"
///     Algorithm to store decorate the IParticle with all energy deposits in the calorimeter needed for post fine tuning of the cut-based
///     calorimeter tagging working points. The tool adds the following three decorations
///           Vector containing all energy deposits                               std::vector<float>    CaloDeposits
///           Vector containing all energy losses                                 std::vector<float>    CaloElosses
///           Vector declaring the type of the deposit/loss (EMB1,HEC0,etc.)      std::vector<unit_16>  CaloDepType
///     Deposits can be either saved to the Muon particle itself or to the associated ID track particle
///     However, in the latter case, the first ID track from the container is always decorated to ensure file integrety

namespace DerivationFramework {

class IDTrackCaloDepositsDecoratorAlg : public AthReentrantAlgorithm {
public:
    IDTrackCaloDepositsDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual ~IDTrackCaloDepositsDecoratorAlg() = default;
    StatusCode initialize() override;

    StatusCode execute(const EventContext& ctx) const override;

private:

    ToolHandle<ITrackDepositInCaloTool> m_trkDepositInCalo{this, "TrackDepositInCaloTool", "TrackDepositInCaloTool/TrackDepositInCaloTool"};

    /// Particle container to decorate the Pivot plane coordinates to
    SG::ReadHandleKey<xAOD::IParticleContainer> m_partKey{this, "ContainerKey", "RandomParticle"};

    Gaudi::Property<float> m_ptMin{this, "PtMin", 2.5 * Gaudi::Units::GeV, 
                                    "Minimal track pt required to decorate the ID track"};

    /// Optional list of decorators to select only the good tracks for the isolation decoration. Only one decorator needs
    /// to pass to launch the isolation calculation    
    Gaudi::Property<std::vector<std::string>> m_trkSelDecors{this, "TrackSelections", {},
                                                  "List of decorator names of which one needs to be true to run the isolation" };
    SG::ReadDecorHandleKeyArray<xAOD::IParticleContainer> m_trkSelKeys{this, "SelectionKeys", {},
                                                                       "Will be overwritten in initialize"};

    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_depositKey{this, "Calo_Deposit", m_partKey, "CaloDeposits", 
                                                "Name of the decorator to store all the energy deposits in the calorimeter"};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_elossKey{this, "Calo_EnergyLoss", m_partKey, "CaloElosses",
                                                                  "Name of the decorator to store the energy loss from EMB1."};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_typeKey{this, "Calo_Type", m_partKey, "CaloDepType",
                                                                 "Name of the decorator to store the energy deposit from EMB2."};
};

}

#endif
