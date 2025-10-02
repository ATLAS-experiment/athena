///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// TileCellsMuonDecorator.h
// Header file for class TileCellsMuonDecorator
///////////////////////////////////////////////////////////////////
#ifndef DERIVATIONFRAMEWORK_DERIVATIONFRAMEWORKTILECAL_TILECELLSMUONDECORATOR_H
#define DERIVATIONFRAMEWORK_DERIVATIONFRAMEWORKTILECAL_TILECELLSMUONDECORATOR_H 1

#include "TileCellsDecorator.h"
#include "ITrackTools.h"

// DerivationFrameworkInterfaces includes
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "ParticlesInConeTools/ITrackParticlesInConeTool.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// STL includes
#include <string>
#include <vector>

class TileID;
class TileHWID;


namespace DerivationFramework {

    class TileCellsMuonDecorator: public extends<AthAlgTool, IAugmentationTool> {


    public:

      /// Constructor with parameters:
      using base_class::base_class;

      virtual StatusCode addBranches() const override final;

      // Athena algtool's Hooks
      virtual StatusCode initialize() override final;

    private:

      Gaudi::Property<std::string> m_prefix{this, "Prefix", "TCAL1_"};
      Gaudi::Property<bool> m_selectMuons{this, "SelectMuons", false};
      Gaudi::Property<double> m_minPt{this, "MinMuonPt", 10000.0};
      Gaudi::Property<double> m_maxAbsEta{this, "MaxAbsMuonEta", 1.7};
      Gaudi::Property<double> m_isoCone{this, "IsoCone", 0.4};
      Gaudi::Property<std::vector<double>> m_drCones{this,
             "DeltaRCones", {0.2, 0.4},  "Sum energies in calorimeter layers in these cones aroud track"};
      Gaudi::Property<std::set<unsigned int>> m_energyInLayers{this,
            "EnergyInSamplings", {1, 2, 3, 5, 6, 7}, "Sum energies in these calorimeter layers in cone aroud track"};
      Gaudi::Property<double> m_maxRelEtrkInIsoCone{this, "MaxRelETrkInIsoCone", 0.1};
      Gaudi::Property<double> m_gapCrackCellsInDeltaEta{this, "GapCrackCellsInDeltaEta", 0.5};
      Gaudi::Property<double> m_gapCrackCellsInDeltaPhi{this, "GapCrackCellsInDeltaPhi", 0.5};

      SG::ReadHandleKey<xAOD::MuonContainer> m_muonContainerKey{this, "MuonContainer", "Muons"};
      SG::ReadHandleKey<CaloCellContainer> m_cellContainerKey{this, "CellContainer", "AllCalo"};
      SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clusterContainerKey{this, "ClusterContainer", "CaloCalTopoClusters"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_selectedMuKey{this, "SelectedMuon", "SelectedMuon"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_econeMuKey{this, "Etrkcone", "etrkcone"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsMuonXKey{this, "CellsMuonX", "cells_muon_x"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsMuonYKey{this, "CellsMuonY", "cells_muon_y"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsMuonZKey{this, "CellsMuonZ", "cells_muon_z"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsMuonEtaKey{this, "CellsMuonEta", "cells_muon_eta"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsMuonPhiKey{this, "CellsMuonPhi", "cells_muon_phi"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsToMuonDxKey{this, "CellsToMuonDx", "cells_to_muon_dx"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsToMuonDyKey{this, "CellsToMuonDy", "cells_to_muon_dy"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsToMuonDzKey{this, "CellsToMuonDz", "cells_to_muon_dz"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsToMuonDetaKey{this, "CellsToMuonDeta", "cells_to_muon_deta"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsToMuonDphiKey{this, "CellsToMuonDphi", "cells_to_muon_dphi"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsMuonDxKey{this, "CellsMuonDx", "cells_muon_dx"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsMuonDeDxKey{this, "CellsMuonDeDx", "cells_muon_dedx"};
      SG::WriteDecorHandleKeyArray<xAOD::MuonContainer> m_larEnergyInConeKeyArray{this,
          "LArEnergyInCone", {}, "It is atuoconfigured based on requested delta R cones, eg.: elarcone40"};

      ToolHandle<TileCal::ITrackTools> m_trackInCalo{this,
          "TrackTools", "TileCall::TrackTools/TrackTools"};

      ToolHandle<xAOD::ITrackParticlesInConeTool> m_tracksInCone{this,
          "TracksInConeTool", "xAOD::TrackParticlesInConeTool/TrackParticlesInConeTool"};

      ToolHandle<DerivationFramework::TileCellsDecorator> m_cellsDecorator{this,
          "CellsDecorator", "DerivationFramework::TileCellsDecorator/TileCellsDecorator"};

      std::set<xAOD::CaloCluster::CaloSample> m_energyInSamplings;
  };

}


#endif //> !DERIVATIONFRAMEWORK_DERIVATIONFRAMEWORKTILECAL_TILECELLSMUONDECORATOR_H
