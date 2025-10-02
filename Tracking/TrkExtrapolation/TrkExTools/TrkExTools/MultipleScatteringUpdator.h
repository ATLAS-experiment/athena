/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MultipleScatteringUpdator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXTOOLS_MULTIPLESCATTERINGUPDATOR_H
#define TRKEXTOOLS_MULTIPLESCATTERINGUPDATOR_H

#include "TrkExInterfaces/IMultipleScatteringUpdator.h"
// Trk
#include "AthenaKernel/IAthRNGSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkEventPrimitives/PropDirection.h"

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"

#include "CLHEP/Random/RandFlat.h"
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"

namespace Trk {

class MaterialProperties;

/**@class MultipleScatteringUpdator

  The Formula used is the highland formula for the projected scattering angle :

    @f$ \theta_{ms} = \frac{13.6MeV}{p}\cdot\sqrt{t/X_{0}}[1 + 0.038\ln(t/X_{0})] @f$

    What is returned is the square of the expectation value of the deflection
    @f$ < (\theta_ms)^2 > = \sigma_ms^2 @f$

    - For electrons the
   @author Andreas.Salzburger@cern.ch
  */
class MultipleScatteringUpdator final
  : public AthAlgTool
  , virtual public IMultipleScatteringUpdator
{

public:
  /** AlgTool like constructor */
  MultipleScatteringUpdator(const std::string&, const std::string&, const IInterface*);

  /**Virtual destructor*/
  virtual ~MultipleScatteringUpdator();

  /** AlgTool initailize method.*/
  virtual StatusCode initialize() override;

  /** Calculate the sigma on theta introduced by multiple scattering,
      according to the RutherFord-Scott Formula
  */
  virtual double sigmaSquare(const MaterialProperties& mat,
                             double p,
                             double pathcorrection,
                             ParticleHypothesis particle = pion,
                             double deltaE = 0.) const override;

private:
  BooleanProperty m_useTrkUtils{this, "UseTrkUtils", true,
      "use eloss parametrisation from TrkUtils MaterialInterAction.h"};
  BooleanProperty m_log_include{this, "MultipleScatteringLogarithmicTermOn", true,
    "boolean switch to include log term"};
  BooleanProperty m_gaussianMixture{this, "GaussianMixtureModel", false,
    "mainly for Fatras"};
  BooleanProperty m_optGaussianMixtureG4{this, "G4OptimisedGaussianMixtureModel", true,
    "modifies the Fruehwirth/Regler model to fit with G4"};

  //========== used for Gaussian mixture model =================================================
  /** Random Generator service  */
  ServiceHandle<IAthRNGSvc> m_rndGenSvc{this, "RandomNumberService", "AthRNGSvc",
    "Name of the random number service"};
  /** Random engine  */
  ATHRNG::RNGWrapper* m_rngWrapper = nullptr;
  StringProperty m_randomEngineName{this, "RandomStreamName", "TrkExRnd",
    "Name of the random number stream"};
};

} // end of namespace

#endif // TRKEXTOOLS_MULTIPLESCATTERING_H

