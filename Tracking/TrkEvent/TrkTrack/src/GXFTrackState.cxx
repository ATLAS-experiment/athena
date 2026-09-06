/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// GXFTrackState.cxx
// see header file for documentation.
//////////////////////////////////////////////////////////////////
//
#include "TrkTrack/GXFTrackState.h"
#include "TrkTrack/GXFMaterialEffects.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkEventPrimitives/TransportJacobian.h"
#include "TrkSurfaces/Surface.h"
#include "TrkEventPrimitives/SurfaceConsistencyCheck.h"
#include "TrkEventPrimitives/unique_clone.h"
#include <utility>

namespace Trk {
  GXFTrackState::GXFTrackState(const GXFTrackState & rhs):
    m_measurement(rhs.m_measurement != nullptr ? std::unique_ptr<const MeasurementBase>(rhs.m_measurement->clone()) : nullptr), 
    m_tsType(rhs.m_tsType), 
    m_trackpar(std::unique_ptr<const TrackParameters>(rhs.m_trackpar != nullptr ? rhs.m_trackpar->clone() : nullptr)),
    m_materialEffects(rhs.m_materialEffects != nullptr ? std::make_unique<GXFMaterialEffects>(*rhs. m_materialEffects) : nullptr),
    m_derivs(rhs.m_derivs),
    m_covariancematrix(rhs.m_covariancematrix),
    m_covariance_set(rhs.m_covariance_set),
    m_fitqual(rhs.m_fitqual), 
    m_sinstereo(rhs.m_sinstereo),
    m_mType(rhs.m_mType), 
    m_recalib(rhs.m_recalib),
    m_measphi(rhs.m_measphi) 
  {
    for (int i = 0; i < 5; i++) {
      m_measerror[i] = rhs.m_measerror[i];
      for (int j = 0; j < 5; j++) {
        m_jacobian(i, j) = rhs.m_jacobian(i, j);
      }
    }
  }

  GXFTrackState::GXFTrackState(
    std::unique_ptr<const MeasurementBase> measurement,
    std::unique_ptr<const TrackParameters> trackpar
  ):
    m_measurement(std::move(measurement)),
    m_trackpar(std::move(trackpar)),
    m_materialEffects(nullptr), 
    m_jacobian {}, 
    m_derivs(), 
    m_covariancematrix(),
    m_covariance_set(false),
    m_fitqual(),
    m_sinstereo(0), 
    m_mType(TrackState::unidentified), 
    m_recalib(false),
    m_measphi(false) {
    setStateType(TrackStateOnSurface::Measurement);
    m_measerror[0] = m_measerror[1] = m_measerror[2] = m_measerror[3] = m_measerror[4] = -1;
  }

  GXFTrackState::GXFTrackState(
    std::unique_ptr<const TrackParameters> trackpar,
    TrackStateOnSurface::TrackStateOnSurfaceType tsType
  ):
    m_measurement(nullptr),
    m_trackpar(std::move(trackpar)),
    m_materialEffects(nullptr), 
    m_jacobian {}, 
    m_derivs(), 
    m_covariancematrix(), 
    m_covariance_set(false),
    m_fitqual(),
    m_sinstereo(0), 
    m_mType(TrackState::unidentified), 
    m_recalib(false),
    m_measphi(false) 
  {
    setStateType(tsType);
    m_measerror[0] = m_measerror[1] = m_measerror[2] = m_measerror[3] = m_measerror[4] = -1;
  }

  GXFTrackState::GXFTrackState(
    std::unique_ptr<GXFMaterialEffects> mef,
    std::unique_ptr<const TrackParameters> trackpar
  ):
    m_measurement(nullptr),
    m_trackpar(std::move(trackpar)),
    m_materialEffects(std::move(mef)), 
    m_jacobian {}, 
    m_derivs(), 
    m_covariancematrix(),
    m_covariance_set(false), 
    m_fitqual(),
    m_sinstereo(0), 
    m_mType(TrackState::unidentified), 
    m_recalib(false),
    m_measphi(false) 
  {
    m_measerror[0] = m_measerror[1] = m_measerror[2] = m_measerror[3] = m_measerror[4] = -1;

    if (m_materialEffects && (m_materialEffects->sigmaDeltaTheta() == 0)) {
      setStateType(TrackStateOnSurface::BremPoint);
    } else {
      setStateType(TrackStateOnSurface::Scatterer);
    }
  }
  
  bool
  GXFTrackState::isSane() const{
    const GXFMaterialEffects* eff = m_materialEffects.get();
    if (not consistentSurfaces(m_measurement.get(),m_trackpar.get(), eff) ){
      std::cerr << "GXFTrackState::isSane. With :" << '\n';
      std::cerr << "Types : " << m_tsType.to_string() << '\n';
      std::cerr << "Surfaces differ! " << std::endl;
      if (m_trackpar) {
        std::cerr << "ParamSurf: [" << &(m_trackpar->associatedSurface())
                  << "] " << m_trackpar->associatedSurface() << std::endl;
      }
      if (m_measurement) {
        std::cerr << "measSurf: [" << &(m_measurement->associatedSurface())
                  << "] " << m_measurement->associatedSurface()
                  << std::endl;
      }
      if (m_materialEffects) {
        std::cerr << "matSurf: ["
                  << &(m_materialEffects->associatedSurface()) << "] "
                  << m_materialEffects->associatedSurface() << std::endl;
      }
      return false;
    }
  return true;
  }

  void GXFTrackState::setMeasurement(std::unique_ptr<const MeasurementBase> meas) {
    m_measurement = std::move(meas);
    m_recalib = true;
  }

  const MeasurementBase *GXFTrackState::measurement(void) {
    return m_measurement.get();
  }  

  void GXFTrackState::setTrackParameters(std::unique_ptr<const TrackParameters> par) {
    m_trackpar = std::move(par);
  }

  void GXFTrackState::setJacobian(TransportJacobian & jac) {
    m_jacobian = jac;
  }

  void
    GXFTrackState::setDerivatives(Amg::MatrixX & deriv) {
    m_derivs = deriv;
  }

  double *GXFTrackState::measurementErrors() {
    return m_measerror;
  }

  void
    GXFTrackState::setMeasurementErrors(const double *measerror) {
    m_measerror[0] = measerror[0];
    m_measerror[1] = measerror[1];
    m_measerror[2] = measerror[2];
    m_measerror[3] = measerror[3];
    m_measerror[4] = measerror[4];
  }

  double
    GXFTrackState::sinStereo() const {
    return m_sinstereo;
  }

  void
    GXFTrackState::setSinStereo(double sinstereo) {
    m_sinstereo = sinstereo;
  }

  const Surface &
  GXFTrackState::associatedSurface() const {
    if (m_measurement != nullptr) {
      return m_measurement->associatedSurface();
    } if (m_trackpar != nullptr) {
      return m_trackpar->associatedSurface();
    } if (m_materialEffects != nullptr) {
      return m_materialEffects->associatedSurface();
    } else {
     throw std::runtime_error("GXFTrackState::associatedSurface: None of measurement, track parameters or material effects are non-null pointers");
    }
    
  }

  void
    GXFTrackState::setTrackCovariance(AmgSymMatrix(5) * covmat) {
    if (covmat == nullptr) {
      m_covariance_set = false;
    } else {
      m_covariance_set = true;
      m_covariancematrix = *covmat;
    }
  }

  void GXFTrackState::setFitQuality(FitQualityOnSurface fitqual) {
    m_fitqual = fitqual;
  }

  const FitQualityOnSurface GXFTrackState::fitQuality(void) {
    return m_fitqual;
  }

  int
    GXFTrackState::numberOfMeasuredParameters() {
    int nmeas = 0;

    if (getStateType(TrackStateOnSurface::Measurement) || getStateType(TrackStateOnSurface::Outlier)) {
      for (double i : m_measerror) {
        if (i > 0) {
          nmeas++;
        }
      }
    }

    return nmeas;
  }

  Amg::Vector3D GXFTrackState::position() {
    if (m_trackpar != nullptr) {
      return m_trackpar->position();
    } if (m_measurement != nullptr) {
      return m_measurement->globalPosition();
    }
    // return surface()->center();  // Better than nothing...
    return m_globpos;
  }

  void
    GXFTrackState::setPosition(Amg::Vector3D & pos) {
    m_globpos = pos;
  }

  bool
    GXFTrackState::measuresPhi() const {
    return m_measphi;
  }

  void
    GXFTrackState::setMeasuresPhi(bool measphi) {
    m_measphi = measphi;
  }

  void
    GXFTrackState::setRecalibrated(bool isrecal) {
    m_recalib = isrecal;
  }

  bool GXFTrackState::hasTrackCovariance(void) const {
    return m_covariance_set;
  }

  void GXFTrackState::zeroTrackCovariance(void) {
    m_covariance_set = true;
    m_covariancematrix.setZero();
  }
  void GXFTrackState::resetTrackCovariance(){
    if (m_covariance_set) {
      setTrackCovariance(nullptr);
    }
  }

  void GXFTrackState::resetStateType(TrackStateOnSurface::TrackStateOnSurfaceType t, bool v) {
    m_tsType.reset();
    setStateType(t, v);
  }

  void GXFTrackState::setStateType(TrackStateOnSurface::TrackStateOnSurfaceType t, bool v) {
    m_tsType[t] = v;
  }

  bool GXFTrackState::getStateType(TrackStateOnSurface::TrackStateOnSurfaceType t) const {
    return m_tsType.test(t);
  }

  std::optional<std::vector<std::unique_ptr<TrackParameters>>> & GXFTrackState::getHoles(void) {
    return m_holes;
  }

  void GXFTrackState::setHoles(std::vector<std::unique_ptr<TrackParameters>> && v) {
    m_holes = std::move(v);
  }
  
  std::unique_ptr<const TrackStateOnSurface>
  GXFTrackState::trackStateOnSurface() const{
    std::unique_ptr<TrackParameters> trackpar = unique_clone(m_trackpar.get());
    std::unique_ptr<MeasurementBase> measurement = unique_clone(m_measurement.get());

    FitQualityOnSurface fitQual =m_fitqual;

    std::unique_ptr<MaterialEffectsBase> mateff = nullptr;
    std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
    if ((m_materialEffects != nullptr) && (m_tsType.test(TrackStateOnSurface::Scatterer) or m_tsType.test(TrackStateOnSurface::BremPoint))) {
      if (m_tsType.test(TrackStateOnSurface::Scatterer)) {
        typePattern.set(TrackStateOnSurface::Scatterer);
      }
      
      if (m_materialEffects->sigmaDeltaE() > 0) {
        if (m_materialEffects->sigmaDeltaTheta() == 0) {
          typePattern.set(TrackStateOnSurface::CaloDeposit);
        } else {
          typePattern.set(TrackStateOnSurface::BremPoint);
        }
      }
      
      if (mateff == nullptr) {
        mateff = m_materialEffects->makeMEOT();
      }
    } else {
      if (m_tsType.test(TrackStateOnSurface::Measurement)) {
        typePattern.set(TrackStateOnSurface::Measurement);        
        if ((fitQual) && (fitQual.chiSquared() > 1.e5 || fitQual.chiSquared() < 0)) {
          double newchi2 = 0;
          int ndf = fitQual.numberDoF();
          
          if (fitQual.chiSquared() < 0) {
            newchi2 = 0;
          } else if (fitQual.chiSquared() > 1.e5) {
            newchi2 = 1.e5;
          }
          
          fitQual = FitQualityOnSurface(newchi2, ndf);
        }
      } else if (m_tsType.test(TrackStateOnSurface::Outlier)) {
        typePattern.set(TrackStateOnSurface::Outlier);
      } else if (m_tsType.test(TrackStateOnSurface::Perigee)) {
        typePattern.set(TrackStateOnSurface::Perigee);
      }
    }
    return std::make_unique<const TrackStateOnSurface>(
      fitQual,
      std::move(measurement),
      std::move(trackpar),
      std::move(mateff),
      typePattern);
  }
}
