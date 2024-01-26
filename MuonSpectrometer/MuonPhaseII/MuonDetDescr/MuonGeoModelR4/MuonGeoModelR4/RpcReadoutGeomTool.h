/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_RPCREAOUDGEOMTOOL_H
#define MUONGEOMODELR4_RPCREAOUDGEOMTOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/CutOutArea.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace MuonGMR4 {

class RpcReadoutGeomTool : public AthAlgTool,
                           virtual public IMuonReadoutGeomTool {
   public:
    // Constructor
    RpcReadoutGeomTool(const std::string &type, const std::string &name,
                       const IInterface *parent);

    StatusCode initialize() override final;

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

    struct FactoryCache {
       
      using ParamBookTable = std::map<std::string, wRPCTable>;
      using CutOutTable = std::map<Identifier, std::vector<CutOutArea>>;

       std::set<StripDesignPtr, StripDesignSorter> stripDesigns{};
       ParamBookTable parameterBook{};
       CutOutTable cutOuts{};
       
    };

    /// Retrieves the auxillary tables from the database
    StatusCode readParameterBook(FactoryCache& cache);
    /// Loads the chamber dimensions from GeoModel
    StatusCode loadDimensions(RpcReadoutElement::defineArgs& args, FactoryCache& factory );

};

}  // namespace MuonGMR4
#endif
