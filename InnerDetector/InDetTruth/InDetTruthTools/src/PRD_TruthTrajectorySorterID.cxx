/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTruthTools/PRD_TruthTrajectorySorterID.h"
#include "InDetPrepRawData/SCT_Cluster.h"
#include "InDetPrepRawData/TRT_DriftCircle.h"
#include "InDetPrepRawData/PixelCluster.h"

using Trk::PrepRawDataType;

InDet::PRD_TruthTrajectorySorterID::PRD_TruthTrajectorySorterID(const std::string& t, const std::string& n, const IInterface* p) :
AthAlgTool(t,n,p)
{
declareInterface<Trk::IPRD_TruthTrajectoryManipulator>(this);

}

StatusCode InDet::PRD_TruthTrajectorySorterID::initialize() {
  ATH_MSG_VERBOSE("Initializing ...");
  return StatusCode::SUCCESS;
}

StatusCode InDet::PRD_TruthTrajectorySorterID::finalize() {
  ATH_MSG_VERBOSE("Finalizing ...");
  return StatusCode::SUCCESS;
}

class mysort{
  public:
   bool operator()(const Trk::PrepRawData *prd1,const Trk::PrepRawData *prd2) {
     PrepRawDataType t1 = prd1->prdType();
     PrepRawDataType t2 = prd2->prdType();
     //Pixels come before anything else
     if (t1 == PrepRawDataType::PixelCluster && t2 != PrepRawDataType::PixelCluster) return true;
     if (t1 != PrepRawDataType::PixelCluster && t2 == PrepRawDataType::PixelCluster) return false;
     //
     //they are both pixels
     if (t1 == PrepRawDataType::PixelCluster && t2 == PrepRawDataType::PixelCluster){
       const InDet::PixelCluster* pixclus1=static_cast<const InDet::PixelCluster*>(prd1);
       const InDet::PixelCluster* pixclus2=static_cast<const InDet::PixelCluster*>(prd2);
       //barrel comes first, before endcap
       if (pixclus1->detectorElement()->isBarrel() && pixclus2->detectorElement()->isEndcap()) return true;
       if (pixclus1->detectorElement()->isEndcap() && pixclus2->detectorElement()->isBarrel()) return false;
       if (pixclus1->detectorElement()->isBarrel()) return (pixclus1->globalPosition().perp()<pixclus2->globalPosition().perp());
       return (std::abs(pixclus1->globalPosition().z())<std::abs(pixclus2->globalPosition().z()));
     }
     //SCT comes before TRT
     if (t1 ==  PrepRawDataType::SCT_Cluster && t2 == PrepRawDataType::TRT_DriftCircle) return true;
     if (t1 == PrepRawDataType::TRT_DriftCircle && t2 == PrepRawDataType::SCT_Cluster) return false;
     //
     //they are both SCT_Cluster
     if (t1 == PrepRawDataType::SCT_Cluster && t2 == PrepRawDataType::SCT_Cluster){
       const InDet::SCT_Cluster* sctclus1=static_cast<const InDet::SCT_Cluster*>(prd1);
       const InDet::SCT_Cluster* sctclus2=static_cast<const InDet::SCT_Cluster*>(prd2);
       if (sctclus1->detectorElement()->isBarrel() && sctclus2->detectorElement()->isEndcap()) return true;
       if (sctclus1->detectorElement()->isEndcap() && sctclus2->detectorElement()->isBarrel()) return false;
       if (sctclus1->detectorElement()->isBarrel()) return (sctclus1->globalPosition().perp()<sctclus2->globalPosition().perp());
       return (std::abs(sctclus1->globalPosition().z())<std::abs(sctclus2->globalPosition().z()));
     }
     //only case left: they are both TRT
     if (t1 == PrepRawDataType::TRT_DriftCircle && t2 == PrepRawDataType::TRT_DriftCircle){
       const InDet::TRT_DriftCircle* trtclus1=static_cast<const InDet::TRT_DriftCircle*>(prd1);
       const InDet::TRT_DriftCircle* trtclus2=static_cast<const InDet::TRT_DriftCircle*>(prd2);
       if (trtclus1->detectorElement()->type()==InDetDD::TRT_BaseElement::BARREL && trtclus2->detectorElement()->type()==InDetDD::TRT_BaseElement::ENDCAP) return true;
       if (trtclus1->detectorElement()->type()==InDetDD::TRT_BaseElement::ENDCAP && trtclus2->detectorElement()->type()==InDetDD::TRT_BaseElement::BARREL) return false;
       Amg::Vector3D pos1=trtclus1->detectorElement()->center(trtclus1->identify());
       Amg::Vector3D pos2=trtclus2->detectorElement()->center(trtclus2->identify());
       if (trtclus1->detectorElement()->type()==InDetDD::TRT_BaseElement::BARREL) return (pos1.perp()<pos2.perp());
       else return (std::abs(pos1.z())<std::abs(pos2.z()));
     }
     //when all else fails
     return true;
  }

};

bool InDet::PRD_TruthTrajectorySorterID::manipulateTruthTrajectory( Trk::PRD_TruthTrajectory &prdvec ) const{
  std::stable_sort(prdvec.prds.begin(),prdvec.prds.end(),mysort());
  return true;
}
