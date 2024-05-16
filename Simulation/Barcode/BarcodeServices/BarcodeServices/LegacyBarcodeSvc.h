/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BARCODESERVICES_LEGACYBARCODESVC_H
#define BARCODESERVICES_LEGACYBARCODESVC_H 1

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthService.h"

//include
#include "BarcodeInterfaces/IBarcodeSvc.h"

#include "tbb/concurrent_unordered_map.h"



namespace Barcode {

  /** @class LegacyBarcodeSvc

      This BarcodeService reproduces the barcode treatmend for MC12:
      http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Simulation/G4Sim/MCTruth/src/TruthStrategyManager.cxx

      @author Andreas.Salzburger -at- cern.ch , Elmar.Ritsch -at- cern.ch
  */

  class LegacyBarcodeSvc : public extends<AthService, IBarcodeSvc> {
  public:

    /** Constructor with parameters */
    LegacyBarcodeSvc( const std::string& name, ISvcLocator* pSvcLocator );

    /** Destructor */
    virtual ~LegacyBarcodeSvc() = default;

    /** Athena algorithm's interface methods */
    virtual StatusCode  initialize() override;

    /** Construct and insert a new set of barcode members. To be called for every new thread. */
    virtual StatusCode initializeBarcodes(int largestGeneratedParticleBC=0, int largestGeneratedVertexBC=0) override;

    /** Reset barcodes. To be called at the beginning of each event. */
    virtual StatusCode resetBarcodes(int largestGeneratedParticleBC=0, int largestGeneratedVertexBC=0) override;

    /** Generate a new unique barcode for a secondary particle above the simulation offset */
    virtual int newSecondaryParticle( int) override;

    /** Generate a new unique particle barcode below the simulation offset (for particles from pre-defined decays) */
    virtual int newGeneratedParticle(int ) override;

    /** Generate a new unique vertex barcode above the simulation offset */
    virtual int newSimulationVertex() override;

    /** Generate a new unique vertex barcode below the simulation offset */
    virtual int newGeneratedVertex() override;

    /** Inform the BarcodeSvc about the largest particle and vertex Barcodes
        in the event input */
    virtual void registerLargestGeneratedParticleBC( int bc) override;
    virtual void registerLargestGeneratedVtxBC( int bc) override;
    virtual void registerLargestSecondaryParticleBC( int bc) override;
    virtual void registerLargestSimulationVtxBC( int bc) override;

    /** Return the secondary particle and vertex offsets */
    virtual int secondaryParticleBcOffset() const override;
    virtual int   secondaryVertexBcOffset()  const override;

  private:

    /** barcode information used for GenVertices */
    int m_firstVertex;
    int m_vertexIncrement;

    /** barcode information used for secondary GenParticles */
    int m_firstSecondary;
    int m_particleIncrement;

    struct BarcodeInfo {
      BarcodeInfo() = delete;
      BarcodeInfo(int csv, int csp, int cgv, int cgp)
        : currentSimulationVertex(csv)
        , currentSecondaryParticle(csp)
        , currentGeneratedVertex(cgv)
        , currentGeneratedParticle(cgp) {};
      int currentSimulationVertex{};
      int currentSecondaryParticle{};
      int currentGeneratedVertex{};
      int currentGeneratedParticle{};
    };

    using LegacyBarcodeSvcThreadMap_t = tbb::concurrent_unordered_map
        < std::thread::id, BarcodeInfo, std::hash<std::thread::id> >;
    LegacyBarcodeSvcThreadMap_t m_bcThreadMap;

    BarcodeInfo& getBarcodeInfo();

  };


} // end 'Barcode' namespace

#endif //> !BARCODESERVICES_LEGACYBARCODESVC_H
