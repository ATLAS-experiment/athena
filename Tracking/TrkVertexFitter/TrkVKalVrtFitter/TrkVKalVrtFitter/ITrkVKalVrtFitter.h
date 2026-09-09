/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// ITrkVKalVrtFitter.h  - old VKalVrtInterface
//
//  Old interface of VKalVrt.
//  Fully implemented also in Reconstruction/VKalVrt/VKalVrtFitSvc
//
//---------------------------------------------------------------
#ifndef TRKVKALVRTFITTER_ITRKVKALVRTFITTER_H
#define TRKVKALVRTFITTER_ITRKVKALVRTFITTER_H

#include "GaudiKernel/IAlgTool.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "EventPrimitives/EventPrimitives.h"
#include  "xAODTracking/TrackParticleFwd.h"
#include  "xAODTracking/NeutralParticleFwd.h"
#include  "TrkVKalVrtFitter/IVKalState.h"

#include <vector>
#include <memory>
#include <span>

class EventContext;

namespace Trk{

class IExtrapolator;
class IVKalState;
class Track;

  class ITrkVKalVrtFitter : virtual public IAlgTool {
    public:
      DeclareInterfaceID(ITrkVKalVrtFitter, 1, 0);

      virtual std::unique_ptr<IVKalState> makeState(const EventContext& ctx) const = 0;

      virtual StatusCode VKalVrtFit(
        const std::vector<const xAOD::TrackParticle*>& listC,
        const std::vector<const xAOD::NeutralParticle*>& listN,
        Amg::Vector3D& Vertex,
        TLorentzVector& Momentum,
        long int& Charge,
        std::vector<double>& ErrorMatrix,
        std::vector<double>& Chi2PerTrk,
        std::vector<std::vector<double>>& TrkAtVrt,
        double& Chi2,
        IVKalState& istate,
        bool ifCovV0 = false) const = 0;

      virtual StatusCode VKalVrtFit(const std::vector<const Perigee*>& list,
                                    Amg::Vector3D& Vertex,
                                    TLorentzVector& Momentum,
                                    long int& Charge,
                                    std::vector<double>& ErrorMatrix,
                                    std::vector<double>& Chi2PerTrk,
                                    std::vector<std::vector<double>>& TrkAtVrt,
                                    double& Chi2,
                                    IVKalState& istate,
                                    bool ifCovV0 = false) const = 0;

      virtual StatusCode VKalVrtFit(
        const std::vector<const TrackParameters*>& listC,
        const std::vector<const NeutralParameters*>& listN,
        Amg::Vector3D& Vertex,
        TLorentzVector& Momentum,
        long int& Charge,
        std::vector<double>& ErrorMatrix,
        std::vector<double>& Chi2PerTrk,
        std::vector<std::vector<double>>& TrkAtVrt,
        double& Chi2,
        IVKalState& istate,
        bool ifCovV0 = false) const = 0;

      //------
      virtual StatusCode VKalVrtCvtTool(const Amg::Vector3D& Vertex,
                                        const TLorentzVector& Momentum,
                                        const std::vector<double>& CovVrtMom,
                                        const long int& Charge,
                                        std::span<double, 5> Perigee,
                                        std::span<double, 15> CovPerigee,
                                        IVKalState& istate) const = 0;
      //.........................................................................................

      virtual StatusCode VKalVrtFitFast(
        std::span<const xAOD::TrackParticle* const> list,
        Amg::Vector3D& Vertex,
        IVKalState& istate) const = 0;

      virtual StatusCode VKalVrtFitFast(
        const std::vector<const TrackParameters*>& list,
        Amg::Vector3D& Vertex,
        IVKalState& istate) const = 0;
      //.........................................................................................

      virtual std::unique_ptr<Perigee>
	CreatePerigee(const std::span<const double, 5> VKPerigee,
                const std::span<const double, 15> VKCov,
		      IVKalState& istate) const = 0;

      virtual StatusCode VKalGetTrkWeights(std::vector<double>& Weights,
                                           const IVKalState& istate) const = 0;

      virtual StatusCode VKalGetFullCov(long int, std::vector<double>& CovMtx,
                                        IVKalState& istate, bool = false) const =0;

      virtual StatusCode VKalGetMassError(double& Mass, double& MassError,
                                          const IVKalState& istate) const =0;

      virtual void setApproximateVertex(double,double,double,
                                        IVKalState& istate) const =0;
      virtual void setMassForConstraint(double,
                                        IVKalState& istate) const =0;

      virtual void setMassForConstraint(double, std::span<const int>,
                                        IVKalState& istate) const =0;

      virtual void setRobustness(int, IVKalState& istate) const =0;

      virtual void setRobustScale(double, IVKalState& istate) const =0;

      virtual void setCnstType(int, IVKalState& istate) const =0;

      virtual void setVertexForConstraint(const xAOD::Vertex &,
                                          IVKalState& istate) const=0;

      virtual void setVertexForConstraint(double,double,double,
                                          IVKalState& istate) const =0;

      virtual void setCovVrtForConstraint(double,double,double,
                                          double,double,double,
                                          IVKalState& istate) const=0;

      virtual void setMassInputParticles( const std::vector<double>&,
                                          IVKalState& istate) const=0;
//----------------------------------------------------------------------------------------------------

      virtual double VKalGetImpact(const xAOD::TrackParticle*,
                                   const Amg::Vector3D& Vertex,
                                   const long int Charge,
                                   std::vector<double>& Impact,
                                   std::vector<double>& ImpactError,
                                   IVKalState& istate) const = 0;

      virtual double VKalGetImpact(const Perigee*,
                                   const Amg::Vector3D& Vertex,
                                   const long int Charge,
                                   std::vector<double>& Impact,
                                   std::vector<double>& ImpactError,
                                   IVKalState& istate) const = 0;

      virtual double VKalGetImpact(const EventContext& ctx,
                                   const xAOD::TrackParticle*,
                                   const Amg::Vector3D& Vertex,
                                   const long int Charge,
                                   std::vector<double>& Impact,
                                   std::vector<double>& ImpactError) const = 0;

      virtual double VKalGetImpact(const EventContext& ctx,
                                   const Perigee*,
                                   const Amg::Vector3D& Vertex,
                                   const long int Charge,
                                   std::vector<double>& Impact,
                                   std::vector<double>& ImpactError) const = 0;

      //----------------------------------------------------------------------------------------------------
   };

} //end of namespace

#endif
