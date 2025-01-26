/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSMUONDETECTOR_MUONDETECTORBUILDERTOOL_H
#define ACTSMUONDETECTOR_MUONDETECTORBUILDERTOOL_H

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <ActsGeometryInterfaces/IDetectorVolumeBuilderTool.h>
#include <ActsGeoUtils/SurfaceBoundSet.h>
#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <Acts/Surfaces/PlanarBounds.hpp>
#include <Acts/Surfaces/Surface.hpp>

struct GeoChildNodeWithTrf;
class GeoMaterial;

namespace ActsTrk{
    class MuonDetectorBuilderTool: public extends<AthAlgTool,IDetectorVolumeBuilderTool> {
    
    public:
        /** @brief Standard tool constructor **/
        MuonDetectorBuilderTool( const std::string& type, const std::string& name, const IInterface* parent );

        virtual ~MuonDetectorBuilderTool() = default;

        StatusCode initialize() override final;

        Acts::Experimental::DetectorComponent construct(const Acts::GeometryContext& context) const override final;     

    private:

        void processPassiveNodes(const ActsGeometryContext& gctx, 
                                 const GeoChildNodeWithTrf& node, 
                                 const std::string& name, 
                                 std::vector<std::shared_ptr<Acts::Experimental::DetectorVolume>>& passiveVolumes, 
                                 const GeoTrf::Transform3D& transform) const;

        
        using BlendedBoundSet = ActsTrk::SurfaceBoundSet<Acts::TrapezoidBounds>;
        std::shared_ptr<Acts::Surface> getChamberMaterial(const MuonGMR4::Chamber& chamber, 
                                                          const Amg::Transform3D& chamberTransform,                                                          
                                                          const int totalMaterials,
                                                          BlendedBoundSet& boundSet) const;

        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        Gaudi::Property<bool> m_dumpDetector{this, "dumpDetector", false, "If set to true the entire MS system are dumped into a visualization file format, will take a long time with sensitives"};

        Gaudi::Property<bool> m_dumpPassive{this, "dumpPassive", false, "If set to true the passive volumes are dumped into a visualization file format"};

        Gaudi::Property<bool> m_dumpDetectorVolumes{this, "dumpDetectorVolumes", false, "If set to true the detector volumes are dumped into a visualization file format, will take a long time with sensitives"};

        Gaudi::Property<bool> m_dumpMaterialSurfaces{this, "dumpMaterialSurfaces", false, "If set to true the material surfaces are dumped into a visualization file format"};

        Gaudi::Property<bool> m_buildSensitives{this, "BuildSensitives", true, "If set to true all sensitive elements are built"};
        
        //private method for the readout element construction
        std::pair<std::vector<std::shared_ptr<Acts::Experimental::DetectorVolume>>,
                  std::vector<std::shared_ptr<Acts::Surface>>> constructElements(const ActsGeometryContext& gctx, 
                                                                                 const MuonGMR4::Chamber& mChamber, std::pair<unsigned int, unsigned int> chId) const;

        bool checkDummyMaterial(const PVConstLink& vol) const;

        using MaterialPtr = GeoIntrusivePtr<const GeoMaterial>;
        void getMaterialContent(const PVConstLink& vol, std::vector<std::pair<MaterialPtr, double>>& materialContent) const;

        std::pair<MaterialPtr, double> getMaterial(const PVConstLink& vol) const;


    };
}
#endif
