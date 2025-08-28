/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class AscObj_TrackState                 //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1TrackSystems/AscObj_TrackState.h"

#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include "VP1HEPVis/nodes/SoTubs.h"

#include <set>

#include "Acts/Surfaces/Surface.hpp"
#include "Acts/EventData/ParticleHypothesis.hpp"
#include "ActsGeometry/ATLASSourceLink.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "VP1Base/IVP13DSystem.h"
#include "VP1Base/VP1CameraHelper.h"
#include "VP1Base/VP1Msg.h"
#include "VP1Base/VP1QtInventorUtils.h"
#include "VP1Base/VP1QtUtils.h"
#include "VP1HEPVis/nodes/SoGenericBox.h"
#include "VP1HEPVis/nodes/SoTransparency.h"
#include "VP1TrackSystems/AscObjSelectionManager.h"
#include "VP1TrackSystems/TrackHandleBase.h"
#include "VP1TrackSystems/TrackSysCommonData.h"
#include "VP1TrackSystems/TrackSystemController.h"
#include "VP1TrackSystems/TrkObjToString.h"
#include "VP1TrackSystems/VP1TrackSystem.h"
#include "VP1Utils/SurfaceToSoNode.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Utils/VP1ErrorUtils.h"
#include "VP1Utils/VP1LinAlgUtils.h"

#include "Acts/Surfaces/ConeSurface.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"

static double surfaceThickness = 0.1;

//____________________________________________________________________
AscObj_TrackState::AscObj_TrackState(
    TrackHandleBase* track, unsigned indexOfPointOnTrack,
    const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy& trackstate)
    : AssociatedObjectHandleBase(track),
      m_parts(TrackCommonFlags::TSOS_NoObjects),
      m_indexOfPointOnTrack(indexOfPointOnTrack),
      m_distToNextPar(-1),
      m_objBrowseTree(nullptr),
      m_trackstate(trackstate) 
{
  SoTransparency::initClass();
  if (m_trackstate.hasReferenceSurface()) {
    if (surface().associatedDetectorElement())
      m_parts |= TrackCommonFlags::TSOS_SurfacesDetElem;
    else
      m_parts |= TrackCommonFlags::TSOS_SurfacesCustom;
  }

  if (m_trackstate.typeFlags().test(Acts::TrackStateFlag::ParameterFlag ))
    m_parts |= TrackCommonFlags::TSOS_TrackPars;
 
  if (m_trackstate.typeFlags().test(Acts::TrackStateFlag::MeasurementFlag)){
    m_parts |= TrackCommonFlags::TSOS_AnyMeasurement;
    if (m_trackstate.typeFlags().test(Acts::TrackStateFlag::OutlierFlag ))
      m_parts |= TrackCommonFlags::TSOS_MeasRioOnTrackOutlier;
    else
      m_parts |= TrackCommonFlags::TSOS_MeasRioOnTrackNotOutlier;
  }

  if (m_trackstate.typeFlags().test(Acts::TrackStateFlag::HoleFlag))
    m_parts |= TrackCommonFlags::TSOS_Hole;
  
  if (m_trackstate.typeFlags().test(Acts::TrackStateFlag::MaterialFlag ))
    m_parts |= TrackCommonFlags::TSOS_MaterialEffects;
}

//____________________________________________________________________
void AscObj_TrackState::setDistToNextPar(const double& d) {
  // We assume this is called right after the constructor - so no need
  // to update 3D objects.
  m_distToNextPar = d;
}

//____________________________________________________________________
Amg::Vector3D AscObj_TrackState::approxCenter() const {
  if (hasSurface())
    return surface().center( common()->geometryContext().context() );
  VP1Msg::message(
      "AscObj_TrackState::approxCenter() WARNING: Failed to determine"
      " position from either params or surface");
  return Amg::Vector3D(0, 0, 0);
}

//____________________________________________________________________
int AscObj_TrackState::regionIndex() const {
  Amg::Vector3D c = approxCenter();
  static const double l = 30.0 * CLHEP::cm;
  return static_cast<int>(c.z() / l) + 1000 * static_cast<int>(c.y() / l) +
         1000000 * static_cast<int>(c.x() / l);
}

// TODO - add back getZTranslationTube

//____________________________________________________________________
void AscObj_TrackState::addTrackParamInfoToShapes(SoSeparator*& shape_simple,
                                                  SoSeparator*& shape_detailed,
                                                  bool showPars,
                                                  bool showParsErrors,
                                                  bool /**showSurfaces*/) {
  ensureInitSeps(shape_simple, shape_detailed);

  SoSeparator* param_simple = new SoSeparator;
  SoSeparator* param_detailed = new SoSeparator;

  if (!m_trackstate.hasReferenceSurface()) {
    // FIXME - do not know how to handle this yet, since I think I need the
    // surface to get the position
    VP1Msg::messageVerbose(
        "AscObj_TrackState::addTrackParamInfoToShapes() - no reference "
        "surface");
    return;
  }

  const Acts::BoundTrackParameters trackparams(
      m_trackstate.referenceSurface().getSharedPtr(), m_trackstate.parameters(),
      m_trackstate.covariance(), Acts::ParticleHypothesis::pion());
  auto p1 = trackparams.position(common()->geometryContext().context());
  if (showPars) {
    auto u = trackparams.direction().unit();
    double length = 15 * CLHEP::cm;
    if (m_distToNextPar > 0)
      length = std::min(m_distToNextPar * 0.75, length);
    Amg::Vector3D p2 = p1 + length * u;
    // Line:
    SoLineSet* line = new SoLineSet();
    SoVertexProperty* vertices = new SoVertexProperty();
    vertices->vertex.set1Value(0, p1.x(), p1.y(), p1.z());
    vertices->vertex.set1Value(1, p2.x(), p2.y(), p2.z());
    line->numVertices.set1Value(0, 2);

    // Point:
    SoPointSet* points = new SoPointSet;
    SoVertexProperty* vertices2 = new SoVertexProperty;
    vertices2->vertex.set1Value(0, p1.x(), p1.y(), p1.z());
    points->numPoints = 1;
    line->vertexProperty = vertices;
    points->vertexProperty = vertices2;
    bool isHole = m_parts & TrackCommonFlags::TSOS_Hole;

    if (trackHandle()->customColouredTSOSParts() &
            TrackCommonFlags::TSOS_TrackPars ||
        (isHole && (trackHandle()->customColouredTSOSParts() &
                    TrackCommonFlags::TSOS_Hole))) {
      SoMaterial* mat = isHole
                            ? common()->controller()->customMatHoleParameters()
                            : common()->controller()->customMatParameters();
      SoSeparator* sep = new SoSeparator;
      sep->addChild(mat);
      sep->addChild(line);
      sep->addChild(points);
      param_simple->addChild(sep);
      param_detailed->addChild(sep);
    } else {
      param_simple->addChild(line);
      param_simple->addChild(points);
      param_detailed->addChild(line);
      param_detailed->addChild(points);
    }
    if (showParsErrors) {
      // TODO
    }
    shape_simple->addChild(param_simple);
    shape_detailed->addChild(param_detailed);
  }
}

//____________________________________________________________________
void AscObj_TrackState::addSurfaceToShapes(SoSeparator*& shape_simple,
                                           SoSeparator*& shape_detailed) {
    // VP1Msg::messageVerbose("\tAscObj_TrackState::addSurfaceToShapes() start. Type="+QString::number(surface().type())+" Bounds="+QString::number(surface().bounds().type()));
  // Check if the TrackStateProxy has a surface

  if (!m_trackstate.hasReferenceSurface())
    return;
  
  const Acts::Surface& surface = m_trackstate.referenceSurface();
  if (surface.type()==Acts::Surface::SurfaceType::Straw && common()->controller()->hideTubeSurfaces())
    return;
  
  SoTransform* sotra = VP1LinAlgUtils::toSoTransform(surface.transform(common()->geometryContext().context()));
  shape_detailed->addChild(sotra);

  // 
  //   enum SurfaceType {
  //   Cone = 0,
  //   Cylinder = 1,
  //   Disc = 2,
  //   Perigee = 3,
  //   Plane = 4,
  //   Straw = 5,
  //   Curvilinear = 6,
  //   Other = 7
  // };

  //  enum BoundsType : int {
  //   eCone = 0,
  //   eCylinder = 1,
  //   eDiamond = 2,
  //   eDisc = 3,
  //   eEllipse = 4,
  //   eLine = 5,
  //   eRectangle = 6,
  //   eTrapezoid = 7,
  //   eTriangle = 8,
  //   eDiscTrapezoid = 9,
  //   eConvexPolygon = 10,
  //   eAnnulus = 11,
  //   eBoundless = 12,
  //   eOther = 13
  // };



  switch (surface.type()) {
    case Acts::Surface::SurfaceType::Plane: {
      addPlaneSurfaceToShapes(shape_simple, shape_detailed, surface);
      break;
    }
    case Acts::Surface::SurfaceType::Straw: {
      addCylindricalSurfaceToShapes(shape_simple, shape_detailed, surface);
      break;
    }
    default:
       VP1Msg::messageVerbose("Cannot currently handle surface of type" + QString::number(surface.type()));
      break;
  }
  VP1Msg::messageVerbose("AscObj_TrackState::addSurfaceToShapes() end");
}

void AscObj_TrackState::addMaterialToSurfaceShapes(SoNode*& /**shape_simple*/,
                                                SoNode*& shape_detailed) {
    if (trackHandle()->customColouredTSOSParts()&TrackCommonFlags::TSOS_AnySurface) {
      SoMaterial * mat = common()->controller()->customMatSurfaces();
      if (shape_detailed->getTypeId().isDerivedFrom(SoSeparator::getClassTypeId())) {
        // static_cast<SoSeparator*>(theSurfSepSimple)->insertChild(mat,0); TODO
        static_cast<SoSeparator*>(shape_detailed)->insertChild(mat,0);
      } else {

        // SoSeparator * sepSimple = new SoSeparator;
        // sepSimple->addChild(mat);
        // sepSimple->addChild(theSurfSepSimple);
        // nodeToAddSimple = sepSimple;
        // TODO ^
        SoSeparator * sep = new SoSeparator;
        sep->addChild(mat);
        sep->addChild(shape_detailed);
        shape_detailed = sep;
      }
    }
}

void AscObj_TrackState::addPlaneSurfaceToShapes(SoSeparator*& shape_simple,
                                                SoSeparator*& shape_detailed,
                                                const Acts::Surface& surface) {
  
  SoNode * nodeToAddSimple = nullptr;
  SoNode * nodeToAddDetailed = nullptr;

  // Check bounds
  switch (surface.bounds().type()) {
    case Acts::SurfaceBounds::BoundsType::eBoundless: {
      const double halfX = 100.0; // FIXME - make this configurable?
      const double halfY = 100.0;
      SoGenericBox* box = new SoGenericBox;
      box->setParametersForBox(halfX, halfY,
                              0.5 * surfaceThickness);
      box->drawEdgeLines.setValue(true);
      nodeToAddDetailed = box;
      break;
    }
    case Acts::SurfaceBounds::BoundsType::eRectangle: {

      const Acts::RectangleBounds& rectBounds =
          dynamic_cast<const Acts::RectangleBounds&>(surface.bounds());
      const double halfX = rectBounds.halfLengthX();
      const double halfY = rectBounds.halfLengthY();
      SoGenericBox* box = new SoGenericBox;
      box->setParametersForBox(halfX, halfY,
                              0.5 * surfaceThickness);
      box->drawEdgeLines.setValue(true);
      nodeToAddDetailed = box;
      // TODO simple
      break;
    }
    default: {
       VP1Msg::messageVerbose("Cannot currently handle surface bound of type" + QString::number(surface.bounds().type()));
      break;
    }
  }

  if (nodeToAddDetailed!=nullptr){
    addMaterialToSurfaceShapes(nodeToAddSimple, nodeToAddDetailed);
    shape_simple->addChild(nodeToAddDetailed); // FIXME
    shape_detailed->addChild(nodeToAddDetailed);
  } 
}

void AscObj_TrackState::addCylindricalSurfaceToShapes(SoSeparator*& shape_simple,
                                                SoSeparator*& shape_detailed,
                                                const Acts::Surface& surface) {
  SoNode * nodeToAddSimple = nullptr;
  SoNode * nodeToAddDetailed = nullptr;
  // Check bounds
  switch (surface.bounds().type()) {
    case Acts::SurfaceBounds::BoundsType::eLine: {
      const Acts::LineBounds& lineBounds =
          dynamic_cast<const Acts::LineBounds&>(surface.bounds());
      double hlength =  lineBounds.get(Acts::LineBounds::eHalfLengthZ);

      SoVertexProperty * scatVtxProperty = new SoVertexProperty();
      scatVtxProperty->vertex.set1Value(0, 0.0,0.0,-hlength);
      scatVtxProperty->vertex.set1Value(1, 0.0,0.0, hlength);
      SoLineSet * lineSurface = new SoLineSet();
      lineSurface->numVertices = 2;
      lineSurface->vertexProperty = scatVtxProperty;

      nodeToAddSimple = lineSurface;

      double radius =  lineBounds.get(Acts::LineBounds::eR);

      SoTubs* lineSurfaceDetailed = new SoTubs();
      (*lineSurfaceDetailed).pRMin = 0.;
      (*lineSurfaceDetailed).pRMax = radius;
      (*lineSurfaceDetailed).pDz   = hlength;

      nodeToAddDetailed = lineSurfaceDetailed;
      break;
    }
    default: {
        VP1Msg::messageVerbose("AscObj_TrackState::addCylindricalSurfaceToShapes(): Unsupported bounds type.");
      break;
    }
  }
  if (nodeToAddDetailed!=nullptr){
    addMaterialToSurfaceShapes(nodeToAddSimple, nodeToAddDetailed);
    shape_simple->addChild(nodeToAddDetailed);
    shape_detailed->addChild(nodeToAddDetailed);
  }
}

//____________________________________________________________________
void AscObj_TrackState::addMaterialEffectsToShapes(
    SoSeparator*& /**shape_simple*/, SoSeparator*& /**shape_detailed*/) {}

void AscObj_TrackState::addMeasurementToShapes(SoSeparator*& shape_simple,
                                               SoSeparator*& shape_detailed) {

  auto flag = m_trackstate.typeFlags();

  // Check if the TrackStateProxy has a measurement
  if (!m_trackstate.hasReferenceSurface() ||
      !flag.test(Acts::TrackStateFlag::MeasurementFlag ||
                 !(m_trackstate.hasUncalibratedSourceLink())))
    return;

  ensureInitSeps(shape_simple, shape_detailed);
  TrackCommonFlags::TSOSPartsFlags f(trackHandle()->shownTSOSParts() & m_parts);
  const bool showMeas(f & TrackCommonFlags::TSOS_AnyMeasurement);
  // const bool showMeasErrors(f & TrackCommonFlags::TSOS_MeasError);
  if (showMeas) {
    if (m_parts & trackHandle()->customColouredTSOSParts() &
        TrackCommonFlags::TSOS_AnyMeasRioOnTrack) {
      SoMaterial* mat;
      if (m_parts & trackHandle()->customColouredTSOSParts() &
          TrackCommonFlags::TSOS_MeasRioOnTrackNotOutlier)
        mat = common()->controller()->customMatMeasurements();
      else
        mat = common()->controller()->customMatMeasurementsOutliers();
      shape_simple->addChild(mat);
      shape_detailed->addChild(mat);
    }

    // Handle measurements
    auto sl = m_trackstate.getUncalibratedSourceLink()
                  .get<ActsTrk::ATLASUncalibSourceLink>();
    assert(sl != nullptr);
    const xAOD::UncalibratedMeasurement& uncalibMeas =
        (ActsTrk::getUncalibratedMeasurement(sl));

    const xAOD::UncalibMeasType measurementType = uncalibMeas.type();

    switch (measurementType) {
      case xAOD::UncalibMeasType::MdtDriftCircleType: {
        // Drift tubes
        break;
      }
      case xAOD::UncalibMeasType::RpcStripType:
      case xAOD::UncalibMeasType::TgcStripType:
      case xAOD::UncalibMeasType::MMClusterType:
      case xAOD::UncalibMeasType::sTgcStripType:
      case xAOD::UncalibMeasType::HGTDClusterType:
      case xAOD::UncalibMeasType::StripClusterType: {
        // TODO
        break;
      }
      case xAOD::UncalibMeasType::PixelClusterType: {
        // TODO
        break;
      }
      case xAOD::UncalibMeasType::Other:
      default: {
        VP1Msg::message(
            "AscObj_TrackState::addMeasurementToShapes: Unable to handle this "
            "measurement type ");
        break;
      }
    }
  }
}

//____________________________________________________________________
void AscObj_TrackState::ensureInitSeps(SoSeparator*& shape_simple,
                                       SoSeparator*& shape_detailed) {
  if (!shape_simple)
    shape_simple = new SoSeparator;
  if (!shape_detailed)
    shape_detailed = new SoSeparator;
}

//____________________________________________________________________
void AscObj_TrackState::buildShapes(SoSeparator*& shape_simple,
                                    SoSeparator*& shape_detailed) {
  VP1Msg::messageVerbose("AscObj_TrackState::buildShapes() start");

  TrackCommonFlags::TSOSPartsFlags f(trackHandle()->shownTSOSParts()&m_parts);
  if (f==TrackCommonFlags::TSOS_NoObjects){
    VP1Msg::messageVerbose("AscObj_TrackState::buildShapes() - no objects to show with shownTSOSParts="+QString::number(trackHandle()->shownTSOSParts())+" and m_parts="+QString::number(m_parts));
    return;
  }
  // TODO Implement the following
  const bool showPars(f & TrackCommonFlags::TSOS_TrackPars);
  // const bool showParsErrors(f & TrackCommonFlags::TSOS_AnyParsErrors);
  // const bool showMeas(f & TrackCommonFlags::TSOS_AnyMeasRioOnTrack);
  // const bool showCompetingRioOnTrack(f & TrackCommonFlags::TSOS_AnyMeasCompetingRioOnTrack);
  const bool showSurfaces(f & TrackCommonFlags::TSOS_AnySurface);
  // const bool showMaterialEffects(f & TrackCommonFlags::TSOS_AnyMaterialEffects);
  // const bool showMeasErrors(f & TrackCommonFlags::TSOS_MeasError);
  ensureInitSeps(shape_simple,shape_detailed);

  if (showPars)
    addTrackParamInfoToShapes( shape_simple, shape_detailed, showPars, false, false);

  if (showSurfaces)
    addSurfaceToShapes( shape_simple, shape_detailed);

  // addMeasurementToShapes is crashing at the moment. Need to investigate
  // if (showMeas)
  //   addMeasurementToShapes( shape_simple, shape_detailed);

  VP1Msg::messageVerbose("AscObj_TrackState::buildShapes() end");

}

//____________________________________________________________________
QStringList AscObj_TrackState::clicked() {
  QStringList l;
  return l;
}

void AscObj_TrackState::setVisible(bool vis) {
  AssociatedObjectHandleBase::setVisible(vis);
}

void AscObj_TrackState::zoomView() {
  VP1Msg::messageVerbose("AscObj_TrackState::zoomView()");
  if ( common()->controller()->orientAndZoomOnSingleSelection() )
  {
    // Zoom without orientation
    // TODO - add orientation?
      VP1Msg::messageVerbose("AscObj_TrackState::zoomView() Zoom without orientation");
      std::set<SoCamera*> cameras = common()->system()->getCameraList();
      std::set<SoCamera*>::iterator it,itE = cameras.end();
      for (it=cameras.begin();it!=itE;++it) {
        if (common()->controller()->assocObjDetailLevel()==TrackCommonFlags::SIMPLE)
        {
          VP1CameraHelper::animatedZoomToSubTree(*it,common()->ascObjSelectionManager()->getAscObjAttachSep(),shapeSimple(),2.0,1.0);
        } else
        {
          VP1CameraHelper::animatedZoomToSubTree(*it,common()->ascObjSelectionManager()->getAscObjAttachSep(),shapeDetailed(),2.0,1.0);
        }
      }
  } else {
    VP1Msg::messageVerbose("AscObj_TrackState::zoomView() - zooming on selection not turned on.");
  }
}

