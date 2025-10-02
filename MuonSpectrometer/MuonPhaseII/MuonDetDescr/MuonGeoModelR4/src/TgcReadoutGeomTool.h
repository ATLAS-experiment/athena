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
#include <GeoModelHelpers/GeoDeDuplicator.h>

class GeoTrd;
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
        /** @brief Parameter map of the Tgc technology. The key is composed
         *         by the chamber design, the detctor side & the gas gap number
         *         inside the chamber  */   
       using ParamBookTable = std::unordered_map<std::string, wTgcTable>;
       ParamBookTable parameterBook{};
       /** @brief Map to share the StripLayer readout objects across multiple
        *         readout elements */
       using ReadoutTable = std::map<std::string, StripLayerPtr>;
       ReadoutTable wireLayers{};
       ReadoutTable stripLayers{};

       /** @brief Set to share equivalent RadialStripDesigns across multiple gas gaps */
       RadialStripDesignSet stripReadouts{};
       /** @brief Set to share equivalent WireGroupDesigns across multiple gas gaps */
       WireGroupDesignSet wireLayouts{};
       /** @brief Helper object to turn Amg::Transforms into GeoModel tree transform nodes */
       GeoDeDuplicator trfNodeMaker{};
    };

    /// Retrieves the auxillary tables from the database
    StatusCode readParameterBook(FactoryCache& cache);
    /// Loads the chamber dimensions from GeoModel
    StatusCode loadDimensions(TgcReadoutElement::defineArgs& args, FactoryCache& factory );
    /** @brief Constructs a new wire group design, if the table has wires defined.
     *  @param table : Reference to the detector table entry defining the wires per group
     *  @param gapTrd: Pointer to the GeoShape describing the Tgc gas gap */   
    std::unique_ptr<WireGroupDesign> 
            constructWireDesign(const wTgcTable& table,
                                const GeoTrd* gapTrd) const;
    /** @brief Constructs a new radial strip design, if the table contains radial strips.
     *  @param table : Reference to the detector table entry defining the
     *                 muonting points of the strips
     *  @param gapTrd: Pointer to the GeoShape describing the Tgc gas gap */   
    std::unique_ptr<RadialStripDesign> 
            constructRadialDesign(const wTgcTable& table,
                                 const GeoTrd* gapTrd) const;

};

}  // namespace MuonGMR4
#endif
