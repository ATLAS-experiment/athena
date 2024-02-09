/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_MUONGEOUTILITYTOOL_H
#define MUONGEOMODELR4_MUONGEOUTILITYTOOL_H

#include <CxxUtils/checker_macros.h>
#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <GeoModelUtilities/GeoModelTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <GeoModelKernel/GeoSimplePolygonBrep.h>


namespace MuonGMR4 {

class MuonGeoUtilityTool final : virtual public IMuonGeoUtilityTool, public AthAlgTool {

   public:
    // Constructor
    MuonGeoUtilityTool(const std::string &type, const std::string &name,
                     const IInterface *parent);

    /// tool hook
    StatusCode initialize() override final;
    // Destructor
    virtual ~MuonGeoUtilityTool() override final;

    const GeoAlignableTransform* findAlignableTransform(const PVConstLink& physVol) const override final;

    const GeoShape* extractShape(const PVConstLink& physVol) const override final;
    const GeoShape* extractShape(const GeoShape* inShape) const override final;
    
    // Navigates through the volume to find the shifts / rotations etc.
    Amg::Transform3D extractShifts(const PVConstLink& physVol) const override final;
    Amg::Transform3D extractShifts(const GeoShape* inShape) const override final;


      
    std::vector<physVolWithTrans> findAllLeafNodesByName(const PVConstLink& physVol, const std::string& volumeName) const override final;

    std::vector<geoShapeWithShift> getComponents(const GeoShapeUnion* unionShape) const override final;
    /// Dumps the shape to string
    std::string dumpShape(const GeoShape* inShape) const override final;

    std::string dumpVolume(const PVConstLink& physVol) const override final;

    /// Transforms the vertices of the Polygon shape into a std::vector consisting of Amg::Vector2D objects
    std::vector<Amg::Vector2D> polygonEdges(const GeoSimplePolygonBrep& polygon) const override;

   private:
    std::string dumpVolume(const PVConstLink& physVol, const std::string& childDelim) const;


    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
};
}  // namespace MuonGMR4
#endif
