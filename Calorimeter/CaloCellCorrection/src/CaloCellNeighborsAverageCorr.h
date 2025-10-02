/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCELLCORRECTION_CALOCELLNEIGHBORSAVERAGECORR_H
#define CALOCELLCORRECTION_CALOCELLNEIGHBORSAVERAGECORR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloInterface/ICaloCellMakerTool.h"
#include "AthenaKernel/IOVSvcDefs.h"

#include <string>

class CaloCellContainer;
class CaloCell_ID;
class TileID;

//inspiration from http://alxr.usatlas.bnl.gov/lxr-stb3/source/atlas/Calorimeter/CaloRec/CaloRec/CaloCellCopyTool.h#032
class CaloCellNeighborsAverageCorr
  : public extends<AthAlgTool, ICaloCellMakerTool>
{

public:

  using base_class::base_class;

  virtual ~CaloCellNeighborsAverageCorr() {};

  /** initialize method
  */
  virtual StatusCode initialize() override;

  /** process calo cell collection to apply corrections
  */
  virtual StatusCode process ( CaloCellContainer * theCellContainer,
                               const EventContext& ctx) const override;

private:

 const CaloCell_ID* m_calo_id=nullptr;
 const TileID* m_tile_id=nullptr;
 Gaudi::Property<bool> m_testMode{this,"testMode",false};
 Gaudi::Property<bool> m_skipDeadFeb{this,"skipDeadFeb",true, "Skip already patched LAr-cells (eg dead Febs)"};
 Gaudi::Property<bool> m_skipDeadLAr{this,"skipDeadLAr",false,"Skip all dead LAr cells"};
 Gaudi::Property<bool> m_skipDeadDrawer{this,"skipDeadDrawer",false,"Skip dead Tile Drawers"};
 Gaudi::Property<bool> m_skipDeadTile{this,"skipDeadTile",true,"Skip all dead Tile cells"};

};

#endif
