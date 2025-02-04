/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MSVVERTEXRECOTOOL_H
#define MSVVERTEXRECOTOOL_H

#include <utility>
#include <optional>
#include <vector>
#include <array>
#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaKernel/RNGWrapper.h"

#include "EventPrimitives/EventPrimitives.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "FourMomUtils/xAODP4Helpers.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonPrepRawData/MdtPrepDataContainer.h"
#include "MuonPrepRawData/RpcPrepDataContainer.h"
#include "MuonPrepRawData/TgcPrepDataContainer.h"

#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkParameters/TrackParameters.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "MSVertexToolInterfaces/IMSVertexRecoTool.h"
#include "MSVertexUtils/MSVertex.h"
#include "MSVertexUtils/Tracklet.h"


namespace Muon {

    class MSVertexRecoTool : virtual public IMSVertexRecoTool, public AthAlgTool {

    public:
        MSVertexRecoTool(const std::string &type, const std::string &name, const IInterface *parent);
        virtual ~MSVertexRecoTool() = default;

        virtual StatusCode initialize(void) override;
        StatusCode findMSvertices(const std::vector<Tracklet> &tracklets, std::vector<std::unique_ptr<MSVertex>> &vertices,
                                  const EventContext &ctx) const override;

        struct TrkCluster {
            double eta{0.};
            double phi{0.};
            unsigned int ntrks{0};
            bool isSystematic{false};
            std::vector<Tracklet> tracks;

            TrkCluster() = default;
        };

    private:
        SG::WriteHandleKey<xAOD::VertexContainer> m_xAODContainerKey{this, "xAODVertexContainer", "MSDisplacedVertex"};

        SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_rpcTESKey{this, "RPCKey", "RPC_Measurements"};
        SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_tgcTESKey{this, "TGCKey", "TGC_Measurements"};
        SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_mdtTESKey{this, "MDTKey", "MDT_DriftCircles"};

        // count hits (total, inwards of the vertex and split by layer) around vertex
        const std::vector<std::string> m_accMDT_str = {"nMDT", "nMDT_inwards", "nMDT_I", "nMDT_E", "nMDT_M", "nMDT_O"};
        const std::vector<std::string> m_accRPC_str = {"nRPC", "nRPC_inwards", "nRPC_I", "nRPC_E", "nRPC_M", "nRPC_O"};
        const std::vector<std::string> m_accTGC_str = {"nTGC", "nTGC_inwards", "nTGC_I", "nTGC_E", "nTGC_M", "nTGC_O"};

        std::vector<SG::AuxElement::Accessor<int>> m_nMDT_accs;
        std::vector<SG::AuxElement::Accessor<int>> m_nRPC_accs;
        std::vector<SG::AuxElement::Accessor<int>> m_nTGC_accs;
   
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", "Random Number Service"};  // Random number service
        ToolHandle<Trk::IExtrapolator> m_extrapolator{this, "MyExtrapolator", "Trk::Extrapolator/AtlasExtrapolator"};


        // cuts to prevent excessive processing timing
        Gaudi::Property<unsigned int> m_maxGlobalTracklets{this, "MaxGlobalTracklets", 40, "maximal number of tracklets in an event"};
        Gaudi::Property<unsigned int> m_maxClusterTracklets{this, "MaxClusterTracklets", 50, "maximal number of tracklets in a cluster"}; // check if this makes sense to be large than m_maxGlobalTracklets 
        // high occupancy definitions
        Gaudi::Property<double> m_ChamberOccupancyMin{this, "MinimumHighOccupancy", 0.25, "minimum occupancy to be considered 'high occupancy'"};
        Gaudi::Property<int> m_minHighOccupancyChambers{this, "MinimumNumberOfHighOccupancy", 2, "number of high occupancy chambers required to be signal like"};
        // clustering properties
        Gaudi::Property<double> m_ClusterdEta{this, "ClusterdEta", 0.7, "eta extend of cluster"};
        Gaudi::Property<double> m_ClusterdPhi{this, "ClusterdPhi", M_PI / 3.*Gaudi::Units::radian, "phi extend of cluster"};
        // options to calculate vertex systematic uncertainty from the tracklet reco uncertainty
        Gaudi::Property<bool> m_doSystematics{this, "DoSystematicUncertainty", false, "find vertex systematic uncertainty"};
        Gaudi::Property<double> m_BarrelTrackletUncert{this, "BarrelTrackletUncertainty", 0.1, "probability of considering a barrel tracklet in the clustering for the systematics reconstruction"};
        Gaudi::Property<double> m_EndcapTrackletUncert{this, "EndcapTrackletUncertainty", 0.1, "probability of considering a endcap tracklet in the clustering for the systematics reconstruction"};
        // properties for vertex fitting 
        Gaudi::Property<double> m_TrackPhiAngle{this, "TrackPhiAngle", 0.0*Gaudi::Units::radian, "nominal phi angle for tracklets in rad"};
        Gaudi::Property<double> m_TrackPhiRotation{this, "TrackPhiRotation", 0.2*Gaudi::Units::radian, "angle to rotate tracklets by for uncertainty estimate in rad"};
        Gaudi::Property<double> m_MaxTrackUncert{this, "MaxTrackUncert", 200.*Gaudi::Units::millimeter, "maximal tracklet uncertainty in mm"};
        Gaudi::Property<double> m_VxChi2ProbCUT{this, "VxChi2ProbabilityCut", 0.05, "chi^2 probability cut"};
        Gaudi::Property<double> m_VertexMinRadialPlane{this, "VertexMinRadialPlane", 3500.*Gaudi::Units::millimeter, "position of first radial plane in mm"};
        Gaudi::Property<double> m_VertexMaxRadialPlane{this, "VertexMaxRadialPlane", 7000.*Gaudi::Units::millimeter, "position of last radial plane in mm"};
        Gaudi::Property<double> m_VxPlaneDist{this, "VertexPlaneDist", 200.*Gaudi::Units::millimeter, "distance between two adjacent planes in mm"};
        Gaudi::Property<double> m_MaxTollDist{this, "MaxTollDist", 300.*Gaudi::Units::millimeter, "maximal distance between tracklet and endcap vertex in mm"};
        Gaudi::Property<bool> m_useOldMSVxEndcapMethod{this, "UseOldMSVxEndcapMethod", false, "use old vertex reconstruction in the endcaps "};
        // number of hits near vertex
        Gaudi::Property<double> m_nMDTHitsEta{this, "nMDTHitsEta", 0.6, "max eta extend between vertex and MDT hit"};
        Gaudi::Property<double> m_nMDTHitsPhi{this, "nMDTHitsPhi", 0.6*Gaudi::Units::radian, "max phi extend between vertex and MDT hit"};
        Gaudi::Property<double> m_nTrigHitsdR{this, "nTrigHitsdR", 0.6*Gaudi::Units::radian, "max delta R between vertex and trigger chamber (RPC or TGC) hit"};
        // minimal good vertex criteria
        Gaudi::Property<int> m_MinMDTHits{this, "MinMDTHits", 250, "minimal number of MDT hits"};
        Gaudi::Property<int> m_MinTrigHits{this, "MinMDTHits", 200, "minimal number of trigger chamber (RPC+TGC) hits"};
        Gaudi::Property<double> m_MaxLxyEndcap{this, "MaxLxyEndcap", 10000*Gaudi::Units::millimeter, "maximal transverse distance for endcap vertex in mm"};
        Gaudi::Property<double> m_MinZEndcap{this, "MinZEndcap", 8000*Gaudi::Units::millimeter, "minimal longitudinal distance for endcap vertex in mm"};
        Gaudi::Property<double> m_MaxZEndcap{this, "MaxZEndcap", 14000*Gaudi::Units::millimeter, "maximal longitudinal distance for endcap vertex in mm"};


        // group tracklets into clusters -- vertex reco runs on each cluster of tracklets
        std::vector<TrkCluster> findTrackClusters(const std::vector<Tracklet> &tracklets) const;  
        std::optional<TrkCluster> ClusterizeTracks(std::vector<Tracklet> &tracks) const;  // core algorithm for creating the clusters
        // barrel vertex reco algorithm
        void MSVxFinder(const std::vector<Tracklet> &tracklets, std::unique_ptr<MSVertex> &vtx, const EventContext &ctx) const;
        // endcap vertex reco algorithm
        void MSStraightLineVx(const std::vector<Tracklet> &trks, std::unique_ptr<MSVertex> &vtx, const EventContext &ctx) const;
        void MSStraightLineVx_oldMethod(const std::vector<Tracklet> &trks, std::unique_ptr<MSVertex> &vtx, const EventContext &ctx) const;
        static Amg::Vector3D VxMinQuad(const std::vector<Tracklet> &tracks) ;  // endcap vertex reco core
        std::vector<Tracklet> RemoveBadTrk(const std::vector<Tracklet> &tracklets, const Amg::Vector3D &Vx) const;  
        bool EndcapHasBadTrack(const std::vector<Tracklet> &tracklets, const Amg::Vector3D &Vx) const;
        // tools
        void HitCounter(MSVertex *MSRecoVx, const EventContext &ctx) const;  // counts MDT, RPC & TGC around a reco'd vertex
        double vxPhiFinder(const double theta, const double phi, const EventContext &ctx) const;  // vertex phi location reco algorithm
        std::vector<Tracklet> getTracklets(const std::vector<Tracklet> &trks, const std::set<int> &tracklet_subset) const;
        void dressVtxHits(xAOD::Vertex* xAODVx, const std::vector<SG::AuxElement::Accessor<int>>& accs, const std::vector<int>& hits) const;
        StatusCode FillOutputContainer(const std::vector<std::unique_ptr<MSVertex>> &, SG::WriteHandle<xAOD::VertexContainer> &xAODVxContainer) const;
    };

}  // namespace Muon

#endif
