/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ParametersBase.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKPARAMETERSBASE_PARAMETERSBASE_H
#define TRKPARAMETERSBASE_PARAMETERSBASE_H
//
#include "TrkParametersBase/ParametersCommon.h"
//system
#include <memory>
#include <type_traits>
#include <optional>
#include <iosfwd>

class MsgStream;

template<typename T>
class TrackParametersCovarianceCnv;
class TrackParametersCnv_p2;
class MeasuredPerigeeCnv_p1;
template<class SURFACE_CNV, class ATA_SURFACE>
class AtaSurfaceCnv_p1;

namespace Trk {
class Surface;
class MaterialEffectsEngine;


/**
   @class ParametersBase

   The base class for neutral and charged Track parameters.
   It represents the free state of a trajectory, represented by
   the track parameters.
   The position and the momentum are both given in the tracking
   reference frame.

   @tparam DIM number of track parameters (usually 5)
   @tparam T   charge of track (either <tt>Trk::Charged</tt> or
   <tt>Trk::Neutral</tt>)

   The relevant allowed aliases and specialization are under
   TrkParameters and TrkNeutralParameters and not in this package.

   @author Andreas.Salzburger@cern.ch
   @author Christos Anastopoulos (Athena MT modifications)
*/

template<int DIM, class T>
class ParametersBase : public ParametersCommon<DIM,T>
{
public:
  /** virtual Destructor */
  virtual ~ParametersBase() = default;

  /** Returns the charge */
  double charge() const;

  /** Access method for the position */
  const Amg::Vector3D& position() const;

  /** Access method for the momentum */
  const Amg::Vector3D& momentum() const;

  /** Access method for transverse momentum */
  double pT() const;

  /** Access method for pseudorapidity - from momentum */
  double eta() const;

  /** equality operator */
  virtual bool operator==(const ParametersBase<DIM, T>&) const;

  /** Test to see if there's a not null surface ptr. */
  virtual bool hasSurface() const override = 0;

  /** Access to the Surface associated to the Parameters*/
  virtual const Surface& associatedSurface() const override = 0;

  /** Return the measurement frame - this is needed for alignment, in
     particular for StraightLine and Perigee Surface
      - the default implementation is the RotationMatrix3D of the
     transform */
  virtual Amg::RotationMatrix3D measurementFrame() const override = 0;

  /** clone method for polymorphic deep copy
       @return new object copied from the concrete type of this object.*/
  virtual ParametersBase<DIM, T>* clone() const override = 0;

  /** clone method for polymorphic deep copy returning unique_ptr; it is not overriden,
       but uses the existing clone method.
       @return new object copied from the concrete type of this object.*/
  std::unique_ptr<ParametersBase<DIM, T>> uniqueClone() const{
    return std::unique_ptr<ParametersBase<DIM, T>>(this->clone());
  }

  /** Return the ParametersType enum */
  constexpr virtual ParametersType type() const override = 0;

  /** Returns the Surface Type enum for the surface used
   * to define the derived class*/
  constexpr virtual SurfaceType surfaceType() const override = 0;

  /** Dumps relevant information about the track parameters into the ostream */
  virtual MsgStream& dump(MsgStream& out) const;
  virtual std::ostream& dump(std::ostream& out) const;

protected:
  /*
   * This is an abstract class and we can not instantiate objects directly.
   * In the other hand derived classed can use ctors
   */
  ParametersBase() = default;
  ParametersBase(ParametersBase&&) noexcept = default;
  ParametersBase& operator=(ParametersBase&&) noexcept  = default;
  ParametersBase(const ParametersBase&) = default;
  ParametersBase& operator=(const ParametersBase&) = default;

  /* Helper ctors for derived classes*/
  ParametersBase(const AmgVector(DIM) parameters,
                 std::optional<AmgSymMatrix(DIM)>&& covariance,
                 const T chargeDef);

  ParametersBase(std::optional<AmgSymMatrix(DIM)>&& covariance);

  ParametersBase(const AmgVector(DIM) & parameters,
                 std::optional<AmgSymMatrix(DIM)>&& covariance = std::nullopt);

  virtual void updateParametersHelper(const AmgVector(DIM) &) override = 0;
  /*
   * Add dependent names into scope
   */
  using ParametersCommon<DIM, T>::m_parameters;
  using ParametersCommon<DIM, T>::m_covariance;
  using ParametersCommon<DIM, T>::m_chargeDef;
  Amg::Vector3D m_position; //!< point on track
  Amg::Vector3D m_momentum; //!< momentum at this point on track
};

/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
template<int DIM, class T>
MsgStream&
operator<<(MsgStream& sl, const Trk::ParametersBase<DIM, T>& tp);

template<int DIM, class T>
std::ostream&
operator<<(std::ostream& sl, const Trk::ParametersBase<DIM, T>& tp);

} // end of namespace Trk

#include "TrkParametersBase/ParametersBase.icc"

#endif
