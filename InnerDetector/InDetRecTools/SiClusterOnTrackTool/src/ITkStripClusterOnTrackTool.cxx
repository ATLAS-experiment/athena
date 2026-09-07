/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class ITkStripClusterOnTrackTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// AlgTool used for ITkStripClusterOnTrack object production
///////////////////////////////////////////////////////////////////

#include "SiClusterOnTrackTool/ITkStripClusterOnTrackTool.h"

#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/AnnulusBounds.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "TrkRIO_OnTrack/ErrorScalingCast.h"

#include <cmath>

using CLHEP::micrometer;
using CLHEP::deg;

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

ITk::StripClusterOnTrackTool::StripClusterOnTrackTool
  (const std::string &t, const std::string &n, const IInterface *p) :
  AthAlgTool(t, n, p) {
  declareInterface<IRIO_OnTrackCreator>(this);
}

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode
ITk::StripClusterOnTrackTool::initialize() {
  StatusCode sc = AlgTool::initialize();

  ATH_MSG_DEBUG("Error strategy set to ");
  switch (m_option_errorStrategy) {
  case -1:  ATH_MSG_DEBUG("keep the PRD errors");
    break;

  case  0:  ATH_MSG_DEBUG("apply width/sqrt(12) as errors");
    break;

  default:  ATH_MSG_ERROR(" -- NO, UNKNOWN. Pls check jobOptions!");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG(" will be applied during ITkStripClusterOnTrack making");

 ATH_MSG_DEBUG("Position correction strategy set to ");
  switch (m_option_correctionStrategy) {
  case -1:  ATH_MSG_DEBUG("keep the global position as evaluated");
    break;

  default:  ATH_MSG_ERROR(" -- NO, UNKNOWN. Pls check jobOptions!");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG(" will be applied during ITkStripClusterOnTrack making");


  // get the error scaling tool
  if (!m_stripErrorScalingKey.key().empty()) {
    ATH_CHECK(m_stripErrorScalingKey.initialize());
    ATH_MSG_DEBUG("Detected need for scaling ITkStrip errors.");
  }

  return sc;
}

///////////////////////////////////////////////////////////////////
// Trk::ITkStripClusterOnTrack  production
///////////////////////////////////////////////////////////////////

InDet::SCT_ClusterOnTrack *
ITk::StripClusterOnTrackTool::correct
  (const Trk::PrepRawData &rio, const Trk::TrackParameters &trackPar, const EventContext& ctx ) const {
  const InDet::SCT_Cluster *cluster = nullptr;

  if (!(cluster = dynamic_cast<const InDet::SCT_Cluster *> (&rio))) {
    ATH_MSG_WARNING("Attempt to correct RIO which is not SCT_Cluster with ITk::StripClusterOnTrackTool: returning nullptr");
    return nullptr;
  }

  ATH_MSG_VERBOSE("STARTING CLUSTER ON TRACK CORRECTION... " << __func__ << "  " << __LINE__);
  ATH_MSG_VERBOSE(" DUMPING CLUSTER POSITION / COVARIANCE: ");
  ATH_MSG_VERBOSE("STRIP CLUSTER POS --> " << cluster->localPosition()[0] << ", " << cluster->localPosition()[1]);
  ATH_MSG_VERBOSE("STRIP CLUSTER COV --> " << cluster->localCovariance()(0, 0) << ", " << cluster->localCovariance()(0, 1));
  ATH_MSG_VERBOSE("STRIP CLUSTER COV --> " << cluster->localCovariance()(1, 0) << ", " << cluster->localCovariance()(1, 1));
  ATH_MSG_VERBOSE("STRIP CLUSTER GLOBAL POSITION = " << cluster->globalPosition().x() << ", " << cluster->globalPosition().y() << ", " << cluster->globalPosition().z());

  // Get pointer to detector element
  //
  const InDetDD::SiDetectorElement *detectorElement = cluster->detectorElement();
  if (!detectorElement) {
    return nullptr;
  }

  // Get local position of track
  //
  Amg::Vector2D loct = trackPar.localPosition();
  double sinAlpha = detectorElement->sinStereoLocal(cluster->localPosition());
  double cosAlpha = std::sqrt(1 - sinAlpha * sinAlpha);
  Amg::Vector3D localstripdir(-sinAlpha, cosAlpha, 0.);
  ATH_MSG_VERBOSE("STRIP DIRECTION = " << localstripdir[0] << ", " << localstripdir[1]);
  Amg::Vector3D globalstripdir = trackPar.associatedSurface().transform().linear() * localstripdir;

  // Evaluate distance between cluster and estimated track parameters
  // used later on to estimate the corrected cluster position
  double distance = (trackPar.position() - cluster->globalPosition()).mag();

  ATH_MSG_VERBOSE(" DUMPING TRACK PARAMETER POSITION / COVARIANCE: ");
  ATH_MSG_VERBOSE("TRACK PAR LOCAL POS = " << loct[0] << ", " << loct[1]);
  ATH_MSG_VERBOSE("TRACK PAR GLOBAL POSITION = " << trackPar.position().x() << ", " << trackPar.position().y() << ", " << trackPar.position().z());

  // phi pitch in radians, for endcap modules
  double phiPitchInRad = 0.;

  // barrel or endcap treatment
  if (detectorElement->isBarrel()) {
    // barrel treatment:
    // get the module half length from the associated surface bounds
    const Trk::SurfaceBounds *bounds = &trackPar.associatedSurface().bounds();
    double boundsy = (static_cast<const Trk::RectangleBounds *>(bounds))->halflengthY();
    ATH_MSG_VERBOSE("BARREL ====>>>> DISTANCE*COSALPHA / HALF LENGTH --> " << distance*cosAlpha << " / " << boundsy);
    // Check if distance between track parameter local position
    // and cluster position is larger than surface bounds (including local stereo angle).
    // If so, set distance to maximum (- tolerance)
    if (distance*cosAlpha > boundsy){
      ATH_MSG_VERBOSE("DISTANCE TO LARGE COMPARED TO BOUNDS, SETTING TO MAXIMUM");
      distance = boundsy/cosAlpha - 1.; // use 1 mm as tolerance parameter
    }
    // if local position is negative, also the distance has to be negative
    if (loct.y() < 0)
      distance = -distance;
  } else {
    // endcap treatment:
    // for annuli do something different, since we already have in-sensor
    // stereo rotations which strip length accounts for
    const InDetDD::StripStereoAnnulusDesign * design =
      static_cast<const InDetDD::StripStereoAnnulusDesign *> (&detectorElement->design());
    const InDetDD::SiCellId & siCellId = detectorElement->cellIdOfPosition(cluster->localPosition());
    double striphalflength = design->stripLength(siCellId) / 2.0;
    ATH_MSG_VERBOSE("ENDCAP ====>>>> DISTANCE / STRIP HALF LENGTH --> " << distance << " / " << striphalflength);
    // caching phi pitch in radians
    phiPitchInRad = design->phiWidth()/design->diodesInRow(0);
    // Check if distance between track parameter local position
    // and cluster position is larger than strip length.
    // If so, set distance to maximum (- tolerance)
    if (distance > striphalflength) {
      ATH_MSG_VERBOSE("DISTANCE TO LARGE COMPARED TO BOUNDS, SETTING TO MAXIMUM");
      distance = striphalflength - 1.; // use 1 mm as tolerance parameter
    }
  }

  Amg::MatrixX prevCov = cluster->localCovariance();
  // Local position and error matrix production
  //
  constexpr double ONE_TWELFTH= 1.0/12.;
  if (m_option_errorStrategy > -1) {
    Amg::MatrixX mat(2, 2);
    mat.setZero();
    const InDet::SiWidth width = cluster->width();
    switch (m_option_errorStrategy) {
    case 0:
      // apply width/sqrt(12) as errors
      mat(0, 0) = std::pow(width.phiR(), 2) * ONE_TWELFTH;
      mat(1, 1) = std::pow(width.z(), 2) * ONE_TWELFTH;
      break;

    default:
      // don't do anything....
      break;
    }

    ATH_MSG_VERBOSE("CLUSTER ON TRACK COVARIANCE = " << mat(0, 0) << ", " << mat(0, 1) );
    ATH_MSG_VERBOSE("                              " << mat(1, 0) << ", " << mat(1, 1) );

    // error matrix rotation for endcap clusters
    if (not detectorElement->isBarrel()) {
      // the cluster covariance is expressed in the module
      // frame and needs to be rotated into the strip frame.
      // This rotation is done using the jacobian:
      //
      //  J = (  cos(alpha)     sin(alpha)
      //         -sin(alpha)    cos(alpha)  )
      //
      // The rotated covariance becomes
      // C = J C_{0} J^T
      //
      // where C_{0} is the cluster covariance,
      // and alpha is the local stereo angle.
      //
      // C_{0} = ( var_{0}^2     0
      //              0      var_{1}^2  )
      //
      // var_{0} also accounts for the pitch scaling.
      //
      double sinAlpha = detectorElement->sinStereoLocal(cluster->localPosition());
      double sinAlpha2 = sinAlpha * sinAlpha;
      double cosAlpha2 = (1. - sinAlpha) * (1. + sinAlpha);;
      double weight = detectorElement->phiPitch(cluster->localPosition()) / detectorElement->phiPitch();
      double v0 = mat(0, 0) * weight * weight;
      double v1 = mat(1, 1);
      mat(0, 0) = (cosAlpha2 * v0 + sinAlpha2 * v1);
      mat(1, 0) = (sinAlpha * std::sqrt(cosAlpha2) * (v0 - v1));
      mat(0, 1) = mat(1, 0);
      mat(1, 1) = (sinAlpha2 * v0 + cosAlpha2 * v1);

      ATH_MSG_VERBOSE(" ROTATION OF ENDCAP STRIP CLUSTER COVARIANCE");
      ATH_MSG_VERBOSE("sinAlpha / sinAlpha2 / cosAlpha2 / weight = " << sinAlpha << " / " << sinAlpha2 << " / " << cosAlpha2 << " / " << weight);
      ATH_MSG_VERBOSE("v0 / v1 = " << v0 << " / " << v1);
      ATH_MSG_VERBOSE("ROTATED CLUSTER COVARIANCE = " << mat(0, 0) << ", " << mat(0, 1) );
      ATH_MSG_VERBOSE("                             " << mat(1, 0) << ", " << mat(1, 1) );

    }
    // updating covariance
    prevCov = std::move(mat);
  }

  Trk::LocalParameters localParameters;
  Amg::MatrixX covariance(prevCov);
  Amg::Vector3D globalPosition(cluster->globalPosition() + distance * globalstripdir);

  // construction of the local parameters to build cluster on track
  if (detectorElement->isBarrel()) {
    Trk::DefinedParameter lpos1dim(cluster->localPosition().x(), Trk::locX);
    localParameters = Trk::LocalParameters(lpos1dim);
    covariance = Amg::MatrixX(1, 1);
    covariance(0, 0) = prevCov(0, 0);
    // scaling errors if required
    if (!m_stripErrorScalingKey.key().empty()) {
      SG::ReadCondHandle<RIO_OnTrackErrorScaling> error_scaling( m_stripErrorScalingKey,ctx );
      covariance = Trk::ErrorScalingCast<SCTRIO_OnTrackErrorScaling>(*error_scaling)
                     ->getScaledCovariance(std::move(covariance), false, 0.0);
    }
  } else {
    localParameters = Trk::LocalParameters(cluster->localPosition());
    // scaling errors if required
    if (!m_stripErrorScalingKey.key().empty()) {
      SG::ReadCondHandle<RIO_OnTrackErrorScaling> error_scaling(m_stripErrorScalingKey,ctx);
      covariance = Trk::ErrorScalingCast<SCTRIO_OnTrackErrorScaling>(*error_scaling)
                     ->getScaledCovariance(std::move(covariance), true,
                                           detectorElement->sinStereoLocal(cluster->localPosition()));
    }

    // For endcap strip clusters, the error matrix needs to be scaled
    // accordingly to the strip pitch (in mm) at the estimated track parameter.
    // The scaling term is evaluated in the strip frame and
    // transformed in the module frame.
    double sinAlpha = detectorElement->sinStereoLocal(cluster->localPosition());
    double sinAlpha2 = sinAlpha * sinAlpha;
    double cosAlpha2 = (1. - sinAlpha) * (1. + sinAlpha);
    double sinAlphaCosAlpha = sinAlpha * std::sqrt(cosAlpha2);
    // Weight factor to express the strip pitch (in mm) from the module center to the
    // estimated track parameter position.
    double radiusAtLocPos = std::hypot(loct.x(), loct.y());
    double phiPitchAtLocPos = phiPitchInRad*radiusAtLocPos;
    double weight = phiPitchAtLocPos / detectorElement->phiPitch();
    // Error matrix scaling term, this is expressend in the module frame.
    // It is evaluated using the same jacobian used above.
    double dV0 = (cosAlpha2 * covariance(0, 0) + sinAlpha2 * covariance(1, 1) +
                  2. * sinAlphaCosAlpha * covariance(1, 0)) * (weight * weight - 1.);

    // The final covariance matrix, which also accounts for the scaling:
    // Scaling and rotation are done with the same transformation:
    //
    // C = J C_{0} J^T
    //
    // where C_{0} is also includes the scaling term dV0:
    //
    // C_{0} = ( var_{0}^2 + dV0     0
    //              0            var_{1}^2  )
    //
    // The scaling terms are then added to the previously evaluated matrix:
    covariance(0, 0) += (cosAlpha2 * dV0);
    covariance(1, 0) += (sinAlphaCosAlpha * dV0);
    covariance(0, 1) = covariance(1, 0);
    covariance(1, 1) += (sinAlpha2 * dV0);

    ATH_MSG_VERBOSE(" SCALING OF ENDCAP STRIP CLUSTER COVARIANCE");
    ATH_MSG_VERBOSE("sinAlpha / sinAlpha2 / cosAlpha2 / weight = " << sinAlpha << " / " << sinAlpha2 << " / " << cosAlpha2 << " / " << weight );
    ATH_MSG_VERBOSE("dV0 = (" << cosAlpha2 * covariance(0, 0) << " + "
                              << sinAlpha2 * covariance(1, 1) << " + "
                              << 2. * sinAlphaCosAlpha * covariance(1, 0)
                              << " ) * " << (weight * weight - 1.) << " = "
                              << dV0);
    ATH_MSG_VERBOSE("SCALED CLUSTER COVARIANCE = " << covariance(0, 0) << ", "
                                                   << covariance(0, 1));
    ATH_MSG_VERBOSE("                            " << covariance(1, 0) << ", " << covariance(1, 1) );
  }
  // final construction of clustr of track
  bool isbroad = m_option_errorStrategy == 0;
  return new InDet::SCT_ClusterOnTrack(cluster, std::move(localParameters), std::move(covariance),
                                       detectorElement->identifyHash(), globalPosition, isbroad);
}
