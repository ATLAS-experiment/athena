/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_Fatras_McEnergyLossUpdator_H
#define ISF_Fatras_McEnergyLossUpdator_H

// GaudiKernel & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CxxUtils/checker_macros.h"

// Trk
#include "TrkExInterfaces/IEnergyLossUpdator.h"
#include "TrkEventPrimitives/PropDirection.h"

namespace Trk{
  class MaterialProperties;
  class EnergyLoss;
}

namespace iFatras{


  /** @class McEnergyLossUpdator

      Updator for a eloss of a track on a Trk::Layer,
      it extends the IEnergyLossUpdtor interface

      @author Tom.Atkinson@cern.ch, Andreas.Salzburger@cern.ch
  */

  class ATLAS_NOT_THREAD_SAFE McEnergyLossUpdator : public extends<AthAlgTool, Trk::IEnergyLossUpdator> {

  public:

    /** Constructor with AlgTool parameters */
    McEnergyLossUpdator( const std::string&, const std::string&, const IInterface* );

    /** Destructor */
    virtual ~McEnergyLossUpdator() = default;

    /** AlgTool initialise method */
    virtual StatusCode initialize() override;

    /** IEnergyLossUpdator public method to compute dEdX */
    virtual
      double dEdX( const Trk::MaterialProperties& materialProperties,
                   double momentum,
                   Trk::ParticleHypothesis particleHypothesis = Trk::pion ) const override;

    /** IEnergyLossUpdator public method to compute the mean and variance of the energy loss */
    virtual
      Trk::EnergyLoss energyLoss( const Trk::MaterialProperties& materialProperties,
                                  double momentum,
                                  double pathCorrection,
                                  Trk::PropDirection direction = Trk::alongMomentum,
                                  Trk::ParticleHypothesis particleHypothesis = Trk::pion,
                                  bool usePDGformula = false) const override;

    /** Dummy methodes imposed by public interface - cleanup */
    /** Method to recalculate Eloss values for the fit setting an elossFlag using as an input
        the detailed Eloss information Calorimeter energy, error momentum and momentum error */
    virtual
      Trk::EnergyLoss updateEnergyLoss( Trk::EnergyLoss&, double, double, double, double, int&) const override { return {}; }

    /** Routine to calculate X0 and Eloss scale factors for the Calorimeter and Muon System */
    virtual
      void getX0ElossScales(int, double, double, double&, double& ) const override {}
    /** Dummy methods end here */

  private:

    PublicToolHandle<IEnergyLossUpdator> m_energyLossUpdator{this, "EnergyLossUpdator", "Trk::EnergyLossUpdator/AtlasEnergyLossUpdator"};            //!< ToolHandle to the energy loss updator
    IntegerProperty m_energyLossDistribution{this, "EnergyLossDistribution", 3};       //!< include energy loss straggling or not ( 0 == none, 1 == gauss, 2 == landau)

    /** Random Generator service  - not thread-safe */
    ServiceHandle<IAtRndmGenSvc> m_rndGenSvc{this, "RandomNumberService", "AtDSFMTGenSvc"};
    /** Random engine  */
    CLHEP::HepRandomEngine *m_randomEngine{};
    StringProperty m_randomEngineName{this, "RandomStreamName", "FatrasRnd"};         //!< Name of the random number stream
    BooleanProperty m_usePDGformula{this, "UsePDG_EnergyLossFormula", false};



  };

} // end iFatras namespace

#endif
