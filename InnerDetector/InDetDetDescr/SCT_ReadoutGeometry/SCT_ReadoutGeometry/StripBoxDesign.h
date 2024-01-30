/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETREADOUTGEOMETRY_STRIPBOXDESIGN_H
#define INDETREADOUTGEOMETRY_STRIPBOXDESIGN_H

//
//    Upgrade strip barrel sensor with several rows of strips 
//

// Base class
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"

#include "GeoModelKernel/GeoDefinitions.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "TrkSurfaces/RectangleBounds.h"

#include "CLHEP/Geometry/Vector3D.h" // For unused phiMeasureSegment

#include <stdexcept> // For throw stuff
#include <vector>

namespace Trk {
  class SurfaceBounds;
}

namespace InDetDD {
class SiDiodesParameters;

class StripBoxDesign: public SCT_ModuleSideDesign {
public:
    StripBoxDesign(); // Just for access to Axis; or can it be private?

    StripBoxDesign(const SiDetectorDesign::Axis stripDirection,
                   const SiDetectorDesign::Axis thicknessDirection,
                   const double thickness,
                   const int readoutSide,
                   const InDetDD::CarrierType carrier,
                   const int nRows,
                   const int nStrips,
                   const double pitch,
                   const double length,
                   InDetDD::DetectorType detectorType = InDetDD::Undefined,
                   const double zShift=0.0);

    ~StripBoxDesign() = default;

    // Copy constructor and assignment:
    StripBoxDesign(const StripBoxDesign &design);
    StripBoxDesign &operator = (const StripBoxDesign &design);
//
//    I make a one-dimensional strip number to store in the SiCellId, so etaIndex is always 0,
//    even with multi-row detectors. This was an easier way to get digitization than doing 
//    the more natural 2D (strip, row) identifier. The following methods convert 1D to 2D and v.v.
//
    std::pair<int,int> getStripRow(SiCellId id) const final;
    virtual int strip1Dim(int strip, int row) const override;
    int diodes() const; 
    virtual int diodesInRow(const int row) const override; 
//
//    Pure virtual methods in base class:
//
    // Distance to nearest detector active edge (+ve = inside, -ve = outside)
    virtual void distanceToDetectorEdge(const SiLocalPosition &localPosition, double &etaDist,
                                        double &phiDist) const override;

    // check if the position is in active area
    virtual bool inActiveArea(const SiLocalPosition &chargePos, bool checkBondGap = true) const override;

    // Element boundary
    virtual const Trk::SurfaceBounds &bounds() const override;

    // Retrieve the two ends of a "strip"
    virtual std::pair<SiLocalPosition, SiLocalPosition> endsOfStrip(
        const SiLocalPosition &position) const override;

    // Phi-pitch (strip-width). Two names for same thing
    virtual double stripPitch(const SiLocalPosition &localPosition) const override;
    double stripPitch(const SiCellId &cellId) const;
    virtual double stripPitch() const override;
    virtual double phiPitch(const SiLocalPosition &localPosition) const override;
    double phiPitch(const SiCellId &cellId) const;
    virtual double phiPitch() const override;

    // distance to the nearest diode in units of pitch, from 0.0 to 0.5,
    // this method should be fast as it is called for every surface charge
    // in the SCT_SurfaceChargesGenerator
    // an active area check, done in the Generator anyway, is removed here
    virtual double scaledDistanceToNearestDiode(const SiLocalPosition &chargePos) const override;

    // readout or diode id -> position, size
    virtual SiDiodesParameters parameters(const SiCellId &cellId) const override;
    virtual SiLocalPosition localPositionOfCell(const SiCellId &cellId) const override;
    virtual SiLocalPosition localPositionOfCluster(const SiCellId &cellId, int clusterSize) const override;

    // position -> id
    virtual SiCellId cellIdOfPosition(const SiLocalPosition &localPos) const override;
    // id to position
    SiLocalPosition positionFromStrip(const SiCellId &cellId) const;
    virtual SiLocalPosition positionFromStrip(const int stripNumber) const override;

    // row and strip from 1-dim strip number
    virtual int row(int stripId1Dim) const override;
    virtual int strip(int stripId1Dim) const override;

    // Find and fill a vector with all neighbour strips of a given cell
    virtual void neighboursOfCell(const SiCellId &cellId,
                                  std::vector<SiCellId> &neighbours) const override;
    virtual SiCellId cellIdInRange(const SiCellId &) const override;

    // For Strip sensors, readout cell == diode cell. Overload the SCT_ModuleSideDesign
    // member
    virtual SiReadoutCellId readoutIdOfCell(const SiCellId &cellId) const override;
    
    virtual const Amg::Transform3D moduleShift() const override final;

    virtual InDetDD::DetectorType type() const override final;

    // ---------------------------------------------------------------------------------------
    // DEPRECATED at least for Strips
    virtual HepGeom::Vector3D<double> phiMeasureSegment(const SiLocalPosition &position) const override;

    // Method to calculate length of a strip. Which strip??
    virtual double length() const override;

    // Method to calculate average width of a module. What is it used for??
    virtual double width() const override;

    // Method to calculate minimum width of a module
    virtual double minWidth() const override;

    // Method to calculate maximum width of a module
    virtual double maxWidth() const override;

    // Pitch in eta direction Deprecated for strips: it varies in endcap
    virtual double etaPitch() const override;

    // Return true if hit local direction is the same as readout direction.
    virtual bool swapHitPhiReadoutDirection() const override;
    virtual bool swapHitEtaReadoutDirection() const override;

    virtual bool nearBondGap(const SiLocalPosition &, double) const override;

    // ------------------------------------------------------------------------------------------

//
//    Accessors
//
    double pitch(const SiCellId &cellId) const;
    double stripLength(const SiCellId &cellId) const;

    // Give upper and lower boundaries, and length, of dead area
    virtual double deadAreaUpperBoundary() const override;
    virtual double deadAreaLowerBoundary() const override;
    virtual double deadAreaLength() const override;
private:
    int m_nRows;
    int m_nStrips;
    double m_pitch;
    double m_length;
    double m_zShift;
    Trk::RectangleBounds m_bounds;
};

///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////
inline int StripBoxDesign::diodes() const { // Total diodes
    return m_nRows * m_nStrips;
}

inline int StripBoxDesign::diodesInRow(const int /* row */) const {
    return m_nStrips;
}

// Unfortunately SCT introduced the name stripPitch as an alternative to phiPitch so
// everything gets doubled

inline double StripBoxDesign::stripPitch(const SiLocalPosition & /*pos not used */) const {
    return m_pitch;
}

inline double StripBoxDesign::stripPitch(const SiCellId & /*cellId not used */) const {
    return m_pitch;
}

inline double StripBoxDesign::stripPitch() const {
    return m_pitch;
}

inline double StripBoxDesign::phiPitch(const SiLocalPosition & /*pos not used */) const {
    return m_pitch;
}

inline double StripBoxDesign::phiPitch(const SiCellId & /*cellId not used */) const {
    return m_pitch;
}

inline double StripBoxDesign::phiPitch() const {
    return m_pitch;
}

inline bool StripBoxDesign::nearBondGap(const SiLocalPosition &, double) const {
// No bond gap in strip modules
    return false;
}

inline SiReadoutCellId StripBoxDesign::readoutIdOfCell(const SiCellId &cellId) const {
    int strip = cellId.phiIndex(); /* Gets a 1D strip id */
    int row = cellId.etaIndex();   /* is junk or 0 in 1D scheme */

    return SiReadoutCellId(strip, row);
}

inline int StripBoxDesign::row(int stripId1Dim) const {
    return stripId1Dim / m_nStrips; 
}

inline int StripBoxDesign::strip(int stripId1Dim) const {
    return stripId1Dim % m_nStrips; 
}

inline InDetDD::DetectorType StripBoxDesign::type() const{
    return m_detectorType;
}

/// DEPRECATED for StripBoxDesign; no dead area
double StripBoxDesign::deadAreaUpperBoundary() const {
    return 0.;
}

double StripBoxDesign::deadAreaLowerBoundary() const {
    return 0.;
}

double StripBoxDesign::deadAreaLength() const {
    return 0.;
}

bool StripBoxDesign::swapHitPhiReadoutDirection() const {
    return false;
}

bool StripBoxDesign::swapHitEtaReadoutDirection() const {
    return false;
}

} // namespace InDetDD
#endif // INDETREADOUTGEOMETRY_STRIPBOXDESIGN_H
