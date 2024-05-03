/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_TOOLS_PARTICLEHELPER_H
#define ISF_TOOLS_PARTICLEHELPER_H 1

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
// ISF includes
#include "ISF_Interfaces/IParticleHelper.h"
// Barcode includes
#include "BarcodeEvent/Barcode.h"

namespace ISF {

  class ISFParticle;
  class TruthBinding;

  /** @class ParticleHelper

      A helper tool for the creation or update of an ISF::ISFParticle objects

      @author Andreas.Salzburger -at- cern.ch
  */
  class ParticleHelper : public extends<AthAlgTool, IParticleHelper> {

  public:
    //** Constructor with parameters */
    ParticleHelper( const std::string& t, const std::string& n, const IInterface* p );

    // Athena algtool's Hooks
    virtual StatusCode  initialize() override;
    virtual StatusCode  finalize() override;

    /** Create a new particle */
    virtual ISFParticle* createParticle( double x, double y, double z,
                                         double px, double py, double pz,
                                         double pMass,
                                         double pCharge,
                                         int pPdgId,
                                         int status,
                                         double pTime,
                                         const ISFParticle &parent,
                                         Barcode::ParticleBarcode bc,
                                         int id,
                                         TruthBinding* tBinding = nullptr,
                                         const HepMcParticleLink * partLink = nullptr) const override final;

    /** Create a new particle */
    virtual ISFParticle* createParticle( const Amg::Vector3D& x,
                                         const Amg::Vector3D& p,
                                         double pMass,
                                         double pCharge,
                                         int pPdgId,
                                         int status,
                                         double pTime,
                                         const ISFParticle &parent,
                                         Barcode::ParticleBarcode bc,
                                         int id,
                                         TruthBinding* tBinding = nullptr,
                                         const HepMcParticleLink * partLink = nullptr) const override final;

    /** Create a new particle */
    virtual ISFParticle* createParticle( const HepGeom::Point3D<double>& x,
                                         const HepGeom::Vector3D<double>& p,
                                         double pMass,
                                         double pCharge,
                                         int pPdgId,
                                         int status,
                                         double pTime,
                                         const ISFParticle &parent,
                                         Barcode::ParticleBarcode bc,
                                         int id,
                                         TruthBinding* tBinding = nullptr,
                                         const HepMcParticleLink * partLink = nullptr) const override final;

    /** An updated particle (e.g. after transport) */
    virtual ISFParticle* updatedParticle( const ISFParticle& origIsp,
                                          const Amg::Vector3D& updatedPos,
                                          const Amg::Vector3D& updatedMom,
                                          double deltaTime = 0.) const override final;

    /** An updated particle (e.g. after transport) */
    virtual ISFParticle* updatedParticle( const ISFParticle& origIsp,
                                          const HepGeom::Point3D<double>&  updatedPos,
                                          const HepGeom::Vector3D<double>& updatedMom,
                                          double deltaTime = 0.) const override final;


  };

}


#endif //> !ISF_TOOLS_PARTICLEHELPER_H
