/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FATRASTOOLSID_HITCREATORSILICON_H
#define ISF_FATRASTOOLSID_HITCREATORSILICON_H


// Fatras
#include "ISF_FatrasInterfaces/IHitCreator.h"
// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IIncidentListener.h"
#include "CxxUtils/checker_macros.h"
// Trk
#include "TrkParameters/TrackParameters.h"
// CLHEP
#include "CLHEP/Random/RandomEngine.h"
// Identifier
#include "Identifier/Identifier.h"
//InDet
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "InDetConditionsSummaryService/IInDetConditionsTool.h"

class PixelID;
class SCT_ID;
class IInDetConditionsTool;
class StoreGateSvc;
class TF1;

namespace ISF {
  class ISFParticle;
}

namespace iFatras {

  /**
     @class HitCreatorSilicon

     RIOOnTrack creation, starting from intersection on an active surface

     @author Andreas.Salzburger -at- cern.ch
  */

  class ATLAS_NOT_THREAD_SAFE HitCreatorSilicon : public extends<AthAlgTool, IHitCreator, IIncidentListener>  // deprecated: ATLASSIM-6020
  {
  public:

    /**Constructor */
    HitCreatorSilicon(const std::string&,const std::string&,const IInterface*);

    /**Destructor*/
    virtual ~HitCreatorSilicon() = default;

    /** AlgTool initailize method.*/
    StatusCode initialize();

    /** AlgTool finalize method */
    StatusCode finalize();

    /** handle for incident service */
    void handle(const Incident& inc);

    /** Return nothing - store the HIT in hit collection */
    void createSimHit(const ISF::ISFParticle& isp, const Trk::TrackParameters&, double  ) const;

    /** templated function Return nothing - store the HIT in hit collection */
    void createSimHit(const ISF::ISFParticle& isp, const Trk::TrackParameters& pars, double time, const InDetDD::SiDetectorElement& hitSiDetElement, bool isSiDetElement) const;

    /** Return the cluster on Track -- the PrepRawData is contained in this one */
    const ParametersROT* createHit(const ISF::ISFParticle&, const Trk::TrackParameters&  ) const { return 0; }

    /** Return the cluster on Track -- the PrepRawData is contained in this one */
    const std::vector< ParametersROT >* createHits(const ISF::ISFParticle&, const ParametersLayer& ) const { return 0; }

  protected:

    /* Incident Service */
    ServiceHandle<IIncidentSvc> m_incidentSvc{this, "IncidentService", "IncidentSvc"};

    /*  SiHit collection and collection name */
    SiHitCollection *m_hitColl{};                  //!< the SiHit collection
    StringProperty m_collectionName{this, "CollectionName", "PixelHits"};           //!< name of the collection on storegate

    /** Pointer to the random number generator service */
    ServiceHandle<IAtRndmGenSvc> m_randomSvc{this, "RandomNumberService", "AtRndmGenSvc"};                //!< Random Svc
    StringProperty m_randomEngineName{this, "RandomStreamName", "FatrasRnd"};         //!< Name of the random number stream
    CLHEP::HepRandomEngine *m_randomEngine{};             //!< Random Engine

    StringProperty m_siIdHelperName{this, "IdHelperName", "PixelID"};          //!< where to find the Si helper
    const PixelID *m_pixIdHelper{};             //!< the Pixel ID helper
    const SCT_ID *m_sctIdHelper{};             //!< the SCT ID helper

    /** ToolHandle to ClusterMaker */
    ToolHandle<IInDetConditionsTool>     m_condSummaryTool{this, "ConditionsTool", "PixelConditionsSummaryTool"};          //!< Handle to Pixel/SCT conditions tool
    BooleanProperty m_useConditionsTool{this, "UseConditionsTool", true};
    TF1 *m_dEdX_function{};            //!< function to evaluate dEdx

    DoubleProperty m_siPathToCharge{this, "PathToChargeConversion", 500.};           //!< convert path in silicon to charge
    BooleanProperty m_fastEnergyDepositionModel{this, "FastEnergyDepositionModel", true}; //!< use fast energy deposition model (landau approximation )

    /** Calculate Energyloss with simple Landau approximation */
    double energyDeposit_fast(const ISF::ISFParticle& isp, bool& isPix, bool& isSCT ) const;

    /** Calculate Energyloss with exact Landau*Gauss */
    double energyDeposit_exact(const ISF::ISFParticle& isp, bool& isPix, bool& isSCT ) const;
  };

} // end of namespace

#endif
