/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelModuleDesign.cxx
//   Implementation file for class PixelModuleDesign
///////////////////////////////////////////////////////////////////
// (c) ATLAS Pixel Detector software
///////////////////////////////////////////////////////////////////
// Version 1.9 18/03/2002 Alessandro Fornaini
// Modified: Grant Gorfine
///////////////////////////////////////////////////////////////////

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "Identifier/Identifier.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "ReadoutGeometryBase/SiReadoutCellId.h"

#include <cmath>
#include <utility>

namespace InDetDD {

using std::abs;

// Constructor with parameters:

PixelModuleDesign::PixelModuleDesign(const double thickness,
                                    const bool phiSymmetric,
                                    const bool etaSymmetric,
                                    const bool depthSymmetric,
                                    const int circuitsPerColumn,
                                    const int circuitsPerRow,
                                    const int cellColumnsPerCircuit,
                                    const int cellRowsPerCircuit,
                                    const int diodeColumnsPerCircuit,
                                    const int diodeRowsPerCircuit,
                                    PixelDiodeTree &&diode_tree,
                                    InDetDD::CarrierType carrierType,
                                    int readoutSide,
                                    bool is3D,
                                    InDetDD::DetectorType detectorType,
                                    PixelReadoutTechnology readoutTechnology
                                    ) :

  SiDetectorDesign(thickness, 
		   phiSymmetric, etaSymmetric, depthSymmetric,
		   carrierType,
		   readoutSide),
  m_diodeTree(std::move(diode_tree)),
  m_readoutScheme(circuitsPerColumn,circuitsPerRow,
		  cellColumnsPerCircuit,cellRowsPerCircuit,
		  diodeColumnsPerCircuit,diodeRowsPerCircuit),
  m_bounds(),
  m_detectorType(detectorType),
  m_readoutTechnology(readoutTechnology),
  m_is3D(is3D)
{
}

PixelModuleDesign::PixelModuleDesign(const double thickness,
                                    const int circuitsPerColumn,
                                    const int circuitsPerRow,
                                    const int cellColumnsPerCircuit,
                                    const int cellRowsPerCircuit,
                                    const int diodeColumnsPerCircuit,
                                    const int diodeRowsPerCircuit,
                                    PixelDiodeTree &&diode_tree,
                                    InDetDD::CarrierType carrierType,
                                    int readoutSide,
                                    bool is3D,
                                    InDetDD::DetectorType detectorType,
                                    PixelReadoutTechnology readoutTechnology
                                    ) :
    PixelModuleDesign(thickness, 
    true,true,true, //if symmetry not explicitly  set, assume fully symmetric
    circuitsPerColumn,circuitsPerRow,cellColumnsPerCircuit,cellRowsPerCircuit,
    diodeColumnsPerCircuit,diodeRowsPerCircuit,std::move(diode_tree),carrierType,readoutSide,
    is3D,detectorType, readoutTechnology)
{
}

// Returns distance to nearest detector edge 
// +ve = inside
// -ve = outside
void
PixelModuleDesign::distanceToDetectorEdge(const SiLocalPosition & localPosition,
					  double & etaDist, double & phiDist) const
{ 
  // This assume element is centered at 0,0 
  // As the calculation is symmetric around 0,0 we only have to test it for one side.
  double xEta = abs(localPosition.xEta());
  double xPhi = abs(localPosition.xPhi());

  double xEtaEdge = 0.5*length();
  double xPhiEdge = 0.5*width();

  // Distance to top/bottom
  etaDist = xEtaEdge - xEta;
  
  // Distance to right/left edge
  phiDist = xPhiEdge - xPhi;

}
 
SiDiodesParameters PixelModuleDesign::parameters(const SiCellId & cellId) const
{
   std::array<InDetDD::PixelDiodeTree::CellIndexType,2> diode_idx
       = InDetDD::PixelDiodeTree::makeCellIndex(cellId.phiIndex(),cellId.etaIndex());
   InDetDD::PixelDiodeTree::DiodeProxyWithPosition si_param ( m_diodeTree.diodeProxyFromIdxCachePosition(diode_idx));

   return SiDiodesParameters(SiLocalPosition(Amg::Vector2D{si_param.position()[0],si_param.position()[1]}),
                             SiLocalPosition(Amg::Vector2D{si_param.width()[0],si_param.width()[1]}));
}

SiLocalPosition PixelModuleDesign::localPositionOfCell(const SiCellId & cellId) const
{
   std::array<InDetDD::PixelDiodeTree::CellIndexType,2> diode_idx
       = InDetDD::PixelDiodeTree::makeCellIndex(cellId.phiIndex(),cellId.etaIndex());
   auto pos=m_diodeTree.findFromIdx(diode_idx);
   return SiLocalPosition(Amg::Vector2D{pos[0],pos[1]});
}


// Helper method for stereo angle computation
HepGeom::Vector3D<double> PixelModuleDesign::phiMeasureSegment(const SiLocalPosition&) const
{
  HepGeom::Vector3D<double> segment;
  segment[etaAxis()]=1;
  return segment;
}

// Special method for SCT (irrelevant here):
std::pair<SiLocalPosition,SiLocalPosition> PixelModuleDesign::endsOfStrip(const SiLocalPosition &position) const
{
  return std::pair<SiLocalPosition,SiLocalPosition>(position,position);
}
 
  
// Methods to calculate length of a module
double PixelModuleDesign::length() const
{
   return m_diodeTree.totalWidth()[1]; // eta
}

// Methods to calculate average width of a module
double PixelModuleDesign::width() const
{
  return m_diodeTree.totalWidth()[0]; // phi
}

// Methods to calculate minimum width of a module
double PixelModuleDesign::minWidth() const
{
  return width();
}

// Methods to calculate maximum width of a module
double PixelModuleDesign::maxWidth() const
{
  return width();
}


// Method to calculate eta width from a column range
double PixelModuleDesign::widthFromColumnRange(const int colMin, const int colMax) const 
{
  SiCellId idMin(0, colMin);
  SiCellId idMax(0, colMax);
  
  double minEta = parameters(idMin).xEtaMin(); 
  double maxEta = parameters(idMax).xEtaMax();
  
  return fabs(maxEta-minEta);
}

// Method to calculate phi width from a row range
double PixelModuleDesign::widthFromRowRange(const int rowMin, const int rowMax) const 
{

  SiCellId idMin(rowMin, 0);
  SiCellId idMax(rowMax, 0);
  double minPhi = parameters(idMin).xPhiMin();
  double maxPhi = parameters(idMax).xPhiMax(); 

  return fabs(maxPhi-minPhi);
}

// Pitch in phi direction
double 
PixelModuleDesign::phiPitch() const
{
  // Average pitch. 
  return width() / rows();
}

// Pitch in phi direction
double 
PixelModuleDesign::phiPitch(const SiLocalPosition &) const
{
  // Cheat since we know its constant.
  return phiPitch();
}

// Pitch in eta direction
double 
PixelModuleDesign::etaPitch() const
{
  // Average pitch
  return length() / columns();
}

SiCellId PixelModuleDesign::cellIdOfPosition(const SiLocalPosition & localPosition) const
{
   InDetDD::PixelDiodeTree::Vector2D pos{ localPosition.xPhi(),localPosition.xEta() };
   auto idx=m_diodeTree.findFromPos(pos);
   if (!isInsideMatrix(idx)) {
      return SiCellId();
   }
   return SiCellId(idx[0],idx[1]);
}

int PixelModuleDesign::numberOfConnectedCells(const SiReadoutCellId & readoutId) const
{
  return m_readoutScheme.numberOfConnectedCells(readoutId);
}

SiCellId 
PixelModuleDesign::connectedCell(const SiReadoutCellId & readoutId, int number) const
{
  return m_readoutScheme.connectedCell(readoutId, number);
}

SiCellId 
PixelModuleDesign::gangedCell(const SiCellId & cellId) const
{
  return m_readoutScheme.gangedCell(cellId);  
}

SiReadoutCellId
PixelModuleDesign::readoutIdOfCell(const SiCellId & cellId) const
{
  return m_readoutScheme.readoutIdOfCell(cellId);
}


SiReadoutCellId 
PixelModuleDesign::readoutIdOfPosition(const SiLocalPosition & localPosition) const
{
  InDetDD::PixelDiodeTree::Vector2D pos{ localPosition.xPhi(),localPosition.xEta() };
  auto idx=m_diodeTree.findFromPos(pos);
  return m_readoutScheme.readoutIdOfCell(SiCellId(idx[0],idx[1]));
}

// Given row and column index of diode, returns position of diode center
// ALTERNATIVE/PREFERED way is to use localPositionOfCell(const SiCellId & cellId) or 
// rawLocalPositionOfCell method in SiDetectorElement.
// DEPRECATED (but used in numerous places)
SiLocalPosition PixelModuleDesign::positionFromColumnRow(const int column, const int row) const
{
  return localPositionOfCell(SiCellId(row,column));
}
 

const Trk::SurfaceBounds & 
PixelModuleDesign::bounds() const
{
  // We create on demand as width and length are 0 when PixeModuleDesign first gets
  // created.
  if (not m_bounds) m_bounds.set(std::make_unique<Trk::RectangleBounds>(0.5*width(), 0.5*length()));
  return *m_bounds;
}

SiCellId 
PixelModuleDesign::cellIdInRange(const SiCellId & cellId) const
{
  return (isInsideMatrix(cellId) ? cellId : SiCellId());
}

DetectorType PixelModuleDesign::type() const
{
  return m_detectorType;
}

} // namespace InDetDD
