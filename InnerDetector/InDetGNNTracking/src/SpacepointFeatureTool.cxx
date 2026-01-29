/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SpacepointFeatureTool.h"
#include <cmath>

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetPrepRawData/SiCluster.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "InDetPrepRawData/SCT_Cluster.h"

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"

InDet::SpacepointFeatureTool::SpacepointFeatureTool(const std::string& type,
                                                    const std::string& name,
                                                    const IInterface* parent)
    : base_class(type, name, parent), m_SCT_ID(nullptr) {
  declareInterface<ISpacepointFeatureTool>(this);
}

StatusCode InDet::SpacepointFeatureTool::initialize() {
  // Grab PixelID helper
  ATH_CHECK (detStore()->retrieve(m_pixelID, "PixelID") );

  // Grab SCT_ID helper
  ATH_CHECK (detStore()->retrieve(m_SCT_ID,"SCT_ID") );
  
  return StatusCode::SUCCESS;
}

std::map<std::string, float> InDet::SpacepointFeatureTool::getFeatures(
    const Trk::SpacePoint* sp) const {
    // see the naming conventions in ACORN
    // https://gitlab.cern.ch/gnn4itkteam/acorn/-/blob/dev/acorn/stages/data_reading/models/athena_root_utils.py?ref_type=heads#L19
    std::map<std::string, float> features;
    features["x"] = sp->globalPosition().x();
    features["y"] = sp->globalPosition().y();
    features["z"] = sp->globalPosition().z();
    features["r"] = sp->r();
    features["phi"] = sp->phi();
    features["eta"] = sp->eta();

    const InDet::SiCluster *cluster_1 = static_cast<const InDet::SiCluster *>(sp->clusterList().first);
    // get the cluster position
    features["cluster_x_1"] = cluster_1->globalPosition().x();
    features["cluster_y_1"] = cluster_1->globalPosition().y();
    features["cluster_z_1"] = cluster_1->globalPosition().z();
    features["cluster_r_1"] = std::sqrt(cluster_1->globalPosition().x() * cluster_1->globalPosition().x() + cluster_1->globalPosition().y() * cluster_1->globalPosition().y());
    features["cluster_phi_1"] = std::atan2(cluster_1->globalPosition().y(), cluster_1->globalPosition().x());
    features["cluster_eta_1"] = -1 * std::log(std::tan(0.5 * std::atan2(features["cluster_r_1"], features["cluster_z_1"])));

    const InDet::SiCluster *cluster_2 = static_cast<const InDet::SiCluster *>(sp->clusterList().second);
    bool isStrip = (cluster_2 != nullptr);

    // get the cluster shape
    float charge_count_1 = 0;
    int pixel_count_1 = 0;
    float loc_eta_1 = 0, loc_phi_1 = 0; // clusterShape: [leta, lphi]
    float glob_eta_1 = 0, glob_phi_1 = 0; // clusterShape: [geta, gphi]
    float localDir0_1 = 0, localDir1_1 = 0, localDir2_1 = 0; // clusterShape: [lengthDir0, lengthDir1, lengthDir2]
    float lengthDir0_1 = 0, lengthDir1_1 = 0, lengthDir2_1 = 0; // clusterShape: [lengthDir0, lengthDir1, lengthDir2]
    float eta_angle_1 = 0, phi_angle_1 = 0;
    // module ID
    uint64_t module_id = cluster_1->detectorElement()->identify().get_compact();
    features["module_id"] = static_cast<float>(module_id);

    // 1: {hardware: PIXEL, barrel_endcap: -2}
    // 2: {hardware: STRIP, barrel_endcap: -2}
    // 3: {hardware: PIXEL, barrel_endcap: 0}
    // 4: {hardware: STRIP, barrel_endcap: 0}
    // 5: {hardware: PIXEL, barrel_endcap: 2}
    // 6: {hardware: STRIP, barrel_endcap: 2}
    int region = 0; 

    if (!isStrip) {
      const InDet::PixelCluster* cluster = dynamic_cast<const InDet::PixelCluster*>(cluster_1);
      const InDetDD::SiDetectorElement *element = cluster->detectorElement();
      int barrel_endcap = m_pixelID->barrel_ec(cluster->identify());
      switch (barrel_endcap) {
        case -2:
          region = 1;
          break;
        case 0:
          region = 3;
          break;
        case 2:
          region = 5;
          break;
        default:
          ATH_MSG_ERROR("Unknown barrel_endcap: " << barrel_endcap);
          break;
      }


      // boundary of the cluster in the local module.
      int min_eta = 999;
      int min_phi = 999;
      int max_eta = -999;
      int max_phi = -999;
      for (unsigned int rdo = 0; rdo < cluster->rdoList().size(); rdo++) {
        charge_count_1 += cluster->totList().at(rdo);
        ++ pixel_count_1; 

        const auto &rdoID = cluster->rdoList().at(rdo);
        InDetDD::SiCellId cellID = element->cellIdFromIdentifier(rdoID);
        int phi = cellID.phiIndex();
        int eta = cellID.etaIndex();
        if (min_eta > eta)
          min_eta = eta;
        if (min_phi > phi)
          min_phi = phi;
        if (max_eta < eta)
          max_eta = eta;
        if (max_phi < phi)
          max_phi = phi;
      }

      
      const InDetDD::PixelModuleDesign *design = dynamic_cast<const InDetDD::PixelModuleDesign *>(&element->design());

      InDetDD::SiLocalPosition localPos_entry = design->localPositionOfCell(InDetDD::SiCellId(min_phi, min_eta));
      InDetDD::SiLocalPosition localPos_exit = design->localPositionOfCell(InDetDD::SiCellId(max_phi, max_eta));

      Amg::Vector3D localStartPosition(localPos_entry.xEta() - 0.5 * element->etaPitch(),
                                        localPos_entry.xPhi() - 0.5 * element->phiPitch(),
                                          -0.5 * element->thickness());
      Amg::Vector3D localEndPosition(localPos_exit.xEta() + 0.5 * element->etaPitch(),
                                      localPos_exit.xPhi() + 0.5 * element->phiPitch(), 0.5 * element->thickness());

      // local direction in local coordinates
      Amg::Vector3D localDirection = localEndPosition - localStartPosition;
      cartesion_to_spherical(localDirection, loc_eta_1, loc_phi_1);

      localDir0_1 = localDirection[0];
      localDir1_1 = localDirection[1];
      localDir2_1 = localDirection[2];

      Amg::Vector3D globalStartPosition = element->globalPosition(localStartPosition);
      Amg::Vector3D globalEndPosition = element->globalPosition(localEndPosition);

      Amg::Vector3D direction = globalEndPosition - globalStartPosition;
      cartesion_to_spherical(direction, glob_eta_1, glob_phi_1);

      Amg::Vector3D my_phiax = element->phiAxis();
      Amg::Vector3D my_etaax = element->etaAxis();
      Amg::Vector3D my_normal = element->normal();

      float trkphicomp = direction.dot(my_phiax);
      float trketacomp = direction.dot(my_etaax);
      float trknormcomp = direction.dot(my_normal);
      eta_angle_1 = atan2(trknormcomp, trketacomp);
      phi_angle_1 = atan2(trknormcomp, trkphicomp);
    } else {
      // first cluster of the strip detector.
      auto status = getSCTClusterShapeInfo_fn(cluster_1, charge_count_1, pixel_count_1, loc_eta_1, loc_phi_1, 
          glob_eta_1, glob_phi_1, 
          localDir0_1, localDir1_1, localDir2_1,
          lengthDir0_1, lengthDir1_1, lengthDir2_1,
          eta_angle_1, phi_angle_1
          );
      if (status.isFailure()) {
        ATH_MSG_ERROR("Failed at " << __LINE__ << " of getting SCT cluster shape info.");
      }
      auto cluster = dynamic_cast<const InDet::SCT_Cluster*>(cluster_1);
      int barrel_endcap = m_SCT_ID->barrel_ec(cluster->identify());
      switch (barrel_endcap) {
        case -2:
          region = 2;
          break;
        case 0:
          region = 4;
          break;
        case 2:
          region = 6;
          break;
        default:
          ATH_MSG_ERROR("Unknown barrel_endcap: " << barrel_endcap);
          break;
      }
    }
    features["charge_count_1"] = charge_count_1;
    features["count_1"] = pixel_count_1;
    features["loc_eta_1"] = loc_eta_1;
    features["loc_phi_1"] = loc_phi_1;
    features["glob_eta_1"] = glob_eta_1;
    features["glob_phi_1"] = glob_phi_1;
    features["localDir0_1"] = localDir0_1;
    features["localDir1_1"] = localDir1_1;
    features["localDir2_1"] = localDir2_1;
    features["lengthDir0_1"] = lengthDir0_1;
    features["lengthDir1_1"] = lengthDir1_1;
    features["lengthDir2_1"] = lengthDir2_1;
    features["eta_angle_1"] = eta_angle_1;
    features["phi_angle_1"] = phi_angle_1;
    features["region"] = region;

    if (isStrip) {
      features["cluster_x_2"] = cluster_2->globalPosition().x();
      features["cluster_y_2"] = cluster_2->globalPosition().y();
      features["cluster_z_2"] = cluster_2->globalPosition().z();
      features["cluster_r_2"] = std::sqrt(cluster_2->globalPosition().x() * cluster_2->globalPosition().x() + cluster_2->globalPosition().y() * cluster_2->globalPosition().y());
      features["cluster_phi_2"] = std::atan2(cluster_2->globalPosition().y(), cluster_2->globalPosition().x());
      features["cluster_eta_2"] = -1 * std::log(std::tan(0.5 * std::atan2(features["cluster_r_2"], features["cluster_z_2"])));
      // the second cluster of the strip detector.
      float charge_count_2 = 0;
      int pixel_count_2 = 0;
      float loc_eta_2 = 0, loc_phi_2 = 0; // clusterShape: [leta, lphi]
      float glob_eta_2 = 0, glob_phi_2 = 0; // clusterShape: [geta, gphi]
      float localDir0_2 = 0, localDir1_2 = 0, localDir2_2 = 0; // clusterShape: [lengthDir0, lengthDir1, lengthDir2]
      float lengthDir0_2 = 0, lengthDir1_2 = 0, lengthDir2_2 = 0; // clusterShape: [lengthDir0, lengthDir1, lengthDir2]
      float eta_angle_2 = 0, phi_angle_2 = 0;
      auto status = getSCTClusterShapeInfo_fn(cluster_2, charge_count_2, pixel_count_2, loc_eta_2, loc_phi_2, 
          glob_eta_2, glob_phi_2, 
          localDir0_2, localDir1_2, localDir2_2,
          lengthDir0_2, lengthDir1_2, lengthDir2_2,
          eta_angle_2, phi_angle_2
          );
      if (status.isFailure()) {
        ATH_MSG_ERROR("Failed at " << __LINE__ << " of getting SCT cluster shape info.");
      }

      features["charge_count_2"] = charge_count_2;
      features["count_2"] = pixel_count_2;
      features["loc_eta_2"] = loc_eta_2;
      features["loc_phi_2"] = loc_phi_2;
      features["glob_eta_2"] = glob_eta_2;
      features["glob_phi_2"] = glob_phi_2;
      features["localDir0_2"] = localDir0_2;
      features["localDir1_2"] = localDir1_2;
      features["localDir2_2"] = localDir2_2;
      features["lengthDir0_2"] = lengthDir0_2;
      features["lengthDir1_2"] = lengthDir1_2;
      features["lengthDir2_2"] = lengthDir2_2;
      features["eta_angle_2"] = eta_angle_2;
      features["phi_angle_2"] = phi_angle_2;
    } else {
      // copy the cluster 1 to cluster 2.
      features["cluster_x_2"] = features["cluster_x_1"];
      features["cluster_y_2"] = features["cluster_y_1"];
      features["cluster_z_2"] = features["cluster_z_1"];
      features["cluster_r_2"] = features["cluster_r_1"];
      features["cluster_phi_2"] = features["cluster_phi_1"];
      features["cluster_eta_2"] = features["cluster_eta_1"];
      features["charge_count_2"] = features["charge_count_1"];
      features["count_2"] = features["count_1"];
      features["loc_eta_2"] = features["loc_eta_1"];
      features["loc_phi_2"] = features["loc_phi_1"];
      features["glob_eta_2"] = features["glob_eta_1"];
      features["glob_phi_2"] = features["glob_phi_1"];
      features["localDir0_2"] = features["localDir0_1"];
      features["localDir1_2"] = features["localDir1_1"];
      features["localDir2_2"] = features["localDir2_1"];
      features["lengthDir0_2"] = features["lengthDir0_1"];
      features["lengthDir1_2"] = features["lengthDir1_1"];
      features["lengthDir2_2"] = features["lengthDir2_1"];
      features["eta_angle_2"] = features["eta_angle_1"];
      features["phi_angle_2"] = features["phi_angle_1"];
    }
    return features;
}


void InDet::SpacepointFeatureTool::cartesion_to_spherical(const Amg::Vector3D &xyzVec, float &eta_, float &phi_) const {
  float r3 = 0;
  for (int idx = 0; idx < 3; ++idx) {
    r3 += xyzVec[idx] * xyzVec[idx];
  }
  r3 = std::sqrt(r3);
  phi_ = std::atan2(xyzVec[1], xyzVec[0]);
  float theta_ = std::acos(xyzVec[2] / r3);
  eta_ = std::log(std::tan(0.5 * theta_));  // a bug here, but keep it so it is consistent with the DumpObject.
}

StatusCode InDet::SpacepointFeatureTool::getSCTClusterShapeInfo_fn(const InDet::SiCluster *si_cluster, 
    float &charge_count, int &pixel_count, float &loc_eta, float &loc_phi, 
    float &glob_eta, float &glob_phi, 
    float &localDir0, float &localDir1, float &localDir2,
    float &lengthDir0, float &lengthDir1, float &lengthDir2,
    float &eta_angle, float &phi_angle
    ) const 
{
    const InDet::SCT_Cluster* cluster = dynamic_cast<const InDet::SCT_Cluster*>(si_cluster);
    const InDetDD::SiDetectorElement *element = cluster->detectorElement();
    Amg::Vector2D locpos = cluster->localPosition();
    std::pair<Amg::Vector3D, Amg::Vector3D> ends(
        element->endsOfStrip(InDetDD::SiLocalPosition(locpos.y(), locpos.x(), 0)));
    Amg::Vector3D JanDirection = ends.second - ends.first;
    lengthDir0 = JanDirection[0];
    lengthDir1 = JanDirection[1];
    lengthDir2 = JanDirection[2];

    // get local directions.
    // retrieve cluster shape
    const InDetDD::SCT_ModuleSideDesign *design(
      dynamic_cast<const InDetDD::SCT_ModuleSideDesign *>(&element->design()));
    if (not design) {
      ATH_MSG_ERROR("Failed at " << __LINE__ << " of accessing SCT ModuleSide Design");
      return StatusCode::FAILURE;
    }

    int min_strip = 999;
    int max_strip = -999;

    charge_count = 0;
    pixel_count = 0;

    for (unsigned int rdo = 0; rdo < cluster->rdoList().size(); rdo++) {
      const auto &rdoID = cluster->rdoList().at(rdo);

      int strip = m_SCT_ID->strip(rdoID);

      if (min_strip > strip) min_strip = strip;
      if (max_strip < strip) max_strip = strip;

      ++pixel_count;
    }

    InDetDD::SiLocalPosition localPos_entry = design->localPositionOfCell(InDetDD::SiCellId(min_strip));
    InDetDD::SiLocalPosition localPos_exit = design->localPositionOfCell(InDetDD::SiCellId(max_strip));

    Amg::Vector3D localStartPosition(localPos_entry.xEta() - 0.5 * element->etaPitch(),
                                      localPos_entry.xPhi() - 0.5 * element->phiPitch(),
                                      -0.5 * element->thickness());
    Amg::Vector3D localEndPosition(localPos_exit.xEta() + 0.5 * element->etaPitch(),
                                    localPos_exit.xPhi() + 0.5 * element->phiPitch(), 0.5 * element->thickness());

    Amg::Vector3D localDirection = localEndPosition - localStartPosition;
    cartesion_to_spherical(localDirection, loc_eta, loc_phi);
    localDir0 = localDirection[0];
    localDir1 = localDirection[1];
    localDir2 = localDirection[2];

    Amg::Vector3D globalStartPosition = element->globalPosition(localStartPosition);
    Amg::Vector3D globalEndPosition = element->globalPosition(localEndPosition);

    Amg::Vector3D direction = globalEndPosition - globalStartPosition;
    cartesion_to_spherical(direction, glob_eta, glob_phi);

    Amg::Vector3D my_phiax = element->phiAxis();
    Amg::Vector3D my_etaax = element->etaAxis();
    Amg::Vector3D my_normal = element->normal();

    float trkphicomp = direction.dot(my_phiax);
    float trketacomp = direction.dot(my_etaax);
    float trknormcomp = direction.dot(my_normal);
    phi_angle = atan2(trknormcomp, trkphicomp);
    eta_angle = atan2(trknormcomp, trketacomp);

    return StatusCode::SUCCESS;
}