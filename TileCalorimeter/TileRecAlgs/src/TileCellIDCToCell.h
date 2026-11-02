/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileCellIDCToCell.h
// Author   : Alexandre Solodkov
// Created  : Jan 2003
//
// DESCRIPTION
// 
//    TileCellIDCToCell copies all TileCells from TileIDC container
// to TileCellContainer without any corrections
//
// Properties (JobOption Parameters):
//
//    TileCellIDC               string   Name of TileIDC container to read
//    TileCellContainer         string   Name of CaloCellContainer to write
//    TileInfoName              string   Name of object in TDS with all parameters
//
// BUGS:
//  
// History:
//   02Mar02  Created from TileRawChannelToCell.
//  
//****************************************************************************

#ifndef TILERECALGS_TILECELLIDCTOCELL_H
#define TILERECALGS_TILECELLIDCTOCELL_H

#include "AthenaBaseComps/AthAlgorithm.h"

// Avoid pushing dependencies into clients- just fwd declare the following:

class TileID;
class TileCell;
class TileInfo;
class TileDetDescrManager;

// C++ STL includes
#include <string>

class TileCellIDCToCell: public AthAlgorithm {
  public:
    // Constructor
    TileCellIDCToCell(const std::string& name, ISvcLocator* pSvcLocator);

    // Destructor
    virtual ~TileCellIDCToCell();

    // Gaudi Hooks
    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

  private:
    Gaudi::Property<std::string> m_cellIDC{this, "TileCellIDC", "TileCellIDC", "Name of input container"};
    Gaudi::Property<std::string> m_cellContainer{this, "TileCellContainer", "TileCellContainer", "Name of output container"};
    Gaudi::Property<std::string> m_infoName{this, "TileInfoName", "TileInfo", "Name of TileInfo store"};

    const TileID* m_tileID{nullptr};
    const TileInfo* m_tileInfo{nullptr};
    const TileDetDescrManager* m_tileMgr{nullptr};
};

#endif // TILERECALGS_TILECELLIDCTOCELL_H
