/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HGTD_DETDESCRALGS_PRINTHGTDELEMENTS_H
#define HGTD_DETDESCRALGS_PRINTHGTDELEMENTS_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "HGTD_ReadoutGeometry/HGTD_DetectorElementCollection.h"
#include "HGTD_Identifier/HGTD_ID.h"

#include "GeoModelUtilities/GeoAlignmentStore.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <fstream>

class PrintHGTDElements : public AthAlgorithm
{
public:

  PrintHGTDElements(const std::string& name,
                    ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

private:

  SG::ReadCondHandleKey<InDetDD::HGTD_DetectorElementCollection>
    m_detEleCollKey{
      this,
      "HGTDDetEleCollKey",
      "HGTD_DetectorElementCollection",
      "HGTD detector element collection"
    };

  SG::ReadCondHandleKey<GeoAlignmentStore> m_alignStoreKey{
    this,
    "AlignmentStore",
    "HGTDAlignmentStore",
    "HGTD GeoAlignmentStore"
  };

  const HGTD_ID* m_hgtdId{nullptr};

  Gaudi::Property<std::string>
    m_outputFile{
      this,
      "OutputFile",
      "HGTDGeometry.dat",
      "Output geometry file"
    };

  std::ofstream m_outfile;

  bool m_firstEvent{true};
};

#endif