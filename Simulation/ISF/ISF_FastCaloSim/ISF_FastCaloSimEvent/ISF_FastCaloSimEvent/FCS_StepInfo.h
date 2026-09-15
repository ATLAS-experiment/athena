/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMEVENT_ISF_FCS_STEPINFO_H
#define ISF_FASTCALOSIMEVENT_ISF_FCS_STEPINFO_H

#include <vector>

// CLHEP include for Hep3Vector
#include "CLHEP/Vector/ThreeVector.h"

#include "TileSimEvent/TileHit.h"

// Namespace for the G4 step related classes
namespace ISF_FCS_Parametrization {

/**
 * @brief Transfers Geant4 hit position, energy, and time to FastCaloSim.
 *
 *  @author Wolfgang Ehrenfeld, University of Hamburg, Germany
 *  @author Sasha Glazov, DESY Hamburg, Germany
 *  @author Zdenek Hubacek, CERN
 *
 */

class FCS_StepInfo : public TileHit {

public:
  FCS_StepInfo() : m_pos(), m_valid(false), m_detector(-1) {}

  FCS_StepInfo(CLHEP::Hep3Vector l_vec, Identifier l_cell, double l_energy,
               double l_time, bool l_valid, int l_detector)
      : TileHit(l_cell, l_energy, l_time), m_pos(l_vec), m_valid(l_valid),
        m_detector(l_detector) {}

  void setP(const CLHEP::Hep3Vector &p) { m_pos = p; }
  void setX(double x) { m_pos.setX(x); }
  void setY(double y) { m_pos.setY(y); }
  void setZ(double z) { m_pos.setZ(z); }
  void setValid(bool flag) { m_valid = flag; }
  void setDetector(int det) { m_detector = det; }

  CLHEP::Hep3Vector position() const { return m_pos; }
  double x() const { return m_pos.x(); }
  double y() const { return m_pos.y(); }
  double z() const { return m_pos.z(); }
  bool valid() const { return m_valid; }
  int detector() const { return m_detector; }

  //! Return the squared spatial distance to another hit.
  double diff2(const FCS_StepInfo &other) const;

  //! Merge another hit using absolute-energy weights.
  FCS_StepInfo &operator+=(const FCS_StepInfo &other);

private:
  // data members
  CLHEP::Hep3Vector m_pos; //!< spatial position
  bool m_valid;
  int m_detector;
};

}  // namespace ISF_FCS_Parametrization

#endif  // ISF_FASTCALOSIMEVENT_ISF_FCS_STEPINFO_H
