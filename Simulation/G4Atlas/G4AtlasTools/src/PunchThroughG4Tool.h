/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTOOLS_PUNCHTHROUGHG4TOOL_H
#define G4ATLASTOOLS_PUNCHTHROUGHG4TOOL_H

//
#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsInitialization.h"
#include "G4AtlasInterfaces/IPunchThroughG4Tool.h"

//Envelope (Calo-MS boundary)
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"
//GeoID (From ISF)
#include "ISF_Interfaces/IGeoIDSvc.h"

// standard C++ libraries
#include <vector>
#include <string>

//libXML
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

//Geant4
#include "G4ParticleTable.hh"
#include "G4Track.hh"
#include "G4TrackVector.hh"
#include "G4FastTrack.hh"
#include "G4FastStep.hh"


/// @class PunchThroughG4Tool
///
/// @brief Punch through calculations.
/// 
/// @author  Elmar Ritsch <Elmar.Ritsch@cern.ch>
/// @maintainer/updater Thomas Carter <thomas.michael.carter@cern.ch>
/// @maintainer/updater Firdaus Soberi <firdaus.soberi@cern.ch>

class TFile;
class PunchThroughParticle;
class PunchThroughPDFCreator;

class PunchThroughG4Tool : virtual public extends<AthAlgTool, IPunchThroughG4Tool, IPhysicsInitializationTool>
{
  public:
    PunchThroughG4Tool(const std::string&, const std::string&, const IInterface*);

    virtual ~PunchThroughG4Tool () = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize() override;
    /** AlgTool finalize method */
    virtual StatusCode finalize() override;

    StatusCode initializePhysics() override;

    /** interface function: fill a vector with the punch-through particles */
    //virtual const G4TrackVector* computePunchThroughParticles(G4ParticleTable &ptable, const G4FastTrack& fastTrack, G4FastStep& fastStep, const TFCSSimulationState& simulstate, CLHEP::HepRandomEngine* rndmEngine);
    virtual std::vector<std::map<std::string, double>> computePunchThroughParticles(const G4FastTrack& fastTrack, CLHEP::HepRandomEngine* rndmEngine, double punchThroughProbability, double punchThroughClassifierRand) override;

    /** create all secondary tracks  from kinematics map */
    virtual void createAllSecondaryTracks(G4ParticleTable &ptable, G4FastStep& fastStep, const G4Track& g4PrimaryTrack, std::vector<std::map<std::string, double>> &secKinematicsMapVect, G4TrackVector& secTrackCont, const std::vector<double> &caloMSVars) override;

    /*helpers*/
    virtual std::vector<double> getCaloMSVars() override;

  private:
    /*---------------------------------------------------------------------
     *  Private member functions
     *---------------------------------------------------------------------*/
    // Check initialization order and whether G4ParticleTable is accessible already
    void checkParticleTable(G4ParticleTable &ptable, int secondarySignedPDG);

    /** registers a type of punch-through particles which will be simulated */
    StatusCode registerPunchThroughParticle(G4ParticleTable &ptable, 
                                            int pdg, bool doAntiparticle = false,
                                            double minEnergy = 0., 
                                            int maxNumParticles = -1,
                                            double numParticlesFactor = 1., 
                                            double energyFactor = 1.,
                                            double posAngleFactor = 1.,
                                            double momAngleFactor = 1.);

    /** initialize register all the punch-through particles which will be simulated */
    StatusCode initializeRegisterPunchThroughParticles();

    /** register a correlation for the two given types of punch-through particles
        with a given energy threshold above which we will have full correlation */
    StatusCode registerCorrelation(int pdgID1, int pdgID2,double minCorrEnergy = 0., double fullCorrEnergy = 0.);

    /** initialize register all correlations between particles */
    StatusCode initializeRegisterCorrelations();

    /** reads out the lookuptable for the given type of particle */
    std::unique_ptr<PunchThroughPDFCreator> readLookuptablePDF(int pdgID, TFile* fileLookupTable, const std::string& folderName);

    /** Check calo-MS boundaries */
    //--------------------------------------------------------------------------------
    // get the calo-MS border coordinates. Look at calo and MS geometry definitions, 
    // if same R and Z -> boundary surface
    // std::vector<RZPair> returned by m_envDefSvc is std::vector<std::pair<double,double>>
    // inside: AtlasGeometryCommon/SubDetectorEnvelopes/SubDetectorEnvelopes/IEnvelopeDefSvc.h
    StatusCode checkCaloMSBoundaries(const std::vector<std::pair<double, double>>* rzMS, 
                                     const std::vector<std::pair<double, double>>* rzCalo);

    /** create the right number of punch-through particles for the given pdg
     *  and return the number of particles which was created. also create these
     *  particles with the right distributions (energy, theta, phi).
     *  if a second argument is given, create exactly this number of particles
     *  (also with the right energy,theta,phi distributions */
    int getAllParticles(const G4Track& g4PrimaryTrack, std::vector<std::map<std::string, double>> &secKinematicsMapVect, CLHEP::HepRandomEngine* rndmEngine, int pdg, double interpEnergy, double interpEta, int numParticles = -1);

    /** get particle through the calorimeter */
    G4ThreeVector punchTroughPosPropagator(double theta, double phi, double R1, double R2, double z1, double z2) const;
    /** check the energies satisfying energy condition */
    std::vector<std::map<std::string, double>> checkEnergySumFromSecondaries(double mainEnergyInit, std::vector<std::map<std::string, double>> &secKinematicsMapVect);
    /** create secondary track for each given the kinematics */
    G4Track* createSecondaryTrack( G4ParticleTable &ptable, G4FastStep& fastStep, double currentTime, int secondarySignedPDG,
                                   double energy, double theta, double phi,double momTheta, double momPhi, const std::vector<double> &caloMSVars);

    /** get the right number of particles for the given pdg while considering
     *  the correlation to an other particle type, which has already created
     *  'corrParticles' number of particles */
    int getCorrelatedParticles(const G4Track& g4PrimaryTrack, std::vector<std::map<std::string, double>> &secKinematicsMapVect, int pdg, int corrParticles, CLHEP::HepRandomEngine* rndmEngine, double interpEnergy, double interpEta);

    /** create exactly one punch-through particle with the given pdg and the given max energy */
    std::map<std::string, double> getOneParticleKinematics(CLHEP::HepRandomEngine* rndmEngine, int secondaryPDG, float initParticleTheta, float initParticlePhi, double interpEnergy, double interpEta) const;

    /** get the floating point number in a string, after the given pattern */
    double getFloatAfterPatternInStr(const char *str, const char *pattern);

    //apply the inverse PCA transform
    std::vector<double> inversePCA(int pcaCdfIterator, std::vector<double> &variables) const;

    //apply the inverse CDF trainsform
    static double inverseCdfTransform(double variable, const std::map<double, double> &inverse_cdf_map) ;

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

    /** ROOT objects */
    TFile*                              m_fileLookupTable{nullptr};   //!< the punch-through lookup table file

    /** needed to initially create punch-through particles with the right distributions */
    std::map<int, PunchThroughParticle*> m_particles;       //!< store all punch-through information for each particle id

    /*---------------------------------------------------------------------
     *  Properties
     *---------------------------------------------------------------------*/
    StringProperty m_filenameLookupTable{this, "FilenameLookupTable", "CaloPunchThroughParametrisation.root", "holds the filename of the lookup table"};
    StringProperty m_filenameInverseCDF{this, "FilenameInverseCdf", "", "holds the filename of inverse quantile transformer config"};
    StringProperty m_filenameInversePCA{this, "FilenameInversePca", "",  "holds the filename of inverse PCA config"};

    IntegerArrayProperty  m_pdgInitiators{this, "PunchThroughInitiators", {}, "vector of punch-through initiator pgds"};
    IntegerArrayProperty  m_initiatorsMinEnergy{this, "InitiatorsMinEnergy", {}, "vector of punch-through initiator min energies to create punch through"};
    DoubleArrayProperty   m_initiatorsEtaRange{this, "InitiatorsEtaRange", {}, "vector of min and max abs eta range to allow punch through initiators"};
    IntegerArrayProperty  m_punchThroughParticles{this, "PunchThroughParticles", {}, "vector of pdgs of the particles produced in punch-throughs"};
    BooleanArrayProperty  m_doAntiParticles{this, "DoAntiParticles", {}, "vector of bools to determine if anti-particles are created for each punch-through particle type"};
    IntegerArrayProperty  m_correlatedParticle{this, "CorrelatedParticle", {}, "holds the pdg of the correlated particle for each given pdg"};
    DoubleArrayProperty   m_minCorrEnergy{this, "MinCorrelationEnergy", {}, "holds the energy threshold below which no particle correlation is computed"};
    DoubleArrayProperty   m_fullCorrEnergy{this, "FullCorrelationEnergy", {}, "holds the energy threshold above which a particle correlation is fully developed"};
    DoubleArrayProperty   m_posAngleFactor{this, "ScalePosDeflectionAngles", {}, "tuning parameter to scale the position deflection angles"};
    DoubleArrayProperty   m_momAngleFactor{this, "ScaleMomDeflectionAngles", {}, "tuning parameter to scale the momentum deflection angles"};
    DoubleArrayProperty   m_minEnergy{this, "MinEnergy", {}, "punch-through particles minimum energies"};
    IntegerArrayProperty  m_maxNumParticles{this, "MaxNumParticles", {}, "maximum number of punch-through particles for each particle type"};
    DoubleArrayProperty   m_numParticlesFactor{this, "NumParticlesFactor", {}, "scale the number of punch-through particles"};
    DoubleArrayProperty   m_energyFactor{this, "EnergyFactor", {}, "scale the energy of the punch-through particles"};

    /*---------------------------------------------------------------------
     *  ServiceHandles
     *---------------------------------------------------------------------*/
    ServiceHandle<ISF::IGeoIDSvc>        m_geoIDSvc{this, "GeoIDSvc", "ISF::GeoIDSvc"};
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
}; // class PunchThroughG4Tool


#endif // G4ATLASTOOLS_PUNCHTHROUGHG4TOOL_H




