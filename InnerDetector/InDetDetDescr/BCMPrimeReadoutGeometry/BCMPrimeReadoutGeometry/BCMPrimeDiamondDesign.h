/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDIAMONDDESIGN_H
#define BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDIAMONDDESIGN_H

#include <InDetReadoutGeometry/SiDetectorDesign.h>
#include "ReadoutGeometryBase/SiCellId.h"
#include "ReadoutGeometryBase/SiReadoutCellId.h"

namespace InDetDD
{

/**
 * @class BCMPrimeDiamondDesign
 * Design class for BCMPrime diamond sensors.
 * 
 * BCMPrime uses simple diamond pad sensors with minimal readout structure:
 * - Two pad sizes: 10x10 mm and 5x5 mm (defined in ITKLayouts)
 * - Fixed thickness: ~0.5 mm (diamond pad thickness)
 * - No internal circuit/diode hierarchy (unlike Pixel)
 * 
 * Inherits from SiDetectorDesign to integrate with the digitization framework.
 */
class BCMPrimeDiamondDesign : public SiDetectorDesign
{
public:
  /**
   * Constructor for BCMPrime diamond sensor design.
   * @param sizeX Pad size in X direction (mm) - typically 10.0 or 5.0
   * @param sizeY Pad size in Y direction (mm) - typically 10.0 or 5.0
   * @param thickness Diamond sensor thickness (mm) - typically 0.5
   */
  BCMPrimeDiamondDesign(double sizeX, double sizeY, double thickness);

  /**
   * Destructor
   */
  virtual ~BCMPrimeDiamondDesign() = default;

  /**
   * Get pad size in X direction
   */
  double sizeX() const { return m_sizeX; }

  /**
   * Get pad size in Y direction
   */
  double sizeY() const { return m_sizeY; }

  /**
   * Get diamond sensor thickness
   */
  double diamondThickness() const { return m_thickness; }

  // Minimal implementations to satisfy DetectorDesign/SiDetectorDesign interfaces
  virtual void distanceToDetectorEdge(const SiLocalPosition &localPosition,
                                       double &etaDist,
                                       double &phiDist) const override {
    // simple rectangular pad: distance from center half-sizes
    etaDist = 0.5 * m_sizeY - std::abs(localPosition.xEta());
    phiDist = 0.5 * m_sizeX - std::abs(localPosition.xPhi());
  }

  virtual double length() const override { return m_sizeY; }
  virtual double width() const override { return m_sizeX; }
  virtual double minWidth() const override { return m_sizeX; }
  virtual double maxWidth() const override { return m_sizeX; }

  virtual double phiPitch() const override { return m_sizeX; }
  virtual double phiPitch(const SiLocalPosition&) const override { return m_sizeX; }
  virtual double etaPitch() const override { return m_sizeY; }

  virtual bool swapHitPhiReadoutDirection() const override { return false; }
  virtual bool swapHitEtaReadoutDirection() const override { return false; }

  virtual const Trk::SurfaceBounds& bounds() const override;

  virtual SiDiodesParameters parameters(const SiCellId&) const override { return SiDiodesParameters(); }
  virtual SiLocalPosition localPositionOfCell(const SiCellId&) const override { return SiLocalPosition(); }
  virtual int numberOfConnectedCells(const SiReadoutCellId&) const override { return 1; }
  virtual SiCellId connectedCell(const SiReadoutCellId&, int) const override { return SiCellId(); }
  virtual SiReadoutCellId readoutIdOfCell(const SiCellId&) const override { return SiReadoutCellId(); }
  virtual SiReadoutCellId readoutIdOfPosition(const SiLocalPosition&) const override { return SiReadoutCellId(); }
  virtual SiCellId cellIdOfPosition(const SiLocalPosition&) const override { return SiCellId(); }
  virtual void neighboursOfCell(const SiCellId&, std::vector<SiCellId>&) const override {}
  virtual SiCellId cellIdInRange(const SiCellId&) const override { return SiCellId(); }

  // SiDetectorDesign pure virtuals
  virtual bool nearBondGap(const SiLocalPosition&, double) const override { return false; }
  virtual HepGeom::Vector3D<double> phiMeasureSegment(const SiLocalPosition&) const override { return HepGeom::Vector3D<double>(0,0,0); }
  virtual std::pair<SiLocalPosition, SiLocalPosition> endsOfStrip(const SiLocalPosition &p) const override { return {p,p}; }
  virtual SiCellId gangedCell(const SiCellId &cellId) const override { return SiCellId(); }

private:
  double m_sizeX;        ///< Pad size X in mm
  double m_sizeY;        ///< Pad size Y in mm
  double m_thickness;    ///< Diamond thickness in mm
};

} // namespace InDetDD

#endif // BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDIAMONDDESIGN_H
