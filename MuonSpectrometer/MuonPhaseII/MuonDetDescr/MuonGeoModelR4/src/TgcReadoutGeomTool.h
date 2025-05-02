/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_TgcREAOUDGEOMTOOL_H
#define MUONGEOMODELR4_TgcREAOUDGEOMTOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace MuonGMR4 {

class TgcReadoutGeomTool : public extends<AthAlgTool, IMuonReadoutGeomTool> {
   public:
    // Constructor
    using base_class::base_class;

    StatusCode buildReadOutElements(MuonDetectorManager &mgr) override final;

   private:
    /// Map the Tgc sectors to the classical Muon System sectors
    StatusCode writeSectorMapping(const MuonDetectorManager& mgr) const;
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                          "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc", "GeoDbTagSvc"};

    PublicToolHandle<IMuonGeoUtilityTool> m_geoUtilTool{this,"GeoUtilTool", "" };
    
    /// Helper struct to cache the essential readout parameters from the WTGC tables
    struct wTgcTable {      
        std::vector<double> bottomStripPos{};
        std::vector<double> topStripPos{};
        std::vector<unsigned int> wireGangs{};
        double wirePitch{0.};
        unsigned int gasGap{0};
    };
    struct FactoryCache {       
       using ParamBookTable = std::map<std::string, wTgcTable>;
       ParamBookTable parameterBook{};
       
       using ReadoutTable = std::map<std::string, StripLayerPtr>;
       ReadoutTable wireDesigns{};
       ReadoutTable stripDesigns{};

       RadialStripDesignSet stripLayouts{};
       WireGroupDesignSet wireLayouts{};
    };

    /// Retrieves the auxillary tables from the database
    StatusCode readParameterBook(FactoryCache& cache);
    /// Loads the chamber dimensions from GeoModel
    StatusCode loadDimensions(TgcReadoutElement::defineArgs& args, FactoryCache& factory );
    
    IdentifierHash layerHash(const TgcReadoutElement::defineArgs& args, const int gasGap, const int doubPhi, const bool measPhi) const;

};

}  // namespace MuonGMR4
#endif
