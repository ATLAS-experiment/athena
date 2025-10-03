/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_RPCREAOUDGEOMTOOL_H
#define MUONGEOMODELR4_RPCREAOUDGEOMTOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

#include <GeoModelHelpers/GeoDeDuplicator.h>

class GeoBox;

namespace MuonGMR4 {
/** @brief Implementation to construct Rpc readout element from the list of published
 *         full physical volumes and the WRPC meta data table. */
class RpcReadoutGeomTool : public extends<AthAlgTool,IMuonReadoutGeomTool> {
   public:
    // Constructor
    using base_class::base_class;

    StatusCode buildReadOutElements(MuonDetectorManager &mgr) override final;

   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                          "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc", "GeoDbTagSvc"};

    PublicToolHandle<IMuonGeoUtilityTool> m_geoUtilTool{this,"GeoUtilTool", "" };
    
    /// Struct to cache the relevant parameters of from the WRPC tables
    struct wRPCTable {
       /// Eta strip pitch
       double stripPitchEta{0.};
       /// Phi strip pitch
       double stripPitchPhi{0.};
       /// Eta strip width
       double stripWidthEta{0.};
       /// Phi strip width
       double stripWidthPhi{0.};
       /// Offset of the first phi strip
       double firstOffSetPhi{0.};
       /// Offset of the first eta strip
       double firstOffSetEta{0.};
       /// Number of eta strips
       unsigned int numEtaStrips{0};
       /// Number of phi strips
       unsigned int numPhiStrips{0};
    };

    /** @brief Cache object to the wRPCTable & store stripDesigns & layers
     *         to make the information available throughout the geometry building and to
     *         allow for sharing of Identical StripLayers */
    struct FactoryCache {
       
      using ParamBookTable = std::map<std::string, wRPCTable>;

       std::set<StripDesignPtr, StripDesignSorter> stripDesigns{};
       std::set<StripLayerPtr, StripLayerSorter> stripLayers{};
       ParamBookTable parameterBook{};
      /** @brief Helper object to turn Amg::Transforms into GeoModel tree transform nodes */
       GeoDeDuplicator trfNodeMaker{};
          
    };

    /// Retrieves the auxillary tables from the database
    StatusCode readParameterBook(FactoryCache& cache);
    /// Loads the chamber dimensions from GeoModel
    StatusCode loadDimensions(RpcReadoutElement::defineArgs& args, FactoryCache& factory);
    /** @brief Constructs a new Strip design from the parameter book to describe either
     *         the phi plane or the eta strip-plane 
     *  @param planeBox: Pointer to the shape describing the strip-readout volume,
     *                   needed to fetch the design's dimensions
     *  @param paramBook: Parameter book to read off the strip design paramters in terms of
     *                    pitch, n-strips etc
     *  @param phiPlane: Switch toggling whether the eta / phi design should be created */
    std::unique_ptr<StripDesign> constructDesign(const GeoBox* planeBox,
                                                 const wRPCTable& paramBook,
                                                 bool phiPlane) const;

};

}  // namespace MuonGMR4
#endif
