/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONSYSTEMEXTENSIONTOOL_H
#define MUON_MUONSYSTEMEXTENSIONTOOL_H

#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonLayerEvent/MuonLayerSurface.h"
#include "MuonRecToolInterfaces/IMuonSystemExtensionTool.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkParameters/TrackParameters.h"
#include "xAODTracking/TrackParticle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"


namespace Trk {
    class Surface;
}

namespace Muon {

    class MuonSystemExtension;

    class MuonSystemExtensionTool : public extends<AthAlgTool, IMuonSystemExtensionTool> {
    public:
        typedef std::vector<MuonLayerSurface> SurfaceVec;

        /** Default AlgTool functions */
        using base_class::base_class;
        virtual ~MuonSystemExtensionTool() = default;
        StatusCode initialize() override;

        /** get muon system extension */
        bool muonSystemExtension(const EventContext& ctx, SystemExtensionCache& cache) const override;

        bool muonLayerInterSections(const EventContext& ctx, 
                                    const MuonCombined::TagBase& cmb_tag,
                                    SystemExtensionCache& cache) const override;                                   
       
    private:
        /** initialize geometry */
        bool initializeGeometry();
        bool initializeGeometryBarrel(int sector, const Amg::Transform3D& sectorRotation);
        bool initializeGeometryEndcap(int sector, MuonStationIndex::DetectorRegionIndex regionIndex,
                                      const Amg::Transform3D& sectorRotation);

        /** get surfaces to be intersected for a given start parameters */
        SurfaceVec getSurfacesForIntersection(const Trk::TrackParameters& muonEntryPars, const SystemExtensionCache& cache) const;
        
        ToolHandle<Trk::IParticleCaloExtensionTool> m_caloExtensionTool{
            this,
            "ParticleCaloExtensionTool",
            "Trk::ParticleCaloExtensionTool/ParticleCaloExtensionTool",
        };
        ToolHandle<Trk::IExtrapolator> m_extrapolator{this,"Extrapolator",""};
        
        PublicToolHandle<MuonEDMPrinterTool> m_printer{this, "Printer", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"};
        
        ServiceHandle<IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ServiceHandle<IMuonEDMHelperSvc> m_edmHelperSvc{this,"edmHelper","Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
                                                        "Handle to the service providing the IMuonEDMHelperSvc interface"};

        /** reference surfaces per region and sector */
        using DetRegIdx = MuonStationIndex::DetectorRegionIndex;
        std::array<std::array<SurfaceVec, 16> , 
                   MuonStationIndex::toInt(DetRegIdx::DetectorRegionIndexMax)> m_referenceSurfaces{};

        /** sector mapping helper */
        MuonSectorMapping m_sectorMapping;
    };
}  // namespace Muon

#endif
