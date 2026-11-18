/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNDBTOOLS_INDETTRKALIGNDBTOOL_H
#define INDETALIGNDBTOOLS_INDETTRKALIGNDBTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkAlignInterfaces/ITrkAlignDBTool.h"
#include "GaudiKernel/ToolHandle.h"

/**
  @file InDetTrkAlignDBTool.h 
  @class InDetTrkAlignDBTool 

  @brief Tool for writing Silicon and TRT alignment constants into oracle DB or ASCII files.

  @author Daniel Kollar <daniel.kollar@cern.ch>
  @date 08/09/2009
*/

class TFile;

namespace InDet
{

  class InDetTrkAlignDBTool : public extends<AthAlgTool, Trk::ITrkAlignDBTool> {

  public:

    /** constructor */
    using base_class::base_class;

    /** destructor */
    virtual ~InDetTrkAlignDBTool();

    /** initialize method */
    virtual StatusCode initialize();

    /** finalize method */
    virtual StatusCode finalize();

    /** Writes alignment parameters */
    void writeAlignPar();

  private:

    /** updates alignment constants in the DB */
    void updateDB();

    PublicToolHandle<Trk::ITrkAlignDBTool> m_siDBtool{this, "SiTrkAlignDBTool", "InDet::SiTrkAlignDBTool/SiTrkAlignDBTool", "Tool for handling the Silicon DB I/O"};
    PublicToolHandle<Trk::ITrkAlignDBTool> m_trtDBtool{this, "TRTTrkAlignDBTool", "InDet::TRTTrkAlignDBTool/TRTTrkAlignDBTool", "Tool for handling the TRT DB I/O"};

  }; // end class

} // end namespace

#endif // INDETALIGNDBTOOLS_INDETTRKALIGNDBTOOL_H
