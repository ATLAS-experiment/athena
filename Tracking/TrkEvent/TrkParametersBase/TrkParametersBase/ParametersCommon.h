/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/* @brief ParametersCommon common base class
 * for  ParametersT, CurvilinearParameters and PatternTrackParameters
 */

#ifndef TRKPARAMETERSCOMMON_PARAMETERSCOMMON_H
#define TRKPARAMETERSCOMMON_PARAMETERSCOMMON_H
//
#include "TrkParametersBase/Charged.h"
#include "TrkParametersBase/Neutral.h"
// Amg
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkEventPrimitives/SurfaceTypes.h"
//
#include <optional>
#include <limits>

namespace Trk {
class Surface;

/**
   @enum ParametersType
   Enum to avoid dynamic cast for different parameter types.
*/
enum ParametersType { AtaSurface = 0, Curvilinear = 1, Pattern = 2 };

namespace InvalidParam {
constexpr double INVALID = std::numeric_limits<double>::quiet_NaN();
constexpr double INVALID_P(10e9);
constexpr double INVALID_QOP(10e-9);
}

/**
   @class ParametersCommon
   Common class for neutral, charged and Pattern Track
   Parameters
   @author Christos Anastopoulos
*/

template <int DIM, class T>
class ParametersCommon {
 public:
  static_assert((std::is_same<T, Trk::Charged>::value ||
                 std::is_same<T, Trk::Neutral>::value),
                "Parameters must be Charged or Neutral");
  static constexpr int dim = DIM;

  /** Access methods for the parameters */
  const AmgVector(DIM) & parameters() const;
  AmgVector(DIM) & parameters();

  /** Access method for the covariance matrix - returns nullptr if no covariance
   * matrix is given */
  const AmgSymMatrix(DIM) * covariance() const;
  AmgSymMatrix(DIM) * covariance();

  /** Returns true if Charged or false if Neutral
   */
  constexpr bool isCharged() const;
  /** Access method for the local coordinates, \f$(loc1,loc2)\f$
      local parameter definitions differ for each surface type. */
  Amg::Vector2D localPosition() const;

  /** set parameters*/
  void setParameters(const AmgVector(DIM) & param);

  /** set covariance */
  void setCovariance(const AmgSymMatrix(DIM) & cov);

  /** Update parameters  and covariance , passing covariance by ref. A
   * covariance is created if one does not exist.  Otherwise in place update
   * occurs via assignment.
   *
   * Derived classes override the
   * implementation via updateParametersHelper
   * as this could possibly lead to updating
   * other data members
   */
  void updateParameters(const AmgVector(DIM) &, const AmgSymMatrix(DIM) &);

  /** Update parameters.
   * Derived classes override the
   * implementation via updateParametersHelper
   * as this could possibly lead to updating
   * other data members
   */
  void updateParameters(const AmgVector(DIM) &);

  /** Test to see if there's a not null surface ptr. */
  virtual bool hasSurface() const = 0;

  /** Access to the Surface associated to the Parameters*/
  virtual const Surface& associatedSurface() const = 0;

  /** Return the measurement frame - this is needed for alignment, in
     particular for StraightLine and Perigee Surface
      - the default implementation is the RotationMatrix3D of the
     transform */
  virtual Amg::RotationMatrix3D measurementFrame() const = 0;

  /** clone method for polymorphic deep copy
       @return new object copied from the concrete type of this object.*/
  virtual ParametersCommon<DIM, T>* clone() const = 0;

  /** Return the ParametersType enum */
  constexpr virtual ParametersType type() const = 0;

  /** Returns the Surface Type enum for the surface used
   * to define the derived class*/
  virtual SurfaceType surfaceType() const = 0;

  /** virtual Destructor */
  virtual ~ParametersCommon() = default;

 protected:
  /*
   * This is an abstract class and we can not instantiate objects directly.
   * In the other hand derived classed can use ctors
   */
  ParametersCommon() = default;
  ParametersCommon(ParametersCommon&&) noexcept = default;
  ParametersCommon& operator=(ParametersCommon&&) noexcept = default;
  ParametersCommon(const ParametersCommon&) = default;
  ParametersCommon& operator=(const ParametersCommon&) = default;
  /* Helper ctors for derived classes*/
  ParametersCommon(const AmgVector(DIM) parameters,
                 std::optional<AmgSymMatrix(DIM)>&& covariance,
                 const T chargeDef);

  ParametersCommon(std::optional<AmgSymMatrix(DIM)>&& covariance);

  ParametersCommon(const AmgVector(DIM) & parameters,
                 std::optional<AmgSymMatrix(DIM)>&& covariance = std::nullopt);


  /* Helper implementing the specific per derived class logic for
   * the update of parameters*/
  virtual void updateParametersHelper(const AmgVector(DIM) &) = 0;

  //!< contains the n parameters
  AmgVector(DIM) m_parameters;
  //!< contains the n x n covariance matrix
  std::optional<AmgSymMatrix(DIM)> m_covariance = std::nullopt;
  //!< charge definition for this track
  T m_chargeDef{};  //!< charge definition for this track
};
}  // end of namespace Trk

#include "TrkParametersBase/ParametersCommon.icc"

#endif
