/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_MUONDETECTORTOOL_H
#define MUONGEOMODELR4_MUONDETECTORTOOL_H

#include "MuonReadoutGeometryR4/MuonDetectorDefs.h"
///
#include "CxxUtils/checker_macros.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "GeoModelUtilities/GeoModelTool.h"
#include "MuonGeoModelR4/IMuonReaoutGeomTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonGMR4 {
class MuonDetectorManager;
class MuonDetectorTool final : public GeoModelTool {

   public:
    // Constructor
    MuonDetectorTool(const std::string &type, const std::string &name,
                     const IInterface *parent);

    // Destructor
    virtual ~MuonDetectorTool() override final;

    // build the geometry
    virtual StatusCode create() override final;

    // Dereference tree tops and drop readout objects
    virtual StatusCode clear() override final;

   private:
    /// List of GeoGraphnodes in the tree, that belong to the Muon system.
    Gaudi::Property<std::vector<std::string>> m_treeTopNodes{this, "TreeTopNodes",
        {"MuonBarrel", "NSW", "MuonEndcap_sideA", "MuonEndcap_sideC", "Muon"}};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ToolHandleArray<IMuonReadoutGeomTool> m_detTechTools{
        this,
        "ReadoutEleBuilders",
        {},
        "SubTools to build the readoutElements for each technology"};

    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc",
                                              "GeoDbTagSvc"};
    MuonDetectorManager *m_manager{nullptr};
};
}  // namespace MuonGMR4
#endif
