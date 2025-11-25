/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TRACKING_TRACK_HELPERS_H
#define COLUMNAR_TRACKING_TRACK_HELPERS_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/VectorColumn.h>
#include <ColumnarTracking/TrackDef.h>
#include <TruthUtils/ParticleConstants.h>
#include <xAODTracking/TrackingDetails.h>

namespace columnar
{
  namespace TrackHelpers
  {
    /// @file accessor for variables that have calculations in @ref xAOD::TrackParticle
    ///
    /// Essentially this just copies out the relevant parts of the xAOD
    /// class and makes them look like stand-alone accessors.  The name
    /// of each class is derived from the member function in the xAOD
    /// class.


    template<ContainerIdConcept CI = ContainerId::track,typename CM=ColumnarModeDefault>
    class ChargeAccessor final
    {
      ColumnAccessor<CI,float,CM> m_qOverPAcc;

    public:

      ChargeAccessor (ColumnarTool<CM>& columnarTool)
        : m_qOverPAcc (columnarTool, "qOverP")
      {}

      [[nodiscard]] float operator () (ObjectId<CI,CM> object) const
      {
        return xAOD::TrackingDetails::charge(m_qOverPAcc(object));
      }
    };



    /// this gets the four momentum of a track, given the mass (and
    /// assuming a charge of +/- 1)
    ///
    /// Ideally this should probably be integrated with @ref
    /// MomentumAccessors, except for the two following issues:
    /// * `MomentumAccessors` doesn't allow to pass in a mass, which is
    ///   needed for supporting different particle hypotheses
    /// * `MomentumAccessors` doesn't support "variant" container IDs,
    ///   which muons require for their tracks.
    ///
    /// If these issues get resolved, this class could be merge into
    /// that class, or otherwise it could potentially be set up as some
    /// parallel mechanism.

    template<ContainerIdConcept CI = ContainerId::track,typename CM=ColumnarModeDefault>
    class TrackMomentumAccessors final
    {
      ColumnAccessor<CI,float,CM> m_qOverPAcc;
      ColumnAccessor<CI,float,CM> m_thetaAcc;
      ColumnAccessor<CI,RetypeColumn<double,float>,CM> m_phiAcc;

    public:

      TrackMomentumAccessors (ColumnarTool<CM>& columnarTool)
        : m_qOverPAcc (columnarTool, "qOverP"),
          m_thetaAcc (columnarTool, "theta"),
          m_phiAcc (columnarTool, "phi")
      {}

      using GenVecFourMom_t = xAOD::TrackParticle::GenVecFourMom_t;
      [[nodiscard]] GenVecFourMom_t genvecP4 (ObjectId<CI,CM> trk, double m) const {
        return xAOD::TrackingDetails::genvecP4(m_qOverPAcc(trk), m_thetaAcc(trk), m_phiAcc(trk), m);
      }

      [[nodiscard]] double pt (ObjectId<CI,CM> object, double m) const {
        return genvecP4 (object, m).pt(); }
      [[nodiscard]] double eta (ObjectId<CI,CM> object, double m) const {
        return genvecP4 (object, m).eta(); }
      [[nodiscard]] double phi (ObjectId<CI,CM> object, double m) const {
        return genvecP4 (object, m).phi(); }
    };



    template<ContainerIdConcept CI = ContainerId::track,typename CM=ColumnarModeDefault>
    class DefiningParametersAccessor final
    {
      ColumnAccessor<CI,float,CM> m_d0Acc;
      ColumnAccessor<CI,float,CM> m_z0Acc;
      ColumnAccessor<CI,float,CM> m_phi0Acc;
      ColumnAccessor<CI,float,CM> m_thetaAcc;
      ColumnAccessor<CI,float,CM> m_qOverPAcc;

    public:
      
      DefiningParametersAccessor (ColumnarTool<CM>& columnarTool)
        : m_d0Acc (columnarTool, "d0"),
          m_z0Acc (columnarTool, "z0"),
          m_phi0Acc (columnarTool, "phi"),
          m_thetaAcc (columnarTool, "theta"),
          m_qOverPAcc (columnarTool, "qOverP")
      {}

      [[nodiscard]] xAOD::DefiningParameters_t operator () (ObjectId<CI,CM> trk) const {
        xAOD::DefiningParameters_t tmp;
        tmp << m_d0Acc(trk) , m_z0Acc(trk) , m_phi0Acc(trk) , m_thetaAcc(trk) , m_qOverPAcc(trk);
        return tmp;
      }
    };



    template<ContainerIdConcept CI = ContainerId::track,typename CM=ColumnarModeDefault>
    class DefiningParametersCovAccessor final
    {
      ColumnAccessor<CI,std::vector<float>,CM> m_accCovMatrixDiag;
      ColumnAccessor<CI,std::vector<float>,CM> m_accCovMatrixOffDiag;

    public:

      DefiningParametersCovAccessor (ColumnarTool<CM>& columnarTool)
        : m_accCovMatrixDiag (columnarTool, "definingParametersCovMatrixDiag"),
          m_accCovMatrixOffDiag (columnarTool, "definingParametersCovMatrixOffDiag")
      {}

      [[nodiscard]] const xAOD::ParametersCovMatrix_t operator () (ObjectId<CI,CM> trk) const {
        bool valid = true;
        auto result = xAOD::TrackingDetails::definingParametersCovMatrix( m_accCovMatrixDiag(trk), m_accCovMatrixOffDiag(trk), valid );
        if( !valid ) throw std::runtime_error("DefiningParametersCovAccessor: track covariance matrix not available or improperly formatted");
        return result;
      }
    };
  }
}

#endif
