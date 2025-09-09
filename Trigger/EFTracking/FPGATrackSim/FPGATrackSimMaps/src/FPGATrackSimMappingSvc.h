// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef TRIGFPGATrackSimMAPPINGSVC_H
#define TRIGFPGATrackSimMAPPINGSVC_H

#include "AthenaBaseComps/AthService.h"

#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimMaps/FPGATrackSimNNMap.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"

#include <iostream>
#include <fstream>
#include <string>

class IFPGATrackSimEventSelectionSvc;

class FPGATrackSimMappingSvc : public extends<AthService, IFPGATrackSimMappingSvc>
{
    public:

        FPGATrackSimMappingSvc(const std::string& name, ISvcLocator* svc);
        virtual ~FPGATrackSimMappingSvc() = default;

        virtual StatusCode initialize() override;

        virtual const FPGATrackSimPlaneMap* PlaneMap_1st(int slice) const override { return m_pmap_vector_1st.at(slice).get(); }
        virtual const FPGATrackSimPlaneMap* PlaneMap_2nd(int slice) const override { return m_pmap_vector_2nd.at(slice).get(); }
        virtual const FPGATrackSimRegionMap* RegionMap_1st()        const override { return m_rmap_1st.get(); }
        virtual const FPGATrackSimRegionMap* RegionMap_2nd()        const override { return m_rmap_2nd.get(); }
        virtual const FPGATrackSimRegionMap* SubRegionMap()         const override { return m_subrmap.get();  }
        virtual const FPGATrackSimRegionMap* SubRegionMap_2nd()     const override { return m_subrmap_2nd.get(); }
        virtual std::string getFakeNNMapString() const override;
        virtual std::string getFakeNNMap2ndString() const override;
        virtual std::string getExtensionNNHitMapString() const override;
        virtual std::string getExtensionNNVolMapString() const override;
        virtual std::string getParamNNMapString() const override;
        virtual std::string getParamNNMap2ndString() const override;

    private:

        // Configuration
        Gaudi::Property<unsigned int> m_regionID { this, "regionID", 0, "current region under processing"};  // Current region of interest
        Gaudi::Property<std::string> m_mappingType {this, "mappingType", "FILE", "for now should be FILE only, DB for the future"};
        Gaudi::Property<std::string> m_rmap_path {this, "rmap", "", "path of the region-map file"};
        Gaudi::Property<std::string> m_subrmap_path {this, "subrmap", "", "path of the region-map file for subregions"};
        Gaudi::Property<std::string> m_pmap_path {this, "pmap", "", "path of the PMAP file"};
        Gaudi::Property<std::string> m_modulelut_path {this, "modulemap", "", "path of the ModuleLUT file"};
        Gaudi::Property<std::string> m_NNmap_path_fake {this, "FakeNNonnx1st", "", "path of the NN weighting file for 1st stage"};
        Gaudi::Property<std::string> m_NNmap2nd_path_fake {this, "FakeNNonnx2nd", "", "path of the NN weighting file for 2nd stage"};
        Gaudi::Property<std::string> m_NNmap_path_extension_vol {this, "ExtensionNNVolonnx", "", "path of the NN weighting file"};
        Gaudi::Property<std::string> m_NNmap_path_extension_hit {this, "ExtensionNNHitonnx", "", "path of the NN weighting file"};
        Gaudi::Property<std::string> m_NNmap_path_param {this, "ParamNNonnx1st", "", "path of the NN weighting file for 1st stage"};
        Gaudi::Property<std::string> m_NNmap2nd_path_param {this, "ParamNNonnx2nd", "", "path of the NN weighting file for 2nd stage"};
        Gaudi::Property<std::string> m_radii_path {this, "radiiFile", "", "path of the average radius file" };
        Gaudi::Property<std::string> m_radii2nd_path {this, "radiiFile2nd", "", "path of the average radius file for 2nd stage" };
        Gaudi::Property<std::vector <int> > m_layerOverrides {this, "layerOverride", {}, "Overrides the selection of the 1st stage logical layers in the plane map. Each entry declares a detector layer to use as a logical layer. Specify a detector layer with { SiliconTech * 1000 + DetectorZone * 100 + PhysicalLayer }"};
        Gaudi::Property<bool> m_loadRadii {this, "loadRadii", true, "Whether or not to attempt to read in the idealized radii files" };
        Gaudi::Property<bool> m_loadRegionMap {this, "loadRegionMap", true, "Whether or not to attempt to loda the region maps; if false, isInRegion will always return true" };

            // Map unique pointers
        //vector of pmaps
        std::vector<std::unique_ptr<FPGATrackSimPlaneMap>>  m_pmap_vector_1st; //  pointer to the pmap object for 1st stage
        std::vector<std::unique_ptr<FPGATrackSimPlaneMap>>  m_pmap_vector_2nd; //  pointer to the pmap object for 2nd stage
        virtual size_t GetPlaneMap_1stSliceSize() const override {return m_pmap_vector_1st.size();}
        virtual size_t GetPlaneMap_2ndSliceSize() const override {return m_pmap_vector_2nd.size();}
        std::unique_ptr<FPGATrackSimRegionMap> m_rmap_1st = nullptr; //  pointer to the RMAP object using 1st stage plane map
        std::unique_ptr<FPGATrackSimRegionMap> m_rmap_2nd = nullptr; //  pointer to the RMAP object using 2nd stage plane map
        std::unique_ptr<FPGATrackSimRegionMap> m_subrmap = nullptr;
        std::unique_ptr<FPGATrackSimRegionMap> m_subrmap_2nd = nullptr;
        std::unique_ptr<FPGATrackSimNNMap>     m_NNmap_fake = nullptr;
        std::unique_ptr<FPGATrackSimNNMap>     m_NNmap_param = nullptr;
        std::unique_ptr<FPGATrackSimNNMap>     m_NNmap2nd_fake = nullptr;
        std::unique_ptr<FPGATrackSimNNMap>     m_NNmap2nd_param = nullptr;
        std::unique_ptr<FPGATrackSimNNMap>     m_NNmap_extension_vol = nullptr;
        std::unique_ptr<FPGATrackSimNNMap>     m_NNmap_extension_hit = nullptr;

        size_t m_numberOfPmaps = 0;
        // Helpers
        int readPmapSize(std::ifstream& fileIn);
        int countPmapSize(std::ifstream& fileIn);

        StatusCode checkInputs();
        StatusCode checkAllocs();
};

#endif   // TRIGFPGATrackSimMAPPINGSVC_H
