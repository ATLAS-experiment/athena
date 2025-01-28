
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrkConverter_FPGAClusterConverter_H
#define FPGATrkConverter_FPGAClusterConverter_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrkConverterInterface/IFPGAClusterConverter.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h" 
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

class AtlasDetectorID;
class PixelID;
class SCT_ID;
class Identifier;

namespace InDetDD {
  class PixelDetectorManager;
  class SCT_DetectorManager;
}

namespace InDet {
  class PixelCluster;
  class SCT_Cluster;
}

class FPGATrackSimHit;
class FPGATrackSimCluster;


class FPGAClusterConverter : public extends<AthAlgTool,IFPGAClusterConverter>
 {
  public:
 
    FPGAClusterConverter(const std::string& type, const std::string& name, const IInterface*);
    virtual ~FPGAClusterConverter() = default;
    virtual StatusCode initialize() override final;

    virtual StatusCode convertHits(const std::vector<FPGATrackSimHit>& ,
                                    InDet::PixelClusterCollection &,
                                    InDet::SCT_ClusterCollection &) const override final;
    virtual StatusCode convertHits(const std::vector<const FPGATrackSimHit*>&,
                                    InDet::PixelClusterCollection &,
                                    InDet::SCT_ClusterCollection &) const override final;
    virtual StatusCode convertHits(const std::vector<FPGATrackSimHit>& hits,
                                    xAOD::PixelClusterContainer& pixelCont,
                                    xAOD::StripClusterContainer& SCTCont) const override final;
    virtual StatusCode convertClusters(const std::vector<FPGATrackSimCluster>& ,
                                    InDet::PixelClusterCollection &,
                                    InDet::SCT_ClusterCollection &) const override final;
    virtual StatusCode convertClusters(const std::vector<FPGATrackSimCluster>& cl,
                                    xAOD::PixelClusterContainer& pixelCont,
                                    xAOD::StripClusterContainer& SCTCont) const override final;
    virtual StatusCode convertSpacePoints(const std::vector<FPGATrackSimCluster>& fpgaSPs,
                                    xAOD::SpacePointContainer& SPStripCont,
                                    xAOD::SpacePointContainer& SPPixelCont, 
                                    xAOD::StripClusterContainer& stripClusterCont,
                                    xAOD::PixelClusterContainer& pixelClusterCont) const override final;

    virtual StatusCode createPixelCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, std::unique_ptr<InDet::PixelCluster>&) const override final;
    virtual StatusCode createPixelCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, xAOD::PixelCluster &) const override final;
    virtual StatusCode createSCTCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, std::unique_ptr<InDet::SCT_Cluster>&) const override final;
    virtual StatusCode createSCTCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, xAOD::StripCluster&) const override final;
    virtual StatusCode createPixelCluster(const FPGATrackSimCluster&, std::unique_ptr<InDet::PixelCluster>&) const override final;
    virtual StatusCode createPixelCluster(const FPGATrackSimCluster&, xAOD::PixelCluster& ) const override final;
    virtual StatusCode createSCTCluster(const FPGATrackSimCluster&, std::unique_ptr<InDet::SCT_Cluster>&) const override final;
    virtual StatusCode createSCTCluster(const FPGATrackSimCluster&, xAOD::StripCluster& ) const override final;
    virtual StatusCode createSP(const FPGATrackSimCluster& cl, xAOD::SpacePoint& sp, xAOD::StripClusterContainer& clustersCont ) const override final;
    virtual StatusCode createPixelSPs(xAOD::SpacePointContainer& pixelSPs, xAOD::PixelClusterContainer& clustersCont ) const override final;

    virtual StatusCode getRdoList(std::vector<Identifier> &rdoList, const FPGATrackSimCluster& cluster) const override final;
    virtual StatusCode getRdoList(std::vector<Identifier> &rdoList, const FPGATrackSimHit& hit) const override final;
    virtual StatusCode getStripsInfo(const xAOD::StripCluster& cl, float& halfStripLength, Amg::Vector3D& stripDirection, Amg::Vector3D& stripCenter) const override final;

    SG::ReadHandleKey<FPGATrackSimClusterCollection> m_FPGAClusterKey{this, "FPGATrackSimClusterKey","FPGAClusters","FPGATrackSim Clusters key"};
    SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey{ this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

    bool m_doShift = true; 

  private:

    const PixelID* m_pixelId{nullptr};
    const SCT_ID* m_SCTId{nullptr};
    const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
    const InDetDD::SCT_DetectorManager* m_SCTManager{nullptr};

    ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool {this, "LorentzAngleTool", "SiLorentzAngleTool/SCTLorentzAngleTool", "Tool to retrieve Lorentz angle of SCT"};

    Gaudi::Property<bool> m_skipStripSpacePointFormation {this, "skipStripSpFormation", true, "Should be enabled in case we want to test strip seeding"};
 };

#endif

