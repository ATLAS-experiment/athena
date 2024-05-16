/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_PUNCHTHROUGHTOOLS_SRC_PUNCHTHROUGHTOOL_H
#define ISF_PUNCHTHROUGHTOOLS_SRC_PUNCHTHROUGHTOOL_H 1

#include "ISF_FastCaloSimInterfaces/IPunchThroughTool.h"
#include <string>

#include "ISF_FastCaloSimInterfaces/IPunchThroughClassifier.h"

// Athena Base
#include "AthenaBaseComps/AthAlgTool.h"

//Barcode
#include "BarcodeInterfaces/IBarcodeSvc.h"

//Geometry
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"

#include "ISF_Interfaces/IGeoIDSvc.h"

// Gaudi & StoreGate
#include "GaudiKernel/IPartPropSvc.h"

#include "GeoPrimitives/GeoPrimitives.h"

#include "ISF_Event/ISFParticleContainer.h"

#include "AtlasHepMC/GenEvent_fwd.h"

//libXML
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

/*-------------------------------------------------------------------------
 *  Forward declarations
 *-------------------------------------------------------------------------*/

class TFile;

namespace HepPDT {
  class ParticleDataTable;
}

namespace ISF {
  class PunchThroughParticle;
  class PDFcreator;
}

namespace ISF {

  class PunchThroughTool : public extends<AthAlgTool, IPunchThroughTool>
  {
  public:
    /** Constructor */
    PunchThroughTool(const std::string&,const std::string&,const IInterface*);

    /** Destructor */
    virtual ~PunchThroughTool () = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize();
    /** AlgTool finalize method */
    virtual StatusCode finalize  ();
    /** interface function: fill a vector with the punch-through particles */
    const ISF::ISFParticleVector* computePunchThroughParticles(const ISF::ISFParticle &isfp, const TFCSSimulationState& simulstate, CLHEP::HepRandomEngine* rndmEngine) const;

  private:
    /*---------------------------------------------------------------------
     *  Private member functions
     *---------------------------------------------------------------------*/
    /** registers a type of punch-through particles which will be simulated */
    StatusCode registerParticle(int pdgID, bool doAntiparticle = false,
                                double minEnergy = 0., int maxNumParticles = -1,
                                double numParticlesFactor = 1., double energyFactor = 1.,
                                double posAngleFactor = 1.,
                                double momAngleFactor = 1.);
    /** register a correlation for the two given types of punch-through particles
        with a given energy threshold above which we will have full correlation */
    StatusCode registerCorrelation(int pdgID1, int pdgID2,double minCorrEnergy = 0., double fullCorrEnergy = 0.);

    /** reads out the lookuptable for the given type of particle */
    std::unique_ptr<ISF::PDFcreator> readLookuptablePDF(int pdgID, const std::string& folderName);

    /** create the right number of punch-through particles for the given pdg
     *  and return the number of particles which was created. also create these
     *  particles with the right distributions (energy, theta, phi).
     *  if a second argument is given, create exactly this number of particles
     *  (also with the right energy,theta,phi distributions */
    int getAllParticles(const ISF::ISFParticle &isfp, ISFParticleVector& isfpCont, CLHEP::HepRandomEngine* rndmEngine, int pdg, double interpEnergy, double interpEta, int numParticles = -1) const;

    /** get the right number of particles for the given pdg while considering
     *  the correlation to an other particle type, which has already created
     *  'corrParticles' number of particles */
    int getCorrelatedParticles(const ISF::ISFParticle &isfp, ISFParticleVector& isfpCont, int doPdg, int corrParticles, CLHEP::HepRandomEngine* rndmEngine, double interpEnergy, double interpEta) const;

    /** create exactly one punch-through particle with the given pdg and the given max energy */
    ISF::ISFParticle *getOneParticle(const ISF::ISFParticle &isfp, int pdg, CLHEP::HepRandomEngine* rndmEngine, double interpEnergy, double interpEta) const;

    /** create a ISF Particle state at the MS entrace containing a particle with the given properties */
    ISF::ISFParticle *createExitPs(const ISF::ISFParticle &isfp, int PDGcode, double energy, double theta, double phi, double momTheta, double momPhi) const;

    /** get the floating point number in a string, after the given pattern */
    double getFloatAfterPatternInStr(const char *str, const char *pattern);
    /** get particle through the calorimeter */
    Amg::Vector3D propagator(double theta, double phi) const;

    //apply the inverse PCA transform
    std::vector<double> inversePCA(int pcaCdfIterator, std::vector<double> &variables) const;

    //apply the inverse CDF trainsform
    static double inverseCdfTransform(double variable, const std::map<double, double>& inverse_cdf_map) ;

    //dot product between matrix and vector, used to inverse PCA
    static std::vector<double> dotProduct(const std::vector<std::vector<double>> &m, const std::vector<double> &v) ;

    //returns normal cdf based on normal gaussian value
    static double normal_cdf(double x) ;

    //apply energy interpolation
    double interpolateEnergy(const double &energy, CLHEP::HepRandomEngine* rndmEngine) const;

    //apply eta interpolation
    double interpolateEta(const double &eta, CLHEP::HepRandomEngine* rndmEngine) const;

    //get the infoMap from xml file based on the xmlpathname and also name of mainNode
    std::vector<std::map<std::string,std::string>> getInfoMap(const std::string& mainNode, const std::string &xmlFilePath);

    //decide the pca / cdf part to read based on pdgId and eta
    int passedParamIterator(int pid, double eta, const std::vector<std::map<std::string,std::string>> &mapvect) const;

    //load inverse quantile transformer from XML
    StatusCode initializeInverseCDF(const std::string & quantileTransformerConfigFile);

    //get CDF mapping for individual XML node
    static std::map<double, double> getVariableCDFmappings(xmlNodePtr& nodeParent);

    //load inverse PCA from XML
    StatusCode initializeInversePCA(const std::string & inversePCAConfigFile);


    /*---------------------------------------------------------------------
     *  Private members
     *---------------------------------------------------------------------*/

     /** energy and eta points in param */
     std::vector<double>                 m_energyPoints;
     std::vector<double>                 m_etaPoints;

    /** calo-MS borders */
    double                               m_R1{0.};
    double                               m_R2{0.};
    double                               m_z1{0.};
    double                               m_z2{0.};

    /** ParticleDataTable needed to get connection pdg_code <-> charge */
    const HepPDT::ParticleDataTable*    m_particleDataTable{nullptr};

    /** ROOT objects */
    TFile*                              m_fileLookupTable{nullptr};   //!< the punch-through lookup table file

    /** needed to create punch-through particles with the right distributions */
    std::map<int, PunchThroughParticle*> m_particles;       //!< store all punch-through information for each particle id

    /*---------------------------------------------------------------------
     *  Properties
     *---------------------------------------------------------------------*/

    /** Properties */
    StringProperty m_filenameLookupTable{this, "FilenameLookupTable", "CaloPunchThroughParametrisation.root", "holds the filename of the lookup table"};
    StringProperty m_filenameInverseCDF{this, "FilenameInverseCdf", "", "holds the filename of inverse quantile transformer config"};
    StringProperty m_filenameInversePCA{this, "FilenameInversePca", "",  "holds the filename of inverse PCA config"};

    PublicToolHandle<IPunchThroughClassifier> m_punchThroughClassifier{this, "PunchThroughClassifier", "ISF_PunchThroughClassifier", ""};
    IntegerArrayProperty   m_pdgInitiators{this, "PunchThroughInitiators", {}, "vector of punch-through initiator pgds"};
    IntegerArrayProperty   m_initiatorsMinEnergy{this, "InitiatorsMinEnergy", {}, "vector of punch-through initiator min energies to create punch through"};
    DoubleArrayProperty   m_initiatorsEtaRange{this, "InitiatorsEtaRange", {}, "vector of min and max abs eta range to allow punch through initiators"};
    IntegerArrayProperty   m_punchThroughParticles{this, "PunchThroughParticles", {}, "vector of pdgs of the particles produced in punch-throughs"};
    BooleanArrayProperty m_doAntiParticles{this, "DoAntiParticles", {}, "vector of bools to determine if anti-particles are created for each punch-through particle type"};
    IntegerArrayProperty   m_correlatedParticle{this, "CorrelatedParticle", {}, "holds the pdg of the correlated particle for each given pdg"};
    DoubleArrayProperty   m_minCorrEnergy{this, "MinCorrelationEnergy", {}, "holds the energy threshold below which no particle correlation is computed"};
    DoubleArrayProperty   m_fullCorrEnergy{this, "FullCorrelationEnergy", {}, "holds the energy threshold above which a particle correlation is fully developed"};
    DoubleArrayProperty   m_posAngleFactor{this, "ScalePosDeflectionAngles", {}, "tuning parameter to scale the position deflection angles"};
    DoubleArrayProperty   m_momAngleFactor{this, "ScaleMomDeflectionAngles", {}, "tuning parameter to scale the momentum deflection angles"};
    DoubleArrayProperty   m_minEnergy{this, "MinEnergy", {}, "punch-through particles minimum energies"};
    IntegerArrayProperty   m_maxNumParticles{this, "MaxNumParticles", {}, "maximum number of punch-through particles for each particle type"};
    DoubleArrayProperty   m_numParticlesFactor{this, "NumParticlesFactor", {}, "scale the number of punch-through particles"};
    DoubleArrayProperty   m_energyFactor{this, "EnergyFactor", {}, "scale the energy of the punch-through particles"};

    

    /*---------------------------------------------------------------------
     *  ServiceHandles
     *---------------------------------------------------------------------*/
    ServiceHandle<IPartPropSvc>          m_particlePropSvc{this, "PartPropSvc", "PartPropSvc", "particle properties svc"};
    ServiceHandle<IGeoIDSvc>             m_geoIDSvc{this, "GeoIDSvc", "ISF::GeoIDSvc"};
    ServiceHandle<Barcode::IBarcodeSvc>  m_barcodeSvc{this, "BarcodeSvc", "BarcodeSvc"};
    ServiceHandle<IEnvelopeDefSvc>       m_envDefSvc{this, "EnvelopeDefSvc", "AtlasGeometry_EnvelopeDefSvc"};

    /** beam pipe radius */
    DoubleProperty                                  m_beamPipe{this, "BeamPipeRadius", 500.};

    /** pca vectors */
    std::vector<std::vector<std::vector<double>>> m_inverse_PCA_matrix;
    std::vector<std::vector<double>> m_PCA_means;

    /** infoMaps */
    std::vector<std::map<std::string, std::string>> m_xml_info_pca;
    std::vector<std::map<std::string, std::string>> m_xml_info_cdf;

    /** (vector of map) for CDF mappings */
    std::vector<std::map<double, double>>  m_variable0_inverse_cdf;
    std::vector<std::map<double, double>>  m_variable1_inverse_cdf;
    std::vector<std::map<double, double>>  m_variable2_inverse_cdf;
    std::vector<std::map<double, double>>  m_variable3_inverse_cdf;
    std::vector<std::map<double, double>>  m_variable4_inverse_cdf;
  };
}

#endif
