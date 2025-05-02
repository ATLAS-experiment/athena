/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_MMREAOUDGEOMTOOL_H
#define MUONGEOMODELR4_MMREAOUDGEOMTOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace MuonGMR4 {

class MmReadoutGeomTool : public extends<AthAlgTool, IMuonReadoutGeomTool> {
   public:
    // Constructor
    using base_class::base_class;

    StatusCode buildReadOutElements(MuonDetectorManager& mgr) override final;


   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                          "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc", "GeoDbTagSvc"};

    PublicToolHandle<IMuonGeoUtilityTool> m_geoUtilTool{this,"GeoUtilTool", "" };
    
    /// Struct to cache the relevant parameters of from the WRPC tables
    struct wMMTable {
       double stripPitch{0.};
       double stripWidth{0.};
       double distBotFrameStrip{0.};
       std::vector<double> stereoAngle{};
       std::vector<int> totalActiveStrips{};
       std::vector<int> readoutSide{};
       int nMissedBottomEta{0};
       int nMissedBottomStereo{0};
       int nMissedTopEta{0};
       std::vector<StripLayer> layers{};
    };


    struct FactoryCache {
       
        using ParamBookTable = std::map<std::string, wMMTable>;
        std::set<StripDesignPtr, StripDesignSorter> stripDesigns{};
        std::set<StripLayerPtr, StripLayerSorter> stripLayers{};

        ParamBookTable parameterBook{};
       
    };


    /// Retrieves the auxillary tables from the database
    StatusCode readParameterBook(FactoryCache& cache);

    /// Loads the chamber dimensions from GeoModel
    StatusCode loadDimensions(MmReadoutElement::defineArgs& args, 
                              FactoryCache& factory);
};

}  // namespace MuonGMR4
#endif
