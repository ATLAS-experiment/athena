/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BARCODESERVICES_VALIDATIONBARCODESVC_H
#define BARCODESERVICES_VALIDATIONBARCODESVC_H 1

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthService.h"

//include
#include "BarcodeInterfaces/IBarcodeSvc.h"

class IIncidentSvc;


namespace Barcode {

  /** @class ValidationBarcodeSvc

      This BarcodeService reproduces the barcode treatmend for MC12:
      http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Simulation/G4Sim/MCTruth/src/TruthStrategyManager.cxx

      @author Andreas.Salzburger -at- cern.ch , Elmar.Ritsch -at- cern.ch
  */

  class ValidationBarcodeSvc : public extends<AthService, IBarcodeSvc, IIncidentListener> {
  public:

    /** Constructor with parameters */
    ValidationBarcodeSvc( const std::string& name, ISvcLocator* pSvcLocator );

    /** Destructor */
    virtual ~ValidationBarcodeSvc();

    /** Athena algorithm's interface methods */
    virtual StatusCode  initialize() override;
    virtual StatusCode  finalize() override;

    /** Incident to reset the barcodes at the beginning of the event */
    virtual void handle(const Incident& inc) override;

    /** Generate a new unique vertex barcode, based on the parent particle barcode and
        the physics process code causing the truth vertex*/
    virtual int newSimulationVertex() override;

    /** Generate a new unique barcode for a secondary particle, based on the parent
        particle barcode */
    virtual int newSecondaryParticle( int parentBC=HepMC::UNDEFINED_ID ) override;

   /** Generate a new unique particle barcode below the simulation offset (for particles from pre-defined decays) */
    virtual int newGeneratedParticle(int parentBC=HepMC::UNDEFINED_ID ) override;

    /** Generate a new unique vertex barcode below the simulation offset */
    virtual int newGeneratedVertex() override;

    /** Generate a common barcode which will be shared by all children
        of the given parent barcode (used for child particles which are
        not stored in the mc truth event) */
    virtual int sharedChildBarcode( int parentBC,
                                    int process=0 ) override;

    /** Return the secondary particle and vertex offsets */
    virtual int secondaryParticleBcOffset() const override;
    virtual int secondaryVertexBcOffset() const override;

    /** Inform the BarcodeSvc about the largest particle and vertex Barcodes
        in the event input */
    virtual void registerLargestGeneratedParticleBC( int ) override {};
    virtual void registerLargestGeneratedVtxBC( int ) override {};
    virtual void registerLargestSecondaryParticleBC( int ) override {};
    virtual void registerLargestSimulationVtxBC( int ) override {};

  private:
    ServiceHandle<IIncidentSvc>                   m_incidentSvc;   //!< IncidentSvc to catch begin of event and end of envent

    /** barcode information used for GenVertices */
    int                                 m_firstVertex;
    int                                 m_vertexIncrement;
    int                                 m_currentVertex;

    /** barcode information used for secondary GenParticles */
    int                               m_firstSecondary;
    int                               m_secondaryIncrement;
    int                               m_currentSecondary;

    /** barcode offset for each generation of updated particles */
    int                               m_particleGenerationIncrement;
    int                               m_barcodeGenerationOffset; //!< not sure why this is needed...

  };


} // end 'Barcode' namespace

#endif //> !BARCODESERVICES_VALIDATIONBARCODESVC_H
