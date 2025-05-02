/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_MDTREAOUDGEOMTOOL_H
#define MUONGEOMODELR4_MDTREAOUDGEOMTOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace MuonGMR4 {

class MdtReadoutGeomTool : public extends<AthAlgTool,IMuonReadoutGeomTool> {
   public:
    // Constructor
    using base_class::base_class;


    StatusCode buildReadOutElements(MuonDetectorManager &mgr) override final;

   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc",
                                              "GeoDbTagSvc"};

    PublicToolHandle<IMuonGeoUtilityTool> m_geoUtilTool{this,"GeoUtilTool", "" };
    using parameterBook = MdtReadoutElement::parameterBook;
    using ParamBookTable = std::map<std::string, parameterBook>; 
    
    struct FactoryCache {
        ParamBookTable parBook{};
        /// List of chambers that have the readout chip at 
        /// negative Z
        std::unordered_set<Identifier> readoutOnLeftSide{};
        using CutTubes = MdtTubeLayer::CutTubes;
        using CutTubeSet =  MdtTubeLayer::CutTubeSet;
        std::unordered_map<Identifier, CutTubeSet> cutTubes{};
        MdtTubeLayerSet tubeLayers{};
    };

    /// Retrieves the auxillary tables from the database
    StatusCode readParameterBook(FactoryCache& facCache) const;
    /// Loads the chamber dimensions from GeoModel
    StatusCode loadDimensions(FactoryCache& facCache, MdtReadoutElement::defineArgs& args) const;

};

}  // namespace MuonGMR4
#endif
