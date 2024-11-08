/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackStatePrinterTool.h"

// Athena
#include "TrkParameters/TrackParameters.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/AnnulusBounds.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/EventData/TransformationHelpers.hpp"

// PACKAGE
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsInterop/Logger.h"

#include "ActsGeometry/SurfaceOfMeasurementUtil.h"

// Other
#include <vector>
#include <iostream>
#include <sstream>

namespace ActsTrk
{
  /// =========================================================================
  /// file-local static functions and static TrackStatePrinter member functions.
  /// TrackStatePrinter class method definitions comes later.
  /// =========================================================================

  std::string TrackStatePrinterTool::trackStateName(Acts::ConstTrackStateType trackStateType)
  {
    static constexpr std::array<std::tuple<bool, Acts::TrackStateFlag, char>, 6> trackStateNames{{
        {false, Acts::TrackStateFlag::ParameterFlag, '-'},
        {true, Acts::TrackStateFlag::MeasurementFlag, 'M'},
        {true, Acts::TrackStateFlag::OutlierFlag, 'O'},
        {true, Acts::TrackStateFlag::HoleFlag, 'H'},
        {true, Acts::TrackStateFlag::MaterialFlag, 'm'},
        {true, Acts::TrackStateFlag::SharedHitFlag, 'S'},
    }};
    std::string s;
    for (const auto &[b, f, c] : trackStateNames)
    {
      if (trackStateType.test(f) == b)
        s += c;
    }
    return s;
  }

  // compact surface/boundary name
  std::string TrackStatePrinterTool::actsSurfaceName(const Acts::Surface &surface)
  {
    std::string name = surface.name();
    if (name.compare(0, 6, "Acts::") == 0)
    {
      name.erase(0, 6);
    }
    if (name.size() > 7 && name.compare(name.size() - 7, 7, "Surface") == 0)
    {
      name.erase(name.size() - 7, 7);
    }
    static const std::map<Acts::SurfaceBounds::BoundsType, const char *> boundsNames{{
        {Acts::SurfaceBounds::BoundsType::eCone, "Cone"},
        {Acts::SurfaceBounds::BoundsType::eCylinder, "Cylinder"},
        {Acts::SurfaceBounds::BoundsType::eDiamond, "Diamond"},
        {Acts::SurfaceBounds::BoundsType::eDisc, "Disc"},
        {Acts::SurfaceBounds::BoundsType::eEllipse, "Ellipse"},
        {Acts::SurfaceBounds::BoundsType::eLine, "Line"},
        {Acts::SurfaceBounds::BoundsType::eRectangle, "Rectangle"},
        {Acts::SurfaceBounds::BoundsType::eTrapezoid, "Trapezoid"},
        {Acts::SurfaceBounds::BoundsType::eTriangle, "Triangle"},
        {Acts::SurfaceBounds::BoundsType::eDiscTrapezoid, "DiscTrapezoid"},
        {Acts::SurfaceBounds::BoundsType::eConvexPolygon, "ConvexPolygon"},
        {Acts::SurfaceBounds::BoundsType::eAnnulus, "Annulus"},
        {Acts::SurfaceBounds::BoundsType::eBoundless, "Boundless"},
        {Acts::SurfaceBounds::BoundsType::eOther, "Other"},
    }};
    if (auto it = boundsNames.find(surface.bounds().type());
        it != boundsNames.end() && it->second != name)
    {
      name += ' ';
      name += it->second;
    }
    return name;
  }

  static std::string
  atlasSurfaceName(const Acts::Surface *measurement_surface)
  {
     if (measurement_surface) {
        const ActsDetectorElement *
           acts_detector_element = dynamic_cast<const ActsDetectorElement *>(measurement_surface->associatedDetectorElement());
        if (acts_detector_element) {
           const InDetDD::SiDetectorElement *detElem = dynamic_cast< const InDetDD::SiDetectorElement *>(acts_detector_element->upstreamDetectorElement());
           if (detElem) {
              if (auto idHelper = detElem->getIdHelper())
                 {
                    auto name = idHelper->show_to_string(detElem->identify());
                    if (name.size() >= 2 && name[0] == '[' && name[name.size() - 1] == ']')
                       {
                          return name.substr(1, name.size() - 2);
                       }
                    else
                       {
                          return name;
                       }
                 }
           }
        }
     }
    return {};
  }

  static void printHeader(int type, bool extra = false)
  {
    std::cout << std::left
              << std::setw(5) << "Index" << ' '
              << std::setw(4) << "Type" << ' '
              << std::setw(21) << "SurfaceBounds" << ' ';
    if (type == 0)
    {
      std::cout << std::setw(22) << "GeometryId" << ' '
                << std::setw(20) << "ATLAS ID" << ' '
                << std::right
                << std::setw(10) << "loc0" << ' '
                << std::setw(10) << "loc1"
                << "  "
                << std::setw(9) << "R" << ' '
                << std::setw(9) << "phid" << ' '
                << std::setw(9) << "eta";
      if (extra)
      {
        std::cout << ' '
                  << std::setw(10) << "Trk loc0" << ' '
                  << std::setw(10) << "loc1"
                  << "  "
                  << std::setw(9) << "Trk R" << ' '
                  << std::setw(9) << "phid" << ' '
                  << std::setw(9) << "eta" << ' '
                  << std::setw(10) << "g2l loc0" << ' '
                  << std::setw(10) << "loc1";
      }
      std::cout << '\n';
      static std::atomic<int> kilroy = 0;
      if (!(kilroy++))
      {
        std::cout << "R (mm) and phi (degrees). Estimated local coordinate indicated by \"*\" (from SP), \"o\" (from module center), or \"#\" (globalToLocal(center) failure).";
        if (extra)
          std::cout << " Athena/ACTS comparison only shown if different.";
        std::cout << '\n';
      }
    }
    if (type == 1)
    {
      std::cout << std::setw(22) << "GeometryId/meas/stats" << ' '
                << std::right
                << std::setw(10) << "loc0" << ' '
                << std::setw(10) << "loc1" << ' '
                << std::setw(9) << "Pos R" << ' '
                << std::setw(9) << "phid" << ' '
                << std::setw(9) << "eta" << ' '
                << std::setw(9) << "q*pT" << ' '
                << std::setw(9) << "phid" << ' '
                << std::setw(9) << "eta" << ' '
                << std::setw(6) << "TrkLen" << ' '
                << std::setw(7) << "chi2" << ' '
                << std::setw(6) << "Flags" << '\n';
    }
  }

  static void
  printVec3(const Acts::Vector3 &p)
  {
    std::cout << std::fixed << ' '
              << std::setw(9) << std::setprecision(3) << p.head<2>().norm() << ' '
              << std::setw(9) << std::setprecision(3) << std::atan2(p[1], p[0]) / Acts::UnitConstants::degree << ' '
              << std::setw(9) << std::setprecision(5) << std::atanh(p[2] / p.norm())
              << std::defaultfloat << std::setprecision(-1);
  }

  static void
  printVec3(const Acts::Vector3 &p, const Acts::Vector3 &cmp, int precision = 3)
  {
    if (((p - cmp).array().abs() >= 0.5 * std::pow(10.0, -precision)).any())
    {
      printVec3(p);
    }
    else
    {
      std::cout << std::setw(30) << "";
    }
  }

  static void
  printVec2(const Acts::Vector2 &p, const char *estimated = nullptr)
  {
    const char e0 = estimated ? estimated[0] : ' ';
    const char *e1 = estimated ? estimated + 1 : "";
    std::cout << std::fixed << ' '
              << std::setw(10) << std::setprecision(4) << p[0] << e0
              << std::setw(10) << std::setprecision(4) << p[1] << e1
              << std::defaultfloat << std::setprecision(-1);
  }

  static void
  printVec2(const Acts::Vector2 &p, const Acts::Vector2 &cmp, const char *estimated = nullptr, int precision = 4)
  {
    if (((p - cmp).array().abs() >= 0.5 * std::pow(10.0, -precision)).any())
    {
      printVec2(p, estimated);
    }
    else
    {
      std::cout << std::setw(22 + (estimated ? 1 : 0)) << "";
    }
  }

  static void
  printMeasurement(const Acts::GeometryContext &tgContext,
                   const Acts::Surface *surface,
                   const std::tuple<Acts::Vector2, Amg::Vector2D, int, int> &locData,
                   bool compareMeasurementTransforms = false)
  {
    auto &[loc, locTrk, measInd, est] = locData;
    int flag = est < 0 ? est : 2 * est + measInd;
    int flagTrk = est < 0 ? est : 2 * est;
    // indicates coordinate that is estimated: *=from SP, o=from module center, #=globalToLocal(center) failure
    static const std::map<int, const char *> estimated_flags{{-1, "  "},
                                                             {0, " *"},
                                                             {1, "* "},
                                                             {2, " o"},
                                                             {3, "o "},
                                                             {4, " #"},
                                                             {5, "# "}};
    printVec2(loc, estimated_flags.at(flag));

    if (surface)
    {
      // momentum direction doesn't seem to be needed for measurement surfaces (only LineSurface?)
      auto glob = surface->localToGlobal(tgContext, loc, Acts::Vector3::Zero());
      printVec3(glob);

      if (compareMeasurementTransforms)
      {
        const ActsDetectorElement *
            acts_detector_element = dynamic_cast<const ActsDetectorElement *>(surface->associatedDetectorElement());
        if (acts_detector_element) {
           const InDetDD::SiDetectorElement *detElem = dynamic_cast< const InDetDD::SiDetectorElement *>(acts_detector_element->upstreamDetectorElement());

           // if measInd=1: won't match because comparing x,y and R,phi, but at least not phi,R.
           // This is still useful for debugging because the next test also fails.
           printVec2(locTrk, (measInd == 1 ? loc.reverse() : loc), estimated_flags.at(flagTrk));

           if (detElem)
              {
                 auto globTrk = detElem->surface().localToGlobal(locTrk);
                 printVec3(globTrk, glob);

                 auto res = surface->globalToLocal(tgContext, globTrk, Acts::Vector3::Zero());
                 if (!res.ok())
                    {
                       std::cout << " ** " << res.error() << " **";
                    }
                 else
                    {
                       printVec2(res.value(), loc);
                    }
              }
        }
      }

    }
    std::cout << std::defaultfloat << std::setprecision(-1);
  }

  static std::tuple<Acts::Vector2, Amg::Vector2D, int, int>
  localPositionStrip2D(const Acts::GeometryContext &tgContext,
                       const xAOD::UncalibratedMeasurement &measurement,
                       const Acts::Surface *surface,
                       const xAOD::SpacePoint *sp)
  {
    auto *disc = dynamic_cast<const Acts::DiscSurface *>(surface);
    Acts::Vector2 loc{Acts::Vector2::Zero()};
    int est = 2; // est = 0 (estimated from SP), 1 (from module center), 2 (globalToLocal(center) failure), -1 (pixel)
    if (surface)
    {
      if (sp)
      {
        auto res = surface->globalToLocal(tgContext, sp->globalPosition().cast<double>(), Acts::Vector3::Zero());
        if (res.ok())
        {
          loc = res.value();
          est = 0;
        }
      }

      if (est != 0)
      {
        if (auto *annulus = dynamic_cast<const Acts::AnnulusBounds *>(&surface->bounds()))
        {
          loc[0] = 0.5 * (annulus->rMin() + annulus->rMax());
          est = 1;
        }
        else
        {
          auto res = surface->globalToLocal(tgContext, surface->center(tgContext), Acts::Vector3::Zero());
          if (res.ok())
          {
            loc = res.value();
            est = 1;
          }
        }
      }
    }

    const int measInd = disc ? 1 : 0;
    loc[measInd] = measurement.localPosition<1>()[0];
    if (disc)
    {
      Amg::Vector2D locTrk{disc->localPolarToCartesian(loc).reverse()};
      locTrk[0] = -locTrk[0];
      return {loc, locTrk, measInd, est};
    }
    else
    {
      return {loc, loc, measInd, est};
    }
  }

  void
  TrackStatePrinterTool::printMeasurementAssociatedSpacePoint(const Acts::GeometryContext &tgContext,
							      const Acts::TrackingGeometry &tracking_geometry,
							      const DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap,
							      const xAOD::UncalibratedMeasurement *measurement,
							      const std::vector<small_vector<const xAOD::SpacePoint *>> &measToSp,
							      size_t offset) const
  {
    if (!measurement)
      return;

    std::cout << std::setw(5) << (measurement->index() + offset) << ' '
              << std::setw(3) << measurement->numDimensions() << "D ";

    const Acts::Surface *surface_ptr = ActsTrk::getSurfaceOfMeasurement( tracking_geometry, detectorElementToGeometryIdMap, *measurement);
    if (!surface_ptr)
    {
      std::cout << std::setw(20 + 22 + 20 + 2) << "** no surface for measurement **";
    }
    else
    {
      std::cout << std::left;
      std::cout << std::setw(21) << actsSurfaceName(*surface_ptr) << ' '
                << std::setw(22) << to_string(surface_ptr->geometryId()) << ' ';
      std::cout << std::setw(20) << atlasSurfaceName(surface_ptr);
      std::cout << std::right;
    }

    if (measurement->type() == xAOD::UncalibMeasType::PixelClusterType)
    {
      const auto loc = measurement->localPosition<2>().cast<double>();
      printMeasurement(tgContext, surface_ptr, {loc, loc, -1, -1}, m_compareMeasurementTransforms);
    }
    else if (measurement->type() == xAOD::UncalibMeasType::StripClusterType)
    {
      const small_vector<const xAOD::SpacePoint *> &spvec = measToSp.at(measurement->index());
      if (spvec.empty())
      {
        printMeasurement(tgContext, surface_ptr,
                         localPositionStrip2D(tgContext, *measurement, surface_ptr, nullptr),
                         m_compareMeasurementTransforms);
      }
      else
      {
        size_t isp = 0;
        for (auto *sp : spvec)
        {
          if (isp++)
          {
            std::cout << '\n'
                      << std::left
                      << std::setw(76) << to_string("** Spacepoint ", isp, " **")
                      << std::right;
          }
          printMeasurement(tgContext, surface_ptr,
                           localPositionStrip2D(tgContext, *measurement, surface_ptr, sp),
                           m_compareMeasurementTransforms);
        }
      }
    }
    else if (measurement->type() == xAOD::UncalibMeasType::HGTDClusterType) {
      const auto loc3D = measurement->localPosition<3>().cast<double>();
      const std::tuple<Acts::Vector2, Amg::Vector2D, int, int> locTup = {Acts::Vector2{loc3D.head<2>()}, Amg::Vector2D{loc3D.head<2>()}, -1, -1};
      printMeasurement(tgContext, surface_ptr, locTup, m_compareMeasurementTransforms);
    }
    std::cout << '\n';
  }

  void TrackStatePrinterTool::printParameters(const Acts::Surface &surface,
					      const Acts::GeometryContext &tgContext,
					      const Acts::BoundVector &bound)
  {
    auto p = Acts::transformBoundToFreeParameters(surface, tgContext, bound);
    std::cout << std::fixed
              << std::setw(10) << std::setprecision(4) << bound[Acts::eBoundLoc0] << ' '
              << std::setw(10) << std::setprecision(4) << bound[Acts::eBoundLoc1] << ' '
              << std::setw(9) << std::setprecision(3) << p.segment<2>(Acts::eFreePos0).norm() << ' '
              << std::setw(9) << std::setprecision(3) << std::atan2(p[Acts::eFreePos1], p[Acts::eFreePos0]) / Acts::UnitConstants::degree << ' '
              << std::setw(9) << std::setprecision(5) << std::atanh(p[Acts::eFreePos2] / p.segment<3>(Acts::eFreePos0).norm()) << ' '
              << std::setw(9) << std::setprecision(3) << p.segment<2>(Acts::eFreeDir0).norm() / p[Acts::eFreeQOverP] << ' '
              << std::setw(9) << std::setprecision(3) << std::atan2(p[Acts::eFreeDir1], p[Acts::eFreeDir0]) / Acts::UnitConstants::degree << ' '
              << std::setw(9) << std::setprecision(5) << std::atanh(p[Acts::eFreeDir2])
              << std::defaultfloat << std::setprecision(-1);
  }

  /// =========================================================================
  /// TrackStatePrinter class method definitions
  /// =========================================================================

  TrackStatePrinterTool::TrackStatePrinterTool(const std::string &type,
					       const std::string &name,
					       const IInterface *parent)
    : AthAlgTool(type, name, parent)
  {}

  StatusCode TrackStatePrinterTool::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG("   " << m_compareMeasurementTransforms);
    ATH_MSG_DEBUG("   " << m_printFilteredStates);

    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_spacePointKey.initialize());

    return StatusCode::SUCCESS;
  }

  void
  TrackStatePrinterTool::printSeed(const Acts::GeometryContext &tgContext,
				   const ActsTrk::Seed &seed,
				   const Acts::BoundTrackParameters &initialParameters,
				   const std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, size_t>> &offset,
				   size_t iseed,
				   bool isKF) const
  {
    if (!isKF)
      printHeader(1);

    std::ostringstream os;
    size_t nos = 0;
    for (const auto *sp : seed.sp())
    {
      size_t nom = 0;
      for (const auto *el : sp->measurements())
      {
        if (nom > 0)
          os << '+';
        else if (nos > 0)
          os << ',';
        ++nos;
        ++nom;
        os << el->index() + containerOffset(el->container(), offset);
      }
    }

    std::cout << std::setw(5) << iseed << ' '
              << std::left
              << std::setw(4) << (!isKF ? "seed" : "KF") << ' '
              << std::setw(21) << actsSurfaceName(initialParameters.referenceSurface()) << ' '
              << std::setw(22) << to_string(os.str()) << ' '
              << std::right;
    printParameters(initialParameters.referenceSurface(), tgContext, initialParameters.parameters());
    std::cout << '\n'
              << std::flush;
  }

  void
  TrackStatePrinterTool::printMeasurements(const EventContext &ctx,
					   const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
					   const DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap,
					   const std::vector<size_t> &offsets) const
  {
    const Acts::TrackingGeometry *
       acts_tracking_geometry = m_trackingGeometryTool->trackingGeometry().get();
    if (!acts_tracking_geometry) {
       ATH_MSG_WARNING("No Acts tracking geometry.");
       return;
    }
    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();

    auto measToSp = addSpacePoints(ctx, clusterContainers, offsets);

    ATH_MSG_INFO("CKF input measurements:");
    printHeader(0, m_compareMeasurementTransforms);

    for (std::size_t icontainer = 0; icontainer < clusterContainers.size(); ++icontainer)
    {
      for (const auto *measurement : *clusterContainers[icontainer])
      {
         printMeasurementAssociatedSpacePoint(tgContext,
                                              *acts_tracking_geometry,
                                              detectorElementToGeometryIdMap,
                                              measurement,
                                              measToSp[icontainer],
                                              offsets[icontainer]);
      }
    }
    std::cout << std::flush;
  }

  std::vector<std::vector<TrackStatePrinterTool::small_vector<const xAOD::SpacePoint *>>>
  TrackStatePrinterTool::addSpacePoints(const EventContext &ctx,
					const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
					const std::vector<size_t> &offsets) const
  {
    std::vector<std::vector<TrackStatePrinterTool::small_vector<const xAOD::SpacePoint *>>> measToSp{clusterContainers.size()};
    for (std::size_t icontainer = 0; icontainer < clusterContainers.size(); ++icontainer)
    {
      measToSp[icontainer].resize(clusterContainers[icontainer]->size());
    }

    for (auto &spacePointKey : m_spacePointKey)
    {
      ATH_MSG_DEBUG("Retrieving from input SpacePoint collection '" << spacePointKey.key() << "' ...");
      SG::ReadHandle<xAOD::SpacePointContainer> handle = SG::makeHandle(spacePointKey, ctx);
      if (!handle.isValid())
      {
        ATH_MSG_ERROR("Error retrieving from input SpacePoint collection '" << spacePointKey.key() << "'");
        continue;
      }
      ATH_MSG_DEBUG("    \\__ " << handle->size() << " elements!");
      for (const auto *sp : *handle)
      {
        for (const xAOD::UncalibratedMeasurement *meas : sp->measurements())
        {
          if (!meas)
            continue;
          for (std::size_t icontainer = 0; icontainer < clusterContainers.size(); ++icontainer)
          {
            // This measurement may well be in a different clusterContainer. Skip all but the one we are interested in.
            if (!(meas && meas->index() < clusterContainers[icontainer]->size() && meas == clusterContainers[icontainer]->at(meas->index())))
              continue;
            small_vector<const xAOD::SpacePoint *> &measSp = measToSp[icontainer].at(meas->index());
            if (!measSp.empty())
            {
              ATH_MSG_INFO("Cluster "
                           << (meas->index() + offsets[icontainer])
                           << " used by SpacePoints at ("
                           << sp->globalPosition()[0] << ',' << sp->globalPosition()[1] << ',' << sp->globalPosition()[2]
                           << ") and ("
                           << measSp[0]->globalPosition()[0] << ',' << measSp[0]->globalPosition()[1] << ',' << measSp[0]->globalPosition()[2]
                           << ')');
            }
            measSp.push_back(sp);
          }
        }
      }
    }
    return measToSp;
  }

} // namespace ActsTrk
