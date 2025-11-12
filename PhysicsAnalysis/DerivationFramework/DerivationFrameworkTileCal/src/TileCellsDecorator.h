///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// TileCellsDecorator.h
// Header file for class TileCellsDecorator
///////////////////////////////////////////////////////////////////
#ifndef DERIVATIONFRAMEWORK_DERIVATIONFRAMEWORKTILECAL_TILECELLSDECORATOR_H
#define DERIVATIONFRAMEWORK_DERIVATIONFRAMEWORKTILECAL_TILECELLSDECORATOR_H 1

// Tile includes
#include "TileConditions/TileCablingSvc.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODMuon/MuonContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// STL includes
#include <string>
#include <vector>

class CaloCell;
class TileID;
class TileHWID;
class EventContext;

namespace xAOD {
  class IParticle;
}


namespace DerivationFramework {

  class TileCellsDecorator: public AthAlgTool {

    public:

      /// Constructor with parameters:
      TileCellsDecorator( const std::string& type, const std::string& name, const IInterface* parent );

      // Athena algtool's Hooks
      StatusCode initialize() override final;

    StatusCode decorate(const std::map<const xAOD::IParticle*, std::vector<const CaloCell*>>& muonCellsMap, const EventContext& ctx) const;

    private:

      SG::ReadHandleKey<xAOD::MuonContainer> m_muonContainer{this, "MuonContainer", "Muons"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsEnergyKey{this, "CellsEnergy", m_muonContainer, "cells_energy"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsEtKey{this, "CellsEt", m_muonContainer, "cells_et"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsEtaKey{this, "CellsEta", m_muonContainer, "cells_eta"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPhiKey{this, "CellsPhi", m_muonContainer, "cells_phi"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsGainKey{this, "CellsGain", m_muonContainer, "cells_gain"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsBadKey{this, "CellsBad", m_muonContainer, "cells_bad"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSamplingKey{this, "CellsSampling", m_muonContainer, "cells_sampling"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsTimeKey{this, "CellsTime", m_muonContainer, "cells_time"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsQualityKey{this, "CellsQuality", m_muonContainer, "cells_quality"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSinThKey{this, "CellsSinTh", m_muonContainer, "cells_sinTh"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsCosThKey{this, "CellsCosTh", m_muonContainer, "cells_cosTh"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsCotThKey{this, "CellsCotTh", m_muonContainer, "cells_cotTh"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsXKey{this, "CellsX", m_muonContainer, "cells_x"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsYKey{this, "CellsY", m_muonContainer, "cells_y"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsZKey{this, "CellsZ", m_muonContainer, "cells_z"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsRKey{this, "CellsR", m_muonContainer, "cells_r"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDxKey{this, "CellsDx", m_muonContainer, "cells_dx"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDyKey{this, "CellsDy", m_muonContainer, "cells_dy"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDzKey{this, "CellsDz", m_muonContainer, "cells_dz"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDrKey{this, "CellsDr", m_muonContainer, "cells_dr"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsVolumeKey{this, "CellsVolume", m_muonContainer, "cells_volume"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDetaKey{this, "CellsDeta", m_muonContainer, "cells_deta"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDphiKey{this, "CellsDphi", m_muonContainer, "cells_dphi"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSideKey{this, "CellsSide", m_muonContainer, "cells_side"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSectionKey{this, "CellsSection", m_muonContainer, "cells_section"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsModuleKey{this, "CellsModule", m_muonContainer, "cells_module"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsTowerKey{this, "CellsTower", m_muonContainer, "cells_tower"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSampleKey{this, "CellsSample", m_muonContainer, "cells_sample"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1RosKey{this, "CellsPmt1Ros", m_muonContainer, "cells_pmt1_ros"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2RosKey{this, "CellsPmt2Ros", m_muonContainer, "cells_pmt2_ros"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1DrawerKey{this, "CellsPmt1Drawer", m_muonContainer, "cells_pmt1_drawer"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2DrawerKey{this, "CellsPmt2Drawer", m_muonContainer, "cells_pmt2_drawer"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1ChannelKey{this, "CellsPmt1Channel", m_muonContainer, "cells_pmt1_channel"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2ChannelKey{this, "CellsPmt2Channel", m_muonContainer, "cells_pmt2_channel"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1EnergyKey{this, "CellsPmt1Energy", m_muonContainer, "cells_pmt1_energy"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2EnergyKey{this, "CellsPmt2Energy", m_muonContainer, "cells_pmt2_energy"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1TimeKey{this, "CellsPmt1Time", m_muonContainer, "cells_pmt1_time"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2TimeKey{this, "CellsPmt2Time", m_muonContainer, "cells_pmt2_time"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1QualityKey{this, "CellsPmt1Quality", m_muonContainer, "cells_pmt1_quality"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2QualityKey{this, "CellsPmt2Quality", m_muonContainer, "cells_pmt2_quality"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1QbitKey{this, "CellsPmt1Qbit", m_muonContainer, "cells_pmt1_qbit"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2QbitKey{this, "CellsPmt2Qbit", m_muonContainer, "cells_pmt2_qbit"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1BadKey{this, "CellsPmt1Bad", m_muonContainer, "cells_pmt1_bad"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2BadKey{this, "CellsPmt2Bad", m_muonContainer, "cells_pmt2_bad"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1GainKey{this, "CellsPmt1Gain", m_muonContainer, "cells_pmt1_gain"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2GainKey{this, "CellsPmt2Gain", m_muonContainer, "cells_pmt2_gain"};

     /**
      * @brief Name of Tile cabling service
      */
    ServiceHandle<TileCablingSvc> m_cablingSvc{ this,
         "TileCablingSvc", "TileCablingSvc", "The Tile cabling service"};

      const TileID* m_tileID{nullptr};
      const TileHWID* m_tileHWID{nullptr};
  };

}


#endif //> !DERIVATIONFRAMEWORK_DERIVATIONFRAMEWORKTILECAL_TILECELLSDECORATOR_H
