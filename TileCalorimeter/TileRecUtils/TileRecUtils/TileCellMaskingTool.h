/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECELLMASKINGTOOL_H
#define TILECELLMASKINGTOOL_H

// Gaudi includes
#include "GaudiKernel/ServiceHandle.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgTool.h"

// Calo includes
#include "CaloInterface/ICaloCellMakerTool.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include <string>
#include <vector>
#include <bitset>

class TileID;
class TileHWID;
class HWIdentifier;

class TileCellMaskingTool: public AthAlgTool, virtual public ICaloCellMakerTool {
  public:
    TileCellMaskingTool(const std::string& type
                        , const std::string& name
                        , const IInterface* parent);

    virtual StatusCode initialize() override;
    virtual StatusCode process (CaloCellContainer* theCellContainer,
                                const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

    virtual bool channel_is_good(HWIdentifier & hwid);

  private:

    StatusCode fillIncludedCellsMap();
    void killer(std::string_view component, int ros, int drw, int index);

    Gaudi::Property<std::vector<std::string>> m_rejectedTileDrawer{this,
        "RejectedTileDrawer", {}, "List of rejected Tile drawers (ros drawer)"};
    Gaudi::Property<std::vector<std::string>> m_rejectedTileMB{this,
        "RejectedTileMB", {}, "List of rejected Tile motherboards (ros drawer mb)"};
    Gaudi::Property<std::vector<std::string>> m_rejectedTileDigitizer{this,
        "RejectedTileDigitizer", {}, "List of rejected Tile digitizers (ros drawer digitizer)"};
    Gaudi::Property<std::vector<std::string>> m_rejectedTileDMU{this,
        "RejectedTileDMU", {}, "List of rejected Tile DMUs (ros drawer dmu)"};
    Gaudi::Property<std::vector<std::string>> m_rejectedTileChannels{this,
        "RejectedTileChannels", {}, "List of rejected Tile channels (ros drawer channel)"};
    // half a MeV in both PMTs i.e. one MeV in a cell
    Gaudi::Property<float> m_zeroEnergy{this, "BadChannelZeroEnergy", 0.5 * CLHEP::MeV,
        "Energy assigned to masked channels"};

    const TileID* m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};

    std::bitset<65536> m_includedCellsMap;

};

#endif
