/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonGeoUtilityTool.h"

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <GeoModelKernel/GeoBox.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoSimplePolygonBrep.h>

#include <GeoModelKernel/GeoTube.h>

#include <GeoModelKernel/GeoShapeUnion.h>
#include <GeoModelKernel/GeoShapeIntersection.h>
#include <GeoModelKernel/GeoShapeSubtraction.h>
#include <GeoModelKernel/GeoShapeShift.h>

#include <GeoModelKernel/GeoTransform.h>

#include <GeoModelKernel/GeoVolumeCursor.h>

#include <GeoModelHelpers/GeoShapeUtils.h>
#include <GeoModelHelpers/printVolume.h>
#include <GeoModelHelpers/TransformToStringConverter.h>


#include <set>
#include <sstream>
#include <string>


using namespace ActsTrk;
namespace MuonGMR4{

MuonGeoUtilityTool::~MuonGeoUtilityTool() = default;

const GeoShape* MuonGeoUtilityTool::extractShape(const PVConstLink& physVol) const {
    const GeoLogVol* logVol = physVol->getLogVol();
    if (!logVol) {
        ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" Physical volume has no logical volume attached ");
        return nullptr;
    }
    return extractShape(logVol->getShape());
}
const GeoShape* MuonGeoUtilityTool::extractShape(const GeoShape* inShape) const {
   
    if (!inShape) {
      ATH_MSG_INFO(__FILE__<<":"<<__LINE__<<" "<<__func__<<" nullptr given ");
      return nullptr;
    }    
    if (inShape->typeID() == GeoShapeShift::getClassTypeID()) {
        const GeoShapeShift* shift =  dynamic_pointer_cast<const GeoShapeShift>(compressShift(inShape));
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<
                        "Shape is a shift by "<<GeoTrf::toString(shift->getX())
                        << ". Continue navigation "<<shift);
        return extractShape(shift->getOp());
    }
    if (inShape->typeID() == GeoShapeSubtraction::getClassTypeID()){
      ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<
                      "Shape is a subtraction. Extract the basic shape. Continue navigation "<<inShape);
      const GeoShapeSubtraction* subtract = static_cast<const GeoShapeSubtraction*>(inShape);
      return extractShape(subtract->getOpA());
    }    
    return inShape;
}   
Amg::Transform3D MuonGeoUtilityTool::extractShifts(const PVConstLink& physVol) const { 
    const GeoLogVol* logVol = physVol->getLogVol();
    if (!logVol) {
      ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" Physical volume has no logical volume attached. ");
      return Amg::Transform3D::Identity();
    }
    return extractShifts(logVol->getShape());
}

Amg::Transform3D MuonGeoUtilityTool::extractShifts(const GeoShape* inShape) const { 
  if (!inShape) {
      ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" "<<__func__<<" nullptr given ");
      return Amg::Transform3D::Identity();
  }  
  Amg::Transform3D sumTrans{Amg::Transform3D::Identity()};
  if (inShape->typeID() == GeoShapeShift::getClassTypeID()) {
        const GeoShapeShift* shift = dynamic_pointer_cast<const GeoShapeShift>(compressShift(inShape));
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Shape is a shift . Continue navigation "<<shift);
        sumTrans = shift->getX();
    }
    ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Extacted transformation "<<GeoTrf::toString(sumTrans));
    return sumTrans;
}
std::string MuonGeoUtilityTool::dumpShape(const GeoShape* shape) const { return printGeoShape(shape); }
std::string MuonGeoUtilityTool::dumpVolume(const PVConstLink& physVol) const { return printVolume(physVol); }
const GeoAlignableTransform* MuonGeoUtilityTool::findAlignableTransform(const PVConstLink& physVol) const {
    PVConstLink parent{physVol->getParent()}, child{physVol};
    while (parent) {
       const GeoGraphNode * const * node1 = parent->findChildNode(child);
       const GeoGraphNode * const * fence =  parent->getChildNode(0);
       for(const GeoGraphNode * const * current = node1 - 1; current>=fence; current--) {
          const GeoGraphNode* node{*current};
          if (dynamic_cast<const GeoVPhysVol*>(node)) break;
          const GeoAlignableTransform* alignTrans{dynamic_cast<const GeoAlignableTransform*>(node)};
          if (alignTrans) return alignTrans;
       }       
       child = parent;
       parent = child->getParent();
    }
    return nullptr;
}

std::vector<MuonGeoUtilityTool::physVolWithTrans> MuonGeoUtilityTool::findAllLeafNodesByName(const PVConstLink& physVol, const std::string& volumeName) const {
  return getAllSubVolumes(physVol,[&volumeName](const physVolWithTrans& child){
                                                 return child.volume->getLogVol()->getName() == volumeName || child.nodeName == volumeName;
                                  });
}
std::vector<const GeoShape*> MuonGeoUtilityTool::getComponents(const GeoShape* booleanShape) const {
   return getBooleanComponents(booleanShape);
}

std::vector<Amg::Vector2D> MuonGeoUtilityTool::polygonEdges(const GeoSimplePolygonBrep& polygon) const {
  std::vector<Amg::Vector2D> polygonEdges{};
  polygonEdges.reserve(polygon.getNVertices());
  for (unsigned int i = 0; i < polygon.getNVertices(); ++i) {
      polygonEdges.emplace_back(polygon.getXVertex(i), polygon.getYVertex(i));
      ATH_MSG_VERBOSE("Polygon vertext point  " << i << ": "<< GeoTrf::toString(polygonEdges.back(), 2));
    }
  return polygonEdges;
}

std::vector<Amg::Vector3D> MuonGeoUtilityTool::shapeEdges(const GeoShape* shape,
                                                          const Amg::Transform3D& refTrf) const {
    return getPolyShapeEdges(shape, refTrf);
}


}
