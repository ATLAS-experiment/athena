/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingVolumeArrayCreator.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk include
#include <cmath>
#include <algorithm>

#include "TrkDetDescrTools/TrackingVolumeArrayCreator.h"
#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/BinnedArray1D.h"
#include "TrkDetDescrUtils/BinnedArray1D1D.h"
#include "TrkDetDescrUtils/BinnedArray2D.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkDetDescrUtils/NavBinnedArray1D.h"
#include "TrkDetDescrUtils/SharedDoNoDelete.h"
#include "TrkVolumes/BevelledCylinderVolumeBounds.h"
#include "TrkVolumes/CuboidVolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/DoubleTrapezoidVolumeBounds.h"
#include "TrkVolumes/TrapezoidVolumeBounds.h"
// Amg
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

namespace {
    using VolumePtr = Trk::TrackingVolumeArrayCreator::VolumePtr;
    inline std::vector<VolumePtr> translateToShared(const std::vector<Trk::TrackingVolume*>& inVols,
                                                    bool no_deletePtr) {
        std::vector<VolumePtr> outVec{};
        outVec.reserve(inVols.size());
        for (Trk::TrackingVolume* vol : inVols) {
            if(!no_deletePtr) {
                outVec.emplace_back(vol);
            } else {
                outVec.emplace_back(vol, Trk::do_not_delete<Trk::TrackingVolume>);
            }
        }
        return outVec;
    }
}


namespace Trk{
// constructor
TrackingVolumeArrayCreator::TrackingVolumeArrayCreator(const std::string& t,
                                                       const std::string& n,
                                                       const IInterface* p)
  : AthAlgTool(t, n, p)
{
  declareInterface<ITrackingVolumeArrayCreator>(this);
}

// destructor
TrackingVolumeArrayCreator::~TrackingVolumeArrayCreator() = default;

TrackingVolumeArray* TrackingVolumeArrayCreator::cylinderVolumesArrayInR(const std::vector<TrackingVolume*>& vols,
                                                                         bool navtype) const {
    return cylinderVolumesArrayInR(translateToShared(vols, navtype), navtype).release();
}
std::unique_ptr<TrackingVolumeArray> TrackingVolumeArrayCreator::cylinderVolumesArrayInR(const std::vector<VolumePtr>& vols,
                                                                                         bool navtype) const {


  ATH_MSG_VERBOSE("Create VolumeArray of "<< vols.size() << " Volumes (with CylinderVolumeBounds) with R-binning. ");

  // check for compatibility - needs r-sorting first
  double lastZmin{0.}, lastZmax{0.}, lastOuterRadius{0.};

  // the vector of doubles for identification
  std::vector<float> boundaries;
  boundaries.reserve(vols.size() + 1);

  // the vector needed for the BinnedArray
  std::vector<TrackingVolumeOrderPosition> volOrder;
  // loop over volumes and fill primaries
  auto volIter = vols.begin();
  for (unsigned int ivol = 0; volIter != vols.end(); ++volIter, ++ivol) {
    const CylinderVolumeBounds* currentCylBounds = nullptr;
    if (*volIter) currentCylBounds = dynamic_cast<const CylinderVolumeBounds*>(&((*volIter)->volumeBounds()));
    if (!currentCylBounds) {
      ATH_MSG_ERROR("Given TrackingVolume doesn't exist or is not of shape "
                    "'CylinderVolumeBounds': return 0");
      return nullptr;
    }
    // current rmin/rmax
    double currentRmin = currentCylBounds->innerRadius();
    double currentRmax = currentCylBounds->outerRadius();
    if (!ivol)
      boundaries.push_back(currentRmin);
    boundaries.push_back(currentRmax);

    // compatibility checks
    double currentZmin = (*volIter)->center().z() - currentCylBounds->halflengthZ();
    double currentZmax = (*volIter)->center().z() + currentCylBounds->halflengthZ();

    // check for compatibility of the new volume - not for navigation type
    if (ivol && !navtype) {
        // longitudinal clinch
        if (std::abs(currentZmin - lastZmin) > 0.1 || std::abs(currentZmax - lastZmax) > 0.1) {
          ATH_MSG_ERROR("Given TrackingVolume(s) do not extend in z to the same point (required) : return 0");
          ATH_MSG_VERBOSE("Information : lastZmin / lastZmin    = "<< lastZmin << " / " << currentZmin);
          ATH_MSG_VERBOSE("              lastZmax / currentZmax = "<< lastZmax << " / " << currentZmax);
          return nullptr;
        }
        // radial clinch
        if (std::abs(currentRmin - lastOuterRadius) > 0.1) {
          ATH_MSG_ERROR("Given TrackingVolume(s) are not wrapping, neither inside-out, nor v.v. : return 0");
          ATH_MSG_VERBOSE("Information : currentRmin / lastOuterRadius = "<< currentRmin << " / " << lastOuterRadius);
          return nullptr;
        }
    }
    // register for next round
    lastZmin = currentZmin;
    lastZmax = currentZmax;
    lastOuterRadius = currentRmax;
    // output
    ATH_MSG_VERBOSE("Adding Volume '" << (*volIter)->volumeName() << "' to Array");

    volOrder.emplace_back((*volIter), currentCylBounds->mediumRadius() * Amg::Vector3D::UnitX());
    // push back the volume order position

  }
  if (!volOrder.empty()) {
    auto volBinUtilR = BinUtility(boundaries, open, binR);
    ATH_MSG_VERBOSE("Return created Array. ");
    return std::make_unique<BinnedArray1D<TrackingVolume>>(volOrder, volBinUtilR);
  }
  ATH_MSG_ERROR("No TrackingVolumes provided to the TrackingVolumeArrayCreator: return 0");
  return nullptr;
}


TrackingVolumeArray* TrackingVolumeArrayCreator::cylinderVolumesArrayInZ(const std::vector<TrackingVolume*>& vols,
                                                                         bool navtype) const {
      return cylinderVolumesArrayInZ(translateToShared(vols, navtype), navtype).release();
  }

std::unique_ptr<TrackingVolumeArray>
  TrackingVolumeArrayCreator::cylinderVolumesArrayInZ(const std::vector<VolumePtr>& vols,
                                                      bool navtype) const {
  ATH_MSG_VERBOSE("Create VolumeArray of "
                  << vols.size()
                  << " Volumes (with CylinderVolumeBounds) with Z-binning. ");

  // for compatibility checks
  double lastRmin{0.}, lastRmax{0.}, lastZmax{0.};

  // the vector of doubles for identification
  std::vector<float> boundaries;
  boundaries.reserve(vols.size() + 1);

  // the vector needed for the BinnedArray
  std::vector<TrackingVolumeOrderPosition> volOrder;
  // loop over volumes and fill primaries
  auto volIter = vols.begin();
  for (unsigned int ivol = 0; volIter != vols.end(); ++volIter, ++ivol) {
    const CylinderVolumeBounds* currentCylBounds = nullptr;
    if (*volIter)currentCylBounds = dynamic_cast<const CylinderVolumeBounds*>( &((*volIter)->volumeBounds()));
    if (!currentCylBounds) {
      ATH_MSG_ERROR("Given TrackingVolume doesn't exist or is not of shape 'CylinderVolumeBounds': return 0");
      return nullptr;
    }
    //
    const Amg::Vector3D& volCenter = (*volIter)->center();
    double halflengthZ = currentCylBounds->halflengthZ();
    // get the numbers
    double currentZmin = volCenter.z() - halflengthZ;
    double currentZmax = volCenter.z() + halflengthZ;
    if (!ivol) boundaries.push_back(currentZmin);
    boundaries.push_back(currentZmax);

    // consistency checks
    double currentRmin = currentCylBounds->innerRadius();
    double currentRmax = currentCylBounds->outerRadius();

    // compatibility check - not for navtype
    if (ivol && !navtype) {
      // first the radial check
      if (std::abs(lastRmin - currentRmin) > 0.1 || std::abs(lastRmax - currentRmax) > 0.1) {
        ATH_MSG_ERROR("Given TrackingVolume(s) do not have same radial extends (required): return 0");
        ATH_MSG_VERBOSE("Information : lastRmin / currentRmin = " << lastRmin << " / " << currentRmin);
        ATH_MSG_VERBOSE("              lastRmax / currentRmax = " << lastRmax << " / " << currentRmax);
        return nullptr;
      }
      // then let's see whether they leave gaps in z
      if (std::abs(lastZmax - currentZmin) > 0.1) {
          ATH_MSG_ERROR("Given TrackingVolume(s) are not attaching in z (required) : return 0");
          return nullptr;
      }
    }
    // for the next round
    lastRmin = currentRmin;
    lastRmax = currentRmax;
    lastZmax = currentZmax;
    // output
    ATH_MSG_VERBOSE("Adding Volume '" << (*volIter)->volumeName()
                                      << "' to Array");
    // push back the volume order position
    volOrder.emplace_back((*volIter), (*volIter)->center());

  }
  if (!volOrder.empty()) {
    auto volBinUtil = BinUtility(boundaries, open, binZ);
    ATH_MSG_VERBOSE("Return created Array. ");
    return std::make_unique<BinnedArray1D<TrackingVolume>>(volOrder, volBinUtil);
  }
  ATH_MSG_ERROR("No TrackingVolumes provided to the TrackingVolumeArrayCreator: return 0");
  return nullptr;
}

TrackingVolumeArray* TrackingVolumeArrayCreator::cylinderVolumesArrayInPhi(const std::vector<TrackingVolume*>& vols,
                                                                           bool navtype) const {
    return cylinderVolumesArrayInPhi(translateToShared(vols,navtype), navtype).release();
}

std::unique_ptr<TrackingVolumeArray>
  TrackingVolumeArrayCreator::cylinderVolumesArrayInPhi(const std::vector<VolumePtr>& vols,
                                                        bool /*navtype*/) const {

  ATH_MSG_VERBOSE("Create VolumeArray of " << vols.size()
               << " Volumes (with CylinderVolumeBounds) with Phi-binning. ");

  // phi binning; assume equidistant
  int nPhiBins = !vols.empty() ? vols.size() : 1;
  double phi = M_PI;
  // the vector needed for the BinnedArray
  std::vector<TrackingVolumeOrderPosition> volOrder;
  // loop over volumes and fill primaries
  auto volIter = vols.begin();
  for (; volIter != vols.end(); ++volIter) {
    const CylinderVolumeBounds* cyl = nullptr;
    if (*volIter) cyl = dynamic_cast<const CylinderVolumeBounds*>(&((*volIter)->volumeBounds()));
    if (!cyl) {
       ATH_MSG_ERROR("Given TrackingVolume doesn't exist or is not of shape 'CylinderVolumeBounds': return 0");
       return nullptr;
    }
    // output
    ATH_MSG_VERBOSE("Adding Volume '" << (*volIter)->volumeName()<< "' to Array");
    // push back the volume order position
     volOrder.emplace_back((*volIter), (*volIter)->transform() * (cyl->mediumRadius() * Amg::Vector3D::UnitX()));

  }
  if (!volOrder.empty()) {
    auto volBinUtil = BinUtility(nPhiBins, -phi, +phi, closed, binPhi);
    return std::make_unique<BinnedArray1D<TrackingVolume>>(volOrder, volBinUtil);
  }
  ATH_MSG_ERROR("No TrackingVolumes provided to the TrackingVolumeArrayCreator: return 0");
  return nullptr;
}

TrackingVolumeArray*
  TrackingVolumeArrayCreator::cylinderVolumesArrayInPhiR(const std::vector<TrackingVolume*>& vols,
                                                         bool navtype) const {
    return cylinderVolumesArrayInPhiR(translateToShared(vols, navtype), navtype).release();
}
std::unique_ptr<TrackingVolumeArray>
TrackingVolumeArrayCreator::cylinderVolumesArrayInPhiR(const std::vector<VolumePtr>& vols,
                                                       bool navtype) const {
  if (vols.empty())
    return nullptr;

  const bool bevelled = std::find_if(vols.begin(),vols.end(),
                              [](const VolumePtr& ptr) -> bool {
                                  return dynamic_cast<const BevelledCylinderVolumeBounds*>(&(ptr->volumeBounds()));
                              }) != vols.end();

  double tol = 0.001;

  // the vector needed for the BinnedArray
  std::vector<TrackingVolumeOrderPosition> volOrder;

  if (bevelled) {
    ATH_MSG_VERBOSE("Create 2dim VolumeArray of "<< vols.size()
                    << " Volumes (with CylinderVolumeBounds) with PhiH-binning. ");
    std::vector<float> phiSteps;
    std::vector<std::pair<std::pair<double, int>, std::pair<double, double>>>
      volPos;
    std::vector<VolumePtr> fullPhiVols;

    for (const VolumePtr& vol : vols) {
      const auto *cyl = dynamic_cast<const CylinderVolumeBounds*>(&(vol->volumeBounds()));
      const auto *bcyl =dynamic_cast<const BevelledCylinderVolumeBounds*>(&(vol->volumeBounds()));
      double rmin{0.}, rmax{0.}, dphi{0.}, mRad{0.};
      int type = 0;

      if (cyl) {
        rmin = cyl->innerRadius();
        rmax = cyl->outerRadius();
        dphi = cyl->halfPhiSector();
        mRad = cyl->mediumRadius();
      } else if (bcyl) {
        rmin = bcyl->innerRadius();
        rmax = bcyl->outerRadius();
        dphi = bcyl->halfPhiSector();
        mRad = bcyl->mediumRadius();
        type = bcyl->type();
      } else {
         ATH_MSG_ERROR("volume not cylinder nor bevelled cylinder ");
         return nullptr;
      }

      if (dphi < M_PI) {
        // push back the volume order position
        Amg::Vector3D ngp((vol->transform()) * (mRad * Amg::Vector3D::UnitX()));
        volOrder.emplace_back(vol, ngp);

        // push back volume position to avoid another loop
        volPos.emplace_back(std::pair<double, int>(ngp.phi(), type),
                            std::pair<double, double>(rmin, rmax));
        // phi binning
        double phi1 = ngp.phi() - dphi;
        double phi2 = ngp.phi() + dphi;
        if (phi1 < -2 * M_PI) {
          phi1 += 2 * M_PI;
        } if (phi2 < -2 * M_PI) {
          phi2 += 2 * M_PI;
        } if (phi1 > 2 * M_PI) {
          phi1 -= 2 * M_PI;
        } if (phi2 > 2 * M_PI) {
          phi2 -= 2 * M_PI;
        }

        if (!phiSteps.empty()) {
          std::vector<float>::iterator iter = phiSteps.begin();
          bool known = false;
          while (iter != phiSteps.end()) {
            if (std::abs(phi1 - (*iter)) < tol) {
              known = true;
              break;
            }
            if (phi1 < (*iter)) {
              phiSteps.insert(iter, phi1);
              known = true;
              break;
            }
            ++iter;
          }
          if (!known)
            phiSteps.push_back(phi1);
          iter = phiSteps.begin();
          known = false;
          while (iter != phiSteps.end()) {
            if (std::abs(phi2 - (*iter)) < tol) {
              known = true;
              break;
            }
            if (phi2 < (*iter)) {
              phiSteps.insert(iter, phi2);
              known = true;
              break;
            }
            ++iter;
          }
          if (!known)
            phiSteps.push_back(phi2);
        } else {
          phiSteps.push_back(fmin(phi1, phi2));
          phiSteps.push_back(fmax(phi1, phi2));
        }
      } else {
        fullPhiVols.push_back(vol);
      }
    } // end of first loop over volumes
    // collect volumes with full phi range
    if (phiSteps.empty()) {
      phiSteps.push_back(-M_PI);
      phiSteps.push_back(+M_PI);
    }
    for (auto & fullPhiVol : fullPhiVols) {
      const auto *cyl =dynamic_cast<const CylinderVolumeBounds*>(&(fullPhiVol->volumeBounds()));
      if (!cyl) {
        ATH_MSG_WARNING("dynamic_cast<const CylinderVolumeBounds*>  failed ... trying to continue loop");
        continue;
      }
      double rmin = cyl->innerRadius();
      double rmax = cyl->outerRadius();

      for (unsigned int iphi = 0; iphi < phiSteps.size(); ++iphi) {
        // reference position
        double phiRef = 0.5 * phiSteps[iphi];
        if (iphi < phiSteps.size() - 1)
          phiRef += 0.5 * phiSteps[iphi + 1];
        else
          phiRef += 0.5 * phiSteps[0] + M_PI;
        // setting the position in the phi sector
        const Amg::Vector3D ngp{cyl->mediumRadius() * std::cos(phiRef),cyl->mediumRadius() * std::sin(phiRef),0.};

        volOrder.emplace_back(fullPhiVol, ngp);

        // push back volume position to avoid another loop
        volPos.emplace_back(std::pair<double, int>(ngp.phi(), 0),
                            std::pair<double, double>(rmin, rmax));
      }
    }
    // all volumes in arrays : build bin utilities

    // adjust phiSteps : upper bound equal the lower
    if (phiSteps.size() > 1) {
      if (phiSteps.back() > M_PI)
        phiSteps.erase(phiSteps.end() - 1);
      else
        phiSteps.erase(phiSteps.begin());
    }

    // phi binning
    std::vector<std::vector<std::pair<int, float>>> hSteps(phiSteps.size());
    std::vector<float> phiRef(phiSteps.size());
    for (unsigned int ip = 0; ip < phiSteps.size() - 1; ++ip)
      phiRef[ip] = 0.5 * (phiSteps[ip] + phiSteps[ip + 1]);
    phiRef.back() = 0.5 * (phiSteps.back() + phiSteps.front());
    phiRef.back() += (phiRef.back() > 0) ? -M_PI : M_PI;

    auto phiBinUtil = BinUtility(phiSteps, closed, binPhi);

    // H binning

    for (unsigned int i = 0; i < volPos.size(); ++i) {

      // double phi = volPos[i].first.first;
      int type = volPos[i].first.second;
      double rmin = volPos[i].second.first;
      double rmax = volPos[i].second.second;
      int tmin = (type != 1 && type != 3) ? 0 : 1;
      int tmax = (type < 2) ? 0 : 1;

      int phibin = phiBinUtil.bin(volOrder[i].second);

      if (!hSteps[phibin].empty()) {
        std::vector<std::pair<int, float>>::iterator iter =
          hSteps[phibin].begin();
        bool known = false;
        while (iter != hSteps[phibin].end()) {
          if (std::abs(rmin - (*iter).second) < tol) {
            known = true;
            break;
          }
          if (rmin < (*iter).second) {
            hSteps[phibin].insert(iter, std::pair<int, float>(tmin, rmin));
            known = true;
            break;
          }
          ++iter;
        }
        if (!known)
          hSteps[phibin].emplace_back(tmin, rmin);
        iter = hSteps[phibin].begin();
        known = false;
        while (iter != hSteps[phibin].end()) {
          if (std::abs(rmax - (*iter).second) < tol) {
            known = true;
            break;
          }
          if (rmax < (*iter).second) {
            hSteps[phibin].insert(iter, std::pair<int, float>(tmax, rmax));
            known = true;
            break;
          }
          ++iter;
        }
        if (!known)
          hSteps[phibin].emplace_back(tmax, rmax);
      } else {
        hSteps[phibin].emplace_back(tmin, rmin);
        hSteps[phibin].emplace_back(tmax, rmax);
      }
    }
    // verify size of the array
    // 2dim array
    // steering bin utility in phi
    auto hUtil = std::vector<BinUtility>(phiSteps.size());

    for (unsigned int ih = 0; ih < phiSteps.size(); ++ih) {
      (hUtil)[ih] = BinUtility(phiRef[ih], hSteps[ih]);
    }

    return std::make_unique<BinnedArray1D1D<TrackingVolume>>(volOrder, phiBinUtil, hUtil);
  }

  ATH_MSG_VERBOSE("Create 2dim VolumeArray of of "<< vols.size()
                << " Volumes (with CylinderVolumeBounds) with PhiR-binning. ");

  std::vector<float> rSteps;
  double phiSector = M_PI;
  std::vector<std::pair<double, std::pair<double, double>>> volPos;

  for (const auto& vol : vols) {
   const auto *cyl =dynamic_cast<const CylinderVolumeBounds*>(&(vol->volumeBounds()));
    if (!cyl) {
       ATH_MSG_WARNING("dynamic_cast<const CylinderVolumeBounds*> failed ... trying to continue loop");
       continue;
    }
    double rmin = cyl->innerRadius();
    double rmax = cyl->outerRadius();
    double dphi = cyl->halfPhiSector();
    if (phiSector > 0. && std::abs(dphi - phiSector) > 0.001)
      phiSector = phiSector < M_PI ? -1. : dphi;


    const Amg::Vector3D ngp{vol->transform() * (cyl->mediumRadius()* Amg::Vector3D::UnitX())};
    volOrder.emplace_back(vol, ngp);

    // push back volume position to avoid another loop
    volPos.emplace_back(cyl->mediumRadius(), std::make_pair(ngp.phi(), dphi));
    // r binning
    if (!rSteps.empty()) {
      std::vector<float>::iterator iter = rSteps.begin();
      bool known = false;
      while (iter != rSteps.end()) {
        if (std::abs(rmin - (*iter)) < tol) {
          known = true;
          break;
        }
        if (rmin < (*iter)) {
          rSteps.insert(iter, rmin);
          known = true;
          break;
        }
        ++iter;
      }
      if (!known)
        rSteps.push_back(rmin);
      iter = rSteps.begin();
      known = false;
      while (iter != rSteps.end()) {
        if (std::abs(rmax - (*iter)) < tol) {
          known = true;
          break;
        }
        if (rmax < (*iter)) {
          rSteps.insert(iter, rmax);
          known = true;
          break;
        }
        ++iter;
      }
      if (!known)
        rSteps.push_back(rmax);
    } else {
      rSteps.push_back(rmin);
      rSteps.push_back(rmax);
    }
  }

  if (phiSector > 0.) { // overall equidistant binning

    const int rStepsSizem1 = rSteps.size() - 1;
    std::vector<double> phi(rStepsSizem1, M_PI);
    std::vector<int> phiSect(rStepsSizem1,int(M_PI / phiSector));

    // simplify if possible
    if (rSteps.size() == 1) {
      return cylinderVolumesArrayInPhi(vols, navtype);
    }
    if (phiSector == M_PI) {
      return cylinderVolumesArrayInR(vols, navtype);
    }
    // 2dim array
    auto rBinUtil = BinUtility(rSteps, open, binR);
    auto phiUtil = std::vector<BinUtility>(rSteps.size() - 1);
    for (unsigned int ip = 0; ip < phiUtil.size(); ++ip) {
      (phiUtil)[ip] =BinUtility(phiSect[ip], closed, binPhi);
    }
    return std::make_unique<BinnedArray1D1D<TrackingVolume>>(volOrder, rBinUtil, phiUtil);
  }

  // R binning : steering binUtility
  auto binGenR = BinUtility(rSteps, open, binR);

  // phi binning
  std::vector<std::vector<float>> phiSteps(rSteps.size() - 1);

  for (unsigned int i = 0; i < volPos.size(); ++i) {

    double phi = volPos[i].second.first;
    double dphi = volPos[i].second.second;

    int binr = binGenR.bin(volOrder[i].second);

    float phi1 = phi - dphi;
    float phi2 = phi + dphi;
    if (phi1 < 0)
      phi1 += 2 * M_PI;
    if (phi2 < 0)
      phi2 += 2 * M_PI;

    if (!phiSteps[binr].empty()) {
      std::vector<float>::iterator iter = phiSteps[binr].begin();
      bool known = false;
      while (iter != phiSteps[binr].end()) {
        if (std::abs(phi1 - (*iter)) < tol) {
          known = true;
          break;
        }
        if (phi1 < (*iter)) {
          phiSteps[binr].insert(iter, phi1);
          known = true;
          break;
        }
        ++iter;
      }
      if (!known)
        phiSteps[binr].push_back(phi1);
      iter = phiSteps[binr].begin();
      known = false;
      while (iter != phiSteps[binr].end()) {
        if (std::abs(phi2 - (*iter)) < tol) {
          known = true;
          break;
        }
        if (phi2 < (*iter)) {
          phiSteps[binr].insert(iter, phi2);
          known = true;
          break;
        }
        ++iter;
      }
      if (!known)
        phiSteps[binr].push_back(phi2);
    } else {
      phiSteps[binr].push_back(std::fmin(phi1, phi2));
      phiSteps[binr].push_back(std::fmax(phi1, phi2));
    }
  }

  // 2dim array
  auto phiUtil = std::vector<BinUtility>(phiSteps.size());

  for (unsigned int ip = 0; ip < phiSteps.size(); ++ip) {
    (phiUtil)[ip] = BinUtility(phiSteps[ip], closed, binPhi);
  }

  return std::make_unique<BinnedArray1D1D<TrackingVolume>>(volOrder, binGenR, phiUtil);
}

TrackingVolumeArray*
TrackingVolumeArrayCreator::cylinderVolumesArrayInPhiZ(const std::vector<TrackingVolume*>& vols,
                                                       bool navtype) const {
    return cylinderVolumesArrayInPhiZ(translateToShared(vols, navtype), navtype).release();
}
std::unique_ptr<TrackingVolumeArray>
TrackingVolumeArrayCreator::cylinderVolumesArrayInPhiZ(const std::vector<VolumePtr>& vols,
                                                       bool navtype) const {

  ATH_MSG_VERBOSE("Create 2dim VolumeArray of of "<< vols.size()
              << " Volumes (with CylinderVolumeBounds) with PhiZ-binning. ");

  double tol = 0.001;
  // the vector needed for the BinnedArray
  std::vector<TrackingVolumeOrderPosition> volOrder;

  std::vector<float> zSteps;

  double phiSector = M_PI;
  std::vector<std::pair<float, std::pair<float, float>>> volPos;

  for (const VolumePtr& vol : vols) {
    const auto *cyl = dynamic_cast<const CylinderVolumeBounds*>(&(vol->volumeBounds()));
    const auto *bcyl = dynamic_cast<const BevelledCylinderVolumeBounds*>(&(vol->volumeBounds()));
    double zmin{0.}, zmax{0.}, dphi{0.}, mRad{0.};
    if (cyl) {
      zmin = vol->center().z() - cyl->halflengthZ();
      zmax = vol->center().z() + cyl->halflengthZ();
      dphi = cyl->halfPhiSector();
      mRad = cyl->mediumRadius();
    } else if (bcyl) {
      zmin = vol->center().z() - bcyl->halflengthZ();
      zmax = vol->center().z() + bcyl->halflengthZ();
      dphi = bcyl->halfPhiSector();
      mRad = bcyl->mediumRadius();
    } else {
      ATH_MSG_ERROR("volume not cylinder nor bevelled cylinder ");
      return nullptr;
    }

    if (phiSector > 0. && std::abs(dphi - phiSector) > 0.001)
      phiSector = phiSector < M_PI ? -1. : dphi;

    // push back the volume order position
    const Amg::Vector3D ngp{vol->transform() * (mRad * Amg::Vector3D::UnitX())};

    volOrder.emplace_back(vol, ngp);
    // push back volume position to avoid another loop
    volPos.emplace_back(vol->center().z(), std::make_pair(ngp.phi(), dphi));
    // z binning
    if (!zSteps.empty()) {
      std::vector<float>::iterator iter = zSteps.begin();
      bool known = false;
      while (iter != zSteps.end()) {
        if (std::abs(zmin - (*iter)) < tol) {
          known = true;
          break;
        }
        if (zmin < (*iter)) {
          zSteps.insert(iter, zmin);
          known = true;
          break;
        }
        ++iter;
      }
      if (!known)
        zSteps.push_back(zmin);
      iter = zSteps.begin();
      known = false;
      while (iter != zSteps.end()) {
        if (std::abs(zmax - (*iter)) < tol) {
          known = true;
          break;
        }
        if (zmax < (*iter)) {
          zSteps.insert(iter, zmax);
          known = true;
          break;
        }
        ++iter;
      }
      if (!known)
        zSteps.push_back(zmax);
    } else {
      zSteps.push_back(zmin);
      zSteps.push_back(zmax);
    }
  }

  if (phiSector > 0.) { // overall equidistant binning

    const int zStepsSizem1 = zSteps.size() - 1;
    std::vector<double> phi(zStepsSizem1, M_PI);
    std::vector<int> phiSect(zStepsSizem1,int(M_PI / phiSector));

    // simplify if possible
    if (phiSector == M_PI) {
      return cylinderVolumesArrayInZ(vols, navtype);
    }
    if (zSteps.size() == 2) {
      return cylinderVolumesArrayInPhi(vols, navtype);
    }
    // 2dim array
    auto binGenZPhi = BinUtility(zSteps, open, binZ);
    binGenZPhi += BinUtility(phiSector, -M_PI, M_PI, closed, binPhi);
    return std::make_unique<BinnedArray2D<TrackingVolume>>(volOrder, binGenZPhi);
  }

  // steering binUtility in binZ
  auto binGenZ = BinUtility(zSteps, open, binZ);

  // phi binning  - steering binUtility in binZ
  std::vector<std::vector<float>> phiSteps(zSteps.size() - 1);

  for (unsigned int i = 0; i < volPos.size(); ++i) {

    float phi = volPos[i].second.first;
    float dphi = volPos[i].second.second;

    int binZ = binGenZ.bin(volOrder[i].second);

    float phi1 = phi - dphi;
    float phi2 = phi + dphi;
    if (phi1 < 0)
      phi1 += 2 * M_PI;
    if (phi2 < 0)
      phi2 += 2 * M_PI;

    if (!phiSteps[binZ].empty()) {
      std::vector<float>::iterator iter = phiSteps[binZ].begin();
      bool known = false;
      while (iter != phiSteps[binZ].end()) {
        if (std::abs(phi1 - (*iter)) < tol) {
          known = true;
          break;
        }
        if (phi1 < (*iter)) {
          phiSteps[binZ].insert(iter, phi1);
          known = true;
          break;
        }
        ++iter;
      }
      if (!known)
        phiSteps[binZ].push_back(phi1);
      iter = phiSteps[binZ].begin();
      known = false;
      while (iter != phiSteps[binZ].end()) {
        if (std::abs(phi2 - (*iter)) < tol) {
          known = true;
          break;
        }
        if (phi2 < (*iter)) {
          phiSteps[binZ].insert(iter, phi2);
          known = true;
          break;
        }
        ++iter;
      }
      if (!known)
        phiSteps[binZ].push_back(phi2);
    } else {
      phiSteps[binZ].push_back(std::fmin(phi1, phi2));
      phiSteps[binZ].push_back(std::fmax(phi1, phi2));
      //	phiSectors[binZ] = dphi ;
    }
  }

  // 2dim array: construct from two 1D boundaries
  auto phiUtil = std::vector<BinUtility>(phiSteps.size());

  for (unsigned int ip = 0; ip < phiSteps.size(); ++ip) {
    phiUtil[ip] = BinUtility(phiSteps[ip], closed, binPhi);
  }

  return std::make_unique<BinnedArray1D1D<TrackingVolume>>(volOrder, binGenZ, phiUtil);
}


TrackingVolumeArray*
TrackingVolumeArrayCreator::cuboidVolumesArrayNav(const std::vector<TrackingVolume*>& vols,
                                                  BinUtility* binUtil,
                                                  bool navtype) const {
    return cuboidVolumesArrayNav(translateToShared(vols, navtype), binUtil).release();
}
std::unique_ptr<TrackingVolumeArray>
    TrackingVolumeArrayCreator::cuboidVolumesArrayNav(const std::vector<VolumePtr>& vols,
                                                      BinUtility* binUtil) const {
  // the vector needed for the BinnedArray
  std::vector<VolumePtr> volOrder;
  // loop over volumes and fill primaries
  auto volIter = vols.begin();
  for (; volIter != vols.end(); ++volIter) {
    const auto *currentCubBounds = dynamic_cast<const CuboidVolumeBounds*>(&((*volIter)->volumeBounds()));
    if (!currentCubBounds) {
      ATH_MSG_ERROR("Given TrackingVolume to TrackingVolumeArrayCreator didn't "
                    "match specified shape: return 0");
      return nullptr;
    }
     volOrder.push_back(*volIter);
  }
  if (!volOrder.empty()) {
    auto navTransform = std::make_unique<Amg::Transform3D>(Amg::Transform3D::Identity());
    return std::make_unique<NavBinnedArray1D<TrackingVolume>>(volOrder, binUtil, navTransform.release());
  }
  ATH_MSG_ERROR("No TrackingVolumes provided to the TrackingVolumeArrayCreator: return 0");
  return nullptr;
}


TrackingVolumeArray*
TrackingVolumeArrayCreator::trapezoidVolumesArrayNav(const std::vector<TrackingVolume*>& vols,
                                                     BinUtility* binUtil, bool navtype) const {
  return trapezoidVolumesArrayNav(translateToShared(vols, navtype), binUtil).release();
}

std::unique_ptr<TrackingVolumeArray>
  TrackingVolumeArrayCreator::trapezoidVolumesArrayNav(const std::vector<VolumePtr>& vols, BinUtility* binUtil) const {
  // the vector needed for the BinnedArray
  std::vector<VolumePtr> volOrder;
  // loop over volumes and fill primaries
  auto volIter = vols.begin();
  for (; volIter != vols.end(); ++volIter) {
    const auto *currentTrdBounds = dynamic_cast<const TrapezoidVolumeBounds*>(&((*volIter)->volumeBounds()));
    if (!currentTrdBounds) {
      ATH_MSG_ERROR("Given TrackingVolume to TrackingVolumeArrayCreator didn't "
                    "match specified shape: return 0");
      return nullptr;
    }
    // push back the volume order position
    volOrder.push_back(*volIter);
  }
  if (!volOrder.empty()) {
    Amg::Transform3D* navTransform = new Amg::Transform3D(Amg::Transform3D::Identity());
    return std::make_unique<NavBinnedArray1D<TrackingVolume>>(volOrder, binUtil, navTransform);
  }
  ATH_MSG_ERROR("No TrackingVolumes provided to the TrackingVolumeArrayCreator: return 0");
  return nullptr;
}

TrackingVolumeArray* TrackingVolumeArrayCreator::doubleTrapezoidVolumesArrayNav(const std::vector<TrackingVolume*>& vols,
                                                                                BinUtility* binUtil,
                                                                                bool navtype) const {
    return doubleTrapezoidVolumesArrayNav(translateToShared(vols, navtype), binUtil).release();
}

std::unique_ptr<TrackingVolumeArray>
  TrackingVolumeArrayCreator::doubleTrapezoidVolumesArrayNav(const std::vector<VolumePtr>& vols, BinUtility* binUtil) const {
  // the vector needed for the BinnedArray
  std::vector<VolumePtr> volOrder;
  // loop over volumes and fill primaries
  auto volIter = vols.begin();
  for (; volIter != vols.end(); ++volIter) {
    const auto *currentDTrdBounds =dynamic_cast<const DoubleTrapezoidVolumeBounds*>(&((*volIter)->volumeBounds()));
    if (!currentDTrdBounds) {
      ATH_MSG_ERROR("Given TrackingVolume to TrackingVolumeArrayCreator didn't "
                    "match specified shape: return 0");
      return nullptr;
    }
    // push back the volume order position
    volOrder.push_back(*volIter);
  }
  if (!volOrder.empty()) {
    Amg::Transform3D* navTransform = new Amg::Transform3D(Amg::Transform3D::Identity());
    return std::make_unique<NavBinnedArray1D<TrackingVolume>>(volOrder, binUtil, navTransform);
  }
  ATH_MSG_ERROR(
    "No TrackingVolumes provided to the TrackingVolumeArrayCreator: return 0");
  return nullptr;
}

}
