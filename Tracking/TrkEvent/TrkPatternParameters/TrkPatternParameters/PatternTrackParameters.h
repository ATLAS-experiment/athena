/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class PatternTrackParameters
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for pattern track parameters
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 09/08/2006 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef PatternTrackParameters_H
#define PatternTrackParameters_H

#include "TrkParametersBase/ParametersCommon.h"
#include "TrkParametersBase/Charged.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkSurfaces/Surface.h"
#include "TrkPatternParameters/NoiseOnSurface.h"
#include "CxxUtils/CachedValue.h"
#include <cmath>
#include <iosfwd>

class MsgStream;

namespace Trk {

  class PlaneSurface       ;
  class StraightLineSurface;
  class DiscSurface        ;
  class CylinderSurface    ;
  class PerigeeSurface     ;
  class ConeSurface        ;

  class PatternTrackParameters final : public ParametersCommon<5, Trk::Charged>{
    public:
      PatternTrackParameters();
      PatternTrackParameters(const PatternTrackParameters&);
      PatternTrackParameters& operator  = (const PatternTrackParameters&);
      PatternTrackParameters(PatternTrackParameters&&) noexcept = default;
      PatternTrackParameters& operator  = (PatternTrackParameters&&) noexcept = default;
      virtual ~PatternTrackParameters() = default;

      // Main methods
      bool             iscovariance      ()     const {return   m_covariance != std::nullopt ;}
      double           sinPhi            ()     const;
      double           cosPhi            ()     const;
      double           sinTheta          ()     const;
      double           cosTheta          ()     const;
      double           cotTheta          ()     const;
      void             changeDirection   ()          ;
      double           transverseMomentum()     const;

      // Methods from ParametersCommon
      virtual const Surface&  associatedSurface ()  const override final;
      Amg::Vector3D position() const;
      Amg::Vector3D momentum() const;
      double charge() const;
      virtual bool hasSurface() const override final;
      virtual Amg::RotationMatrix3D measurementFrame() const override final;
      virtual PatternTrackParameters * clone() const override final;
      constexpr virtual ParametersType type() const override final;
      virtual SurfaceType surfaceType() const override final;
      virtual void updateParametersHelper(const AmgVector(5) &) override final;

      // set methods
      void setParameters              (const Surface*,const double*              );
      void setCovariance              ( const double*);
      void setParametersWithCovariance(const Surface*,const double*,const double*);
      void setParametersWithCovariance(const Surface*,const double*,const AmgSymMatrix(5)&);

      // Convertors
      std::unique_ptr<ParametersBase<5, Trk::Charged>> convert(bool) const;
      bool production(const ParametersBase<5, Trk::Charged>*);

      // Init methods
      void diagonalization (double);
      bool initiate (PatternTrackParameters&, const Amg::Vector2D&,const Amg::MatrixX&);

      // Add or remove noise
      void addNoise   (const NoiseOnSurface&,PropDirection);
      void removeNoise(const NoiseOnSurface&,PropDirection);

      // Print
      std::ostream& dump(std::ostream&) const;
      MsgStream&    dump(MsgStream&   ) const;

    protected:

      // Protected data
      SurfaceUniquePtrT<const Surface> m_surface;

      ///////////////////////////////////////////////////////////////////
      // Comments
      // m_surface is pointer to associated surface
      // m_parameters[ 0] - 1 local coordinate
      // m_parameters[ 1] - 2 local coordinate
      // m_parameters[ 2] - Azimuthal angle
      // m_parameters[ 3] - Polar     angle
      // m_parameters[ 4] - charge/Momentum
      // m_covariance is the covariance matrix
      ///////////////////////////////////////////////////////////////////


      // Protected methods
      Amg::Vector3D localToGlobal(const PlaneSurface       *) const;
      Amg::Vector3D localToGlobal(const StraightLineSurface*) const;
      Amg::Vector3D localToGlobal(const DiscSurface        *) const;
      Amg::Vector3D localToGlobal(const CylinderSurface    *) const;
      Amg::Vector3D localToGlobal(const PerigeeSurface     *) const;
      Amg::Vector3D localToGlobal(const ConeSurface        *) const;

      Amg::Vector3D calculatePosition(void) const;
      Amg::Vector3D calculateMomentum(void) const;
      double        absoluteMomentum() const;

      private:
      std::string to_string() const;
    };

  /////////////////////////////////////////////////////////////////////////////////
  // Overload operator
  /////////////////////////////////////////////////////////////////////////////////

  std::ostream& operator << (std::ostream&,const PatternTrackParameters&);
  MsgStream& operator    << (MsgStream&, const PatternTrackParameters& );

} // end of name space Trk

#include "TrkPatternParameters/PatternTrackParameters.icc"

#endif // PatternTrackParameters
