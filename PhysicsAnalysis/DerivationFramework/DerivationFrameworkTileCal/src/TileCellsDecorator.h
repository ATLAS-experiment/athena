///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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

      Gaudi::Property<std::string> m_prefix{this, "Prefix", "TCAL1_"};
      Gaudi::Property<std::string> m_muonContainer{this, "MuonContainer", "Muons"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsEnergyKey{this, "CellsEnergy", "cells_energy"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsEtKey{this, "CellsEt", "cells_et"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsEtaKey{this, "CellsEta", "cells_eta"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPhiKey{this, "CellsPhi", "cells_phi"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsGainKey{this, "CellsGain", "cells_gain"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsBadKey{this, "CellsBad", "cells_bad"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSamplingKey{this, "CellsSampling", "cells_sampling"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsTimeKey{this, "CellsTime", "cells_time"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsQualityKey{this, "CellsQuality", "cells_quality"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSinThKey{this, "CellsSinTh", "cells_sinTh"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsCosThKey{this, "CellsCosTh", "cells_cosTh"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsCotThKey{this, "CellsCotTh", "cells_cotTh"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsXKey{this, "CellsX", "cells_x"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsYKey{this, "CellsY", "cells_y"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsZKey{this, "CellsZ", "cells_z"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsRKey{this, "CellsR", "cells_r"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDxKey{this, "CellsDx", "cells_dx"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDyKey{this, "CellsDy", "cells_dy"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDzKey{this, "CellsDz", "cells_dz"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDrKey{this, "CellsDr", "cells_dr"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsVolumeKey{this, "CellsVolume", "cells_volume"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDetaKey{this, "CellsDeta", "cells_deta"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsDphiKey{this, "CellsDphi", "cells_dphi"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSideKey{this, "CellsSide", "cells_side"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSectionKey{this, "CellsSection", "cells_section"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsModuleKey{this, "CellsModule", "cells_module"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsTowerKey{this, "CellsTower", "cells_tower"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsSampleKey{this, "CellsSample", "cells_sample"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1RosKey{this, "CellsPmt1Ros", "cells_pmt1_ros"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2RosKey{this, "CellsPmt2Ros", "cells_pmt2_ros"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1DrawerKey{this, "CellsPmt1Drawer", "cells_pmt1_drawer"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2DrawerKey{this, "CellsPmt2Drawer", "cells_pmt2_drawer"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1ChannelKey{this, "CellsPmt1Channel", "cells_pmt1_channel"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2ChannelKey{this, "CellsPmt2Channel", "cells_pmt2_channel"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1EnergyKey{this, "CellsPmt1Energy", "cells_pmt1_energy"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2EnergyKey{this, "CellsPmt2Energy", "cells_pmt2_energy"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1TimeKey{this, "CellsPmt1Time", "cells_pmt1_time"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2TimeKey{this, "CellsPmt2Time", "cells_pmt2_time"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1QualityKey{this, "CellsPmt1Quality", "cells_pmt1_quality"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2QualityKey{this, "CellsPmt2Quality", "cells_pmt2_quality"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1QbitKey{this, "CellsPmt1Qbit", "cells_pmt1_qbit"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2QbitKey{this, "CellsPmt2Qbit", "cells_pmt2_qbit"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1BadKey{this, "CellsPmt1Bad", "cells_pmt1_bad"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2BadKey{this, "CellsPmt2Bad", "cells_pmt2_bad"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt1GainKey{this, "CellsPmt1Gain", "cells_pmt1_gain"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cellsPmt2GainKey{this, "CellsPmt2Gain", "cells_pmt2_gain"};

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
