/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// PunchThrough headers
#include "PunchThroughPDFCreator.h"
#include "PunchThroughParticle.h"
#include "PunchThroughG4Tool.h"

// PathResolver
#include "PathResolver/PathResolver.h"

// standard C++ libraries
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <numeric>
#include <string_view>
#include <charconv>

// standard C libraries
#include <cmath>

// CLHEP
#include "CLHEP/Random/RandFlat.h"

// ROOT
#include "TFile.h"
#include "TH2F.h"
#include "TH1.h"
#include "TMath.h"
#include "TKey.h"

// namespace Amg functions
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

PunchThroughG4Tool::PunchThroughG4Tool(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type,name,parent)
{
}

// Athena method, used to get out the G4 geometry and set up the Fast Simulation Models
StatusCode PunchThroughG4Tool::initialize(){
  ATH_MSG_DEBUG("[PunchThroughG4Tool] ==>" << name() << "::initialize()");

  // resolving lookuptable file
  std::string resolvedFileName = PathResolverFindCalibFile (m_filenameLookupTable);
  if (resolvedFileName.empty()) {
    ATH_MSG_ERROR( "[PunchThroughG4Tool] Parametrisation file '" << m_filenameLookupTable << "' not found" );
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG( "[PunchThroughG4Tool] Parametrisation file found: " << resolvedFileName );

  //retrieve inverse CDF config file
  if (!initializeInverseCDF(PathResolverFindCalibFile(m_filenameInverseCDF)))
  {
    ATH_MSG_WARNING("[PunchThroughG4Tool] unable to open or read the inverse CDF config");
    return StatusCode::FAILURE;
  }

  //retrieve inverse PCA config file
  if (!initializeInversePCA(PathResolverFindCalibFile(m_filenameInversePCA)))
  {
    ATH_MSG_WARNING("[PunchThroughG4Tool] unable to open or read the inverse PCA config");
    return StatusCode::FAILURE;
  }

  //check first the size of infoMap for both PCA and CDF, they should be equal
  if (!(m_xml_info_pca.size() == m_xml_info_cdf.size()))
  {
    ATH_MSG_WARNING("[PunchThroughG4Tool] size of infoMap for PCA and CDF differs! Something is wrong with input xml files.");
    return StatusCode::FAILURE;
  }

  // open the LookupTable file
  m_fileLookupTable = new TFile( resolvedFileName.c_str(), "READ");
  if (!m_fileLookupTable) {
    ATH_MSG_WARNING("[PunchThroughG4Tool] unable to open the lookup-table for the punch-through simulation (file does not exist)");
    return StatusCode::FAILURE;
  }

  if (!m_fileLookupTable->IsOpen()) {
    ATH_MSG_WARNING("[PunchThroughG4Tool] unable to open the lookup-table for the punch-through simulation (wrong or empty file?)");
    return StatusCode::FAILURE;
  }

  //--------------------------------------------------------------------------------
  // register all the punch-through particles which will be simulated  
  ATH_CHECK(initializeRegisterPunchThroughParticles());

  //--------------------------------------------------------------------------------
  // register all the punch-through correlations between particles to be simulated  
  ATH_CHECK(initializeRegisterCorrelations());

  //--------------------------------------------------------------------------------
  // close the file with the lookuptable
  m_fileLookupTable->Close();

  // Geometry identifier service
  if ( !m_geoIDSvc.empty() && m_geoIDSvc.retrieve().isFailure())
  {
    ATH_MSG_FATAL ( "[PunchThroughG4Tool] Could not retrieve GeometryIdentifier Service. Abort");
    return StatusCode::FAILURE;
  }

  //envelope definition service
  if (m_envDefSvc.retrieve().isFailure() )
  {
    ATH_MSG_ERROR( "[PunchThroughG4Tool] Could not retrieve " << m_envDefSvc );
    return StatusCode::FAILURE;
  }

  // direct-initialization of the Muon and Calo boundaries (giving R, Z coordinates)
  // can also do like below (RZPairVector is vector<pair<double, double>>)
  // const RZPairVector* rzMS = &(m_envDefSvc->getMuonRZBoundary());
  const std::vector<std::pair<double, double>>* rzMS = &(m_envDefSvc->getMuonRZBoundary());  
  const std::vector<std::pair<double, double>>* rzCalo = &(m_envDefSvc->getCaloRZBoundary());  
  ATH_CHECK(checkCaloMSBoundaries(rzMS, rzCalo));

  ATH_MSG_INFO( "[PunchThroughG4Tool] initialization is successful" );
  return StatusCode::SUCCESS;
}

StatusCode PunchThroughG4Tool::initializeRegisterPunchThroughParticles(){
  G4ParticleTable *ptable = G4ParticleTable::GetParticleTable(); 
  for ( unsigned int num = 0; num < m_punchThroughParticles.size(); num++ )
  {
    const int pdg = m_punchThroughParticles[num];
    // if no information is given on the creation of anti-particles -> do not simulate anti-particles
    const bool doAnti = ( num < m_doAntiParticles.size() ) ? m_doAntiParticles[num] : false;
    // if no information is given on the minimum energy -> take 50. MeV as default
    const double minEnergy = ( num < m_minEnergy.size() ) ? m_minEnergy[num] : 50.;
    // if no information is given on the maximum number of punch-through particles -> take -1 as default
    const int maxNum = ( num < m_minEnergy.size() ) ? m_maxNumParticles[num] : -1;
    // if no information is given on the scale factor for the number of particles -> take 1. as defaulft
    const double numFactor = ( num < m_numParticlesFactor.size() ) ? m_numParticlesFactor[num] : 1.;
    // if no information is given on the position angle factor -> take 1.
    const double posAngleFactor = ( num < m_posAngleFactor.size() ) ? m_posAngleFactor[num] : 1.;
    // if no information is given on the momentum angle factor -> take 1.
    const double momAngleFactor = ( num < m_momAngleFactor.size() ) ? m_momAngleFactor[num] : 1.;
    // if no information is given on the scale factor for the energy -> take 1. as default
    const double energyFactor = ( num < m_energyFactor.size() ) ? m_energyFactor[num] : 1.;

    // register the particle
    ATH_MSG_VERBOSE("VERBOSE: [PunchThroughG4Tool] registering punch-through particle type with pdg = " << pdg );    
    if (registerPunchThroughParticle( *ptable, pdg, doAnti, minEnergy, maxNum, numFactor, energyFactor, posAngleFactor, momAngleFactor ) != StatusCode::SUCCESS)
    {
      ATH_MSG_ERROR("[PunchThroughG4Tool] unable to register punch-through particle type with pdg = " << pdg);
      return StatusCode::FAILURE;
    }
  }
  // if all goes well
  return StatusCode::SUCCESS;
}

StatusCode PunchThroughG4Tool::initializeRegisterCorrelations(){
  //--------------------------------------------------------------------------------
  // TODO: implement punch-through parameters for different m_pdgInitiators
  //       currently m_pdgInitiators is only used to filter out particles,
  //       inside computePunchThroughParticles

  // check if more correlations were given than particle types were registered
  unsigned int numCorrelations = m_correlatedParticle.size();
  if ( numCorrelations > m_punchThroughParticles.size() )
  {
    ATH_MSG_WARNING("[PunchThroughG4Tool] more punch-through particle correlations are given, than punch-through particle types are registered (skipping the last ones)");
    numCorrelations = m_punchThroughParticles.size();
  }

  // now register correlation between particles
  for ( unsigned int num = 0; num < numCorrelations; num++ )
  {
    const int pdg1 = m_punchThroughParticles[num];
    const int pdg2 = m_correlatedParticle[num];
    const double fullCorrEnergy = ( num < m_fullCorrEnergy.size() ) ? m_fullCorrEnergy[num] : 0.;
    const double minCorrEnergy = ( num < m_minCorrEnergy.size() ) ?  m_minCorrEnergy[num] : 0.;

    // if correlatedParticle==0 is given -> no correlation
    if ( ! pdg2) continue;
    // register it
    if ( registerCorrelation(pdg1, pdg2, minCorrEnergy, fullCorrEnergy) != StatusCode::SUCCESS )
    {
      ATH_MSG_ERROR("[PunchThroughG4Tool] unable to register punch-through particle correlation for pdg1=" << pdg1 << " pdg2=" << pdg2 );
      return StatusCode::FAILURE;
    }
  }
  // if all goes well
  return StatusCode::SUCCESS;
}

std::vector<double> PunchThroughG4Tool::getCaloMSVars(){
  //m_R1, m_R2, m_z1, m_z2
  std::vector<double> caloMSVarsVect = {m_R1, m_R2, m_z1, m_z2};
  ATH_MSG_DEBUG("[PunchThroughG4Tool] getCaloMSVars()==> m_R1 = "<< caloMSVarsVect[0] << ", m_R2 = " << caloMSVarsVect[1] << ", m_z1 = " << caloMSVarsVect[2] << ", m_z2 = " << caloMSVarsVect[3]);

  return caloMSVarsVect;
}

StatusCode PunchThroughG4Tool::checkCaloMSBoundaries(const std::vector<std::pair<double, double>>* rzMS, 
                                                     const std::vector<std::pair<double, double>>* rzCalo)
{
  bool found1, found2;
  found1=false; found2=false;

  // procedure below is to find intersection of calo and MS boundary (right at the edge of calo and into MS)
  // at least 2 distinct calo-MS boundary points must be found to proceed
  for ( const auto & [r_tempCalo , z_tempCalo] : *rzCalo )
  {
    // calo point must be more than beampipe radius
    if (r_tempCalo> m_beamPipe)
    {
      for ( const auto & [r_tempMS , z_tempMS] : *rzMS )
      {
        // first point, if R and Z of calo matches R and Z of MS
        if (r_tempCalo==r_tempMS && z_tempCalo==z_tempMS && found1==false )
        {
          m_R1=r_tempMS;
          m_z1=std::fabs(z_tempMS);
          found1=true;
          continue;
        }
        // second point, if R and Z of calo matches R and Z of MS
        else if (r_tempCalo==r_tempMS && z_tempCalo==z_tempMS && r_tempCalo!=m_R1 && std::fabs(z_tempCalo)!=m_z1)
        {
          m_R2=r_tempMS;
          m_z2=std::fabs(z_tempMS);
          found2=true;
        }
      }

      if (found1==true && found2==true) break;
    }
  }

  //in case geometry description changes
  if (found1 == false){
    ATH_MSG_ERROR ("[PunchThroughG4Tool] first coordinate of calo-MS border not found"); 
    return StatusCode::FAILURE;
  }
  if (found2 == false){
    ATH_MSG_ERROR ("[PunchThroughG4Tool] second coordinate of calo-MS border not found; first one is: R1 ="<<m_R1<<" z1 ="<<m_z1);
    return StatusCode::FAILURE;
  }

  //now order the found values
  if (m_R1>m_R2) { std::swap(m_R1, m_R2); } //m_R1 - smaller one
  if (m_z1<m_z2) { std::swap(m_z1, m_z2);  } //m_z1 - bigger one

  if (m_R1==m_R2 || m_z1==m_z2){
    ATH_MSG_ERROR ("[PunchThroughG4Tool] Bug in propagation calculation! R1="<<m_R1<<" R2 = "<<m_R2<<" z1="<<m_z1<<" z2= "<<m_z2 );    
    return StatusCode::FAILURE;
  } 

  ATH_MSG_DEBUG ("[PunchThroughG4Tool] calo-MS boundary coordinates: R1="<<m_R1<<" R2 = "<<m_R2<<" z1="<<m_z1<<" z2= "<<m_z2);    

  return StatusCode::SUCCESS;
}

StatusCode PunchThroughG4Tool::finalize()
{
  ATH_MSG_DEBUG( "[PunchThroughG4Tool] finalize() starting" );
  for(auto & each : m_particles) {
    delete each.second;
  }

  ATH_MSG_DEBUG( "[PunchThroughG4Tool] finalize() successful" );

  return StatusCode::SUCCESS;
}

void PunchThroughG4Tool::checkParticleTable(G4ParticleTable &ptable, int secondarySignedPDG){
  // Get Geant4 particle definition
  G4ParticleDefinition* secG4Particle = ptable.FindParticle(secondarySignedPDG);
  double mass = secG4Particle->GetPDGMass();
  ATH_MSG_DEBUG("[PunchThroughG4Tool::checkParticleTable] secondarySignedPDG = " << secondarySignedPDG << ", mass = "<< mass);
}

std::vector<std::map<std::string, double>> PunchThroughG4Tool::computePunchThroughParticles(const G4FastTrack& fastTrack, CLHEP::HepRandomEngine* rndmEngine, double punchThroughProbability, double punchThroughClassifierRand)
{
  ATH_MSG_DEBUG( "[PunchThroughG4Tool] starting punch-through simulation");

  // holder to all secondary particle kinematics
  std::vector<std::map<std::string, double>> secKinematicsMapVect;

  // Get primary Geant4 track (main particle producing punchthrough)
  const G4Track * g4PrimaryTrack = fastTrack.GetPrimaryTrack(); 

  // Get Geant4 particle definition
  const G4ParticleDefinition * mainG4Particle = g4PrimaryTrack->GetDefinition();

  // Get primary Geant4 particle properties
  int pdgID = mainG4Particle->GetPDGEncoding();
  float mainPartMass = mainG4Particle->GetPDGMass();
  float mainMomMag2  = g4PrimaryTrack->GetMomentum().mag2();  
  float mainPartEta  = g4PrimaryTrack->GetPosition().eta();
  // get primary track's current position and momentum direction
  const G4ThreeVector mainMomentumDir = g4PrimaryTrack->GetMomentumDirection();
  const G4ThreeVector mainMomentum    = g4PrimaryTrack->GetMomentum();
  const G4ThreeVector mainPosition    = g4PrimaryTrack->GetPosition();

  // calculate incoming energy and eta
  const double initEnergy = std::sqrt( mainMomMag2 + mainPartMass*mainPartMass );
  const double initEta = mainPartEta;

  ATH_MSG_DEBUG("[PunchThroughG4Tool] position of the input particle: r"<<mainPosition.perp()<<" z= "<<mainPosition.z()  );

  // check if it points to the calorimeter - if not, don't simulate
  // first convert to Amg::Vector3D
  const Amg::Vector3D amg3DPosVec(mainPosition.x(), mainPosition.y(), mainPosition.z());
  const Amg::Vector3D amg3DMomDirVec(mainMomentumDir.x(), mainMomentumDir.y(), mainMomentumDir.z());
  const Amg::Vector3D amg3DMomVec(mainMomentum.x(), mainMomentum.y(), mainMomentum.z());
  // find nextGeoID to get AtlasDetDescr::AtlasRegion
  AtlasDetDescr::AtlasRegion nextGeoID = m_geoIDSvc->identifyNextGeoID(amg3DPosVec,amg3DMomVec);
  if ( nextGeoID != AtlasDetDescr::fAtlasCalo)
  {
    ATH_MSG_DEBUG("[PunchThroughG4Tool](GeoIDSvc) input particle doesn't point to calorimeter"<< "Next GeoID: "<< nextGeoID );
    //return nullptr;
    return {};
  }

  // check if the particle's pdg is registered as a punch-through-causing type
  {
    std::vector<int>::const_iterator pdgIt    = m_pdgInitiators.begin();
    std::vector<int>::const_iterator pdgItEnd = m_pdgInitiators.end();

    std::vector<int>::const_iterator minEnergyIt    = m_initiatorsMinEnergy.begin();
    // loop over all known punch-through initiators
    for ( ; pdgIt != pdgItEnd; ++pdgIt, ++minEnergyIt)
    {
      if (std::abs(pdgID) == *pdgIt){
        if(initEnergy < *minEnergyIt){
          ATH_MSG_DEBUG("[PunchThroughG4Tool] particle does not meet initiator min energy requirement. Dropping it in the calo.");
          //return nullptr;
          return {};
        }
        break;
      }
    }

    // particle will not cause punch-through -> bail out
    if (pdgIt == pdgItEnd)
    {
      ATH_MSG_DEBUG("[PunchThroughG4Tool] particle is not registered as punch-through initiator. Dropping it in the calo.");
      //return nullptr;
      return {};
    }
  }

  if(initEta  < m_initiatorsEtaRange.value().at(0) || initEta > m_initiatorsEtaRange.value().at(1) ){
    ATH_MSG_DEBUG("[PunchThroughG4Tool] particle does not meet initiator eta range requirement. Dropping it in the calo.");
    //return nullptr;
    return {};    
  }

  ATH_MSG_DEBUG("[PunchThroughG4Tool] punchThroughProbability output: " << punchThroughProbability << " RandFlat: " << punchThroughClassifierRand );

  //Safety check: If probability < random number then don't simulate punch through
  if( punchThroughClassifierRand > punchThroughProbability){
      ATH_MSG_DEBUG("[PunchThroughG4Tool] particle not classified to create punch through. Dropping it in the calo.");
      //return nullptr;
      return {};
  }

  //if initial particle is on ID surface, points to the calorimeter, is a punch-through initiator, meets initiator min enery and eta range

  // this is the place where the magic is done:
  // test for each registered punch-through pdg if a punch-through
  // occures and create these particles
  // -> therefore loop over all registered pdg ids
  // to keep track of the correlated particles which were already simulated:
  // first int is pdg, second int is number of particles created

  // interpolate energy and eta
  const double interpEnergy = interpolateEnergy(initEnergy, rndmEngine);
  const double interpEta    = interpolateEta(initEta, rndmEngine);

  std::map<int, int> corrPdgNumDone;

  int maxTries = 10;
  int nTries = 0;

  // loop over all particle pdgs
  while(secKinematicsMapVect.empty() && nTries < maxTries) { //ensure we always create at least one punch through particle, maxTries to catch very rare cases
    // loop over all particle pdgs
    for (const auto& currentParticle : m_particles)
    {
      // the pdg that is currently treated
      int doPdg = currentParticle.first;
      // get the current particle's correlated pdg
      int corrPdg = currentParticle.second->getCorrelatedPdg();

      // if there is a correlated particle type to this one
      if (corrPdg)
      {
        // find out if the current pdg was already simulated
        std::map<int,int>::iterator itPdgPos = corrPdgNumDone.find(doPdg);
        // if the current pdg was not simulated yet, find out if
        // it's correlated one was simulated
        if ( itPdgPos == corrPdgNumDone.end() ) itPdgPos = corrPdgNumDone.find(corrPdg);

        // neither this nor the correlated particle type was simulated
        // so far:
        if ( itPdgPos == corrPdgNumDone.end() )
          {
            // -> roll a dice if we create this particle or its correlated one
            if ( CLHEP::RandFlat::shoot(rndmEngine) > 0.5 ) doPdg = corrPdg;
            // now create the particles with the given pdg and note how many
            // particles of this pdg are created
            corrPdgNumDone[doPdg] = getAllParticles(*g4PrimaryTrack, secKinematicsMapVect, rndmEngine, doPdg, interpEnergy, interpEta);
          }

        // one of the two correlated particle types was already simulated
        // 'itPdgPos' points to the already simulated one
        else
          {
            // get the pdg of the already simulated particle and the number
            // of these particles that were created
            const int donePdg = itPdgPos->first;
            const int doneNumPart = itPdgPos->second;
            // set the pdg of the particle type that will be done
            if (donePdg == doPdg) doPdg = corrPdg;

            // now create the correlated particles
            getCorrelatedParticles(*g4PrimaryTrack, secKinematicsMapVect, doPdg, doneNumPart, rndmEngine, interpEnergy, interpEta);
            // note: no need to take note, that this particle type is now simulated,
            // since this is the second of two correlated particles, which is
            // simulated and we do not have correlations of more than two particles.
          }

        // if no correlation for this particle
        // -> directly create all particles with the current pdg
      }
      else{
        getAllParticles(*g4PrimaryTrack, secKinematicsMapVect, rndmEngine, doPdg, interpEnergy, interpEta);
      } 
    } // for-loop over all particle pdgs
  }

  return secKinematicsMapVect;
}

int PunchThroughG4Tool::getAllParticles(const G4Track& g4PrimaryTrack, std::vector<std::map<std::string, double>> &secKinematicsMapVect, CLHEP::HepRandomEngine* rndmEngine, int pdg, double interpEnergy, double interpEta, int numParticles)
{  
  // Get initial Geant4 position theta and phi
  float initParticleTheta = g4PrimaryTrack.GetPosition().theta();
  float initParticlePhi   = g4PrimaryTrack.GetPosition().phi();

  // get the current particle
  PunchThroughParticle *p = m_particles.at(pdg);
  double minAllowedEnergy = p->getMinEnergy();

  // if no number of particles (=-1) was handed over as an argument
  //  -> get the number of particles from the pdf
  if ( numParticles < 0 )
  {
    // prepare the function arguments for the PunchThroughPDFCreator class
    std::vector<int> parameters;
    parameters.push_back( std::round(interpEnergy) );
    parameters.push_back( std::round(interpEta*100) );
    // the maximum number of particles which should be produced
    // if no maximum number is given, this is -1
    int maxParticles = p->getMaxNumParticles();

    // get the right number of punch-through particles
    // and ensure that we do not create too many particles
    do
    {
      numParticles = int( p->getNumParticlesPDF()->getRand(rndmEngine, parameters) );

      // scale the number of particles if requested
      numParticles = lround( numParticles *= p->getNumParticlesFactor() );
    }
    while ( (maxParticles >= 0.) && (numParticles > maxParticles) );
  }

  ATH_MSG_DEBUG("[PunchThroughG4Tool] adding " << numParticles << " punch-through particles with pdg code: " << pdg);

  // get how many secondary particles to be created
  for ( int numCreated = 0; numCreated < numParticles; numCreated++ )
  {
    // create one particle which fullfills the right energy distribution
    std::map<std::string, double> secondaryKinematicsMap = getOneParticleKinematics(rndmEngine, pdg, initParticleTheta, initParticlePhi, interpEnergy, interpEta);

    //safety to cater minimum energy as rest mass
    double energy = secondaryKinematicsMap.at("energy");
    if(energy > minAllowedEnergy){
      // push to the vector of maps
      secKinematicsMapVect.push_back(secondaryKinematicsMap);      
    }
  }

  // Get the size of the vector
  std::size_t numSecondaries = secKinematicsMapVect.size();

  // the actual (pre) number of particles which was created is numSecondaries, which is incremented in the loop
  return (numSecondaries);
}

int PunchThroughG4Tool::getCorrelatedParticles(const G4Track& g4PrimaryTrack, std::vector<std::map<std::string, double>> &secKinematicsMapVect, int pdg, int corrParticles, CLHEP::HepRandomEngine* rndmEngine, double interpEnergy, double interpEta)
{
  // Get Geant4 particle definition
  const G4ParticleDefinition * mainG4Particle = g4PrimaryTrack.GetDefinition();
  // Get Geant4 particle pdgID
  float mainMomMag2  = g4PrimaryTrack.GetMomentum().mag2();
  float mainPartMass = mainG4Particle->GetPDGMass();

  // get the PunchThroughParticle class
  PunchThroughParticle *p = m_particles.at(pdg);

  const double initEnergy = std::sqrt( mainMomMag2 + mainPartMass*mainPartMass );

  // (1.) decide if we do correlation or not
  double rand = CLHEP::RandFlat::shoot(rndmEngine)
    *(p->getFullCorrelationEnergy() - p->getMinCorrelationEnergy())
    + p->getMinCorrelationEnergy();

  // if initEnergy less than random corr energy (meaning threshold for min energy for doing correlation not passed) 
  if ( initEnergy < rand )
  {
    // here we do not do correlation
    return getAllParticles(g4PrimaryTrack, secKinematicsMapVect, rndmEngine, pdg, interpEnergy, interpEta);
  }

  // (2.) if this point is reached, we do correlation
  // decide which 2d correlation histogram to use
  double *histDomains = p->getCorrelationHistDomains();
  TH2F *hist2d = nullptr;
  // compute the center values of the lowE and highE
  // correlation histogram domains
  if ( initEnergy <  histDomains[1])
  {
    // initial energy lower than border between lowEnergy and highEnergy histogram domain
    //  --> choose lowEnergy correlation histogram
    hist2d = p->getCorrelationLowEHist();
  }
  else
  {
    double rand = CLHEP::RandFlat::shoot(rndmEngine)*(histDomains[2]-histDomains[1]) + histDomains[1];
    hist2d = ( initEnergy < rand) ? p->getCorrelationLowEHist() : p->getCorrelationHighEHist();
  }

  // get the correlation 2d histogram
  // now find out where on the x-axis the the bin for number of
  // correlated particles is
  Int_t xbin = hist2d->GetXaxis()->FindFixBin(corrParticles);
  int numParticles = 0;
  int maxParticles = p->getMaxNumParticles();
  // now the distribution along the y-axis is a PDF for the number
  // of 'pdg' particles
  do
  {
    double rand = CLHEP::RandFlat::shoot(rndmEngine);
    double sum = 0.;
    for ( int ybin = 1; ybin <= hist2d->GetNbinsY(); ybin++ )
    {
      sum += hist2d->GetBinContent(xbin, ybin);
      // check if we choose the current bin or not
      if ( sum >= rand )
        {
          numParticles = ybin - 1;
          break;
        }
    }
    // scale the number of particles is requested
    numParticles = lround( numParticles * p->getNumParticlesFactor() );
  }
  while ( (maxParticles >= 0.) && (numParticles > maxParticles) );

  // finally create this exact number of particles
  return getAllParticles(g4PrimaryTrack, secKinematicsMapVect, rndmEngine, pdg, interpEnergy, interpEta, numParticles);
}

std::map<std::string, double> PunchThroughG4Tool::getOneParticleKinematics(CLHEP::HepRandomEngine* rndmEngine, int secondaryPDG, float initParticleTheta, float initParticlePhi, double interpEnergy, double interpEta) const
{
  // get a local copy of the needed punch-through particle class
  PunchThroughParticle *p = m_particles.at(secondaryPDG);

  // (0.) get the pca / cdf group based on pdgId and eta, eta times 100, e.g eta -4 to 4 is from eta -400 to 400
  int pcaCdfIterator = passedParamIterator(secondaryPDG, interpEta*100, m_xml_info_pca); //pca and cdf info should be of same size

  ATH_MSG_DEBUG("[PunchThroughG4Tool] passedPCAIterator ==> pcaCdfIterator = "<< pcaCdfIterator <<" , pdg = "<< secondaryPDG <<" , interpEnergy = "<< interpEnergy <<" MeV, interpEta(*100) = "<< interpEta*100);

  // (1.) decide if we create a particle or an anti-particle,
  // this is from m_doAntiParticles properties
  int anti = 1;
  if ( p->getdoAnti() )
  {
    // get a random-value
    double rand = CLHEP::RandFlat::shoot(rndmEngine);
    // 50/50 chance to be a particle or its anti-particle
    if (rand > 0.5) anti = -1;
  }

  // (2.) get the right punch-through distributions
  // prepare the function arguments for the PunchThroughPDFCreator class
  std::vector<int> parInitEnergyEta;
  parInitEnergyEta.push_back( std::round(interpEnergy) );
  parInitEnergyEta.push_back( std::round(interpEta*100) );

  //initialise variables to store punch through particle kinematics
  double energy = 0.;
  double deltaTheta = 0.;
  double deltaPhi = 0.;
  double momDeltaTheta = 0.;
  double momDeltaPhi = 0.;

  double principal_component_0 = 0.;
  double principal_component_1 = 0.;
  double principal_component_2 = 0.;
  double principal_component_3 = 0.;
  double principal_component_4 = 0.;
  std::vector<double> transformed_variables;

  principal_component_0 = p->getPCA0PDF()->getRand(rndmEngine, parInitEnergyEta);
  principal_component_1 = p->getPCA1PDF()->getRand(rndmEngine, parInitEnergyEta);
  principal_component_2 = p->getPCA2PDF()->getRand(rndmEngine, parInitEnergyEta);
  principal_component_3 = p->getPCA3PDF()->getRand(rndmEngine, parInitEnergyEta);
  principal_component_4 = p->getPCA4PDF()->getRand(rndmEngine, parInitEnergyEta);

  ATH_MSG_DEBUG("[PunchThroughG4Tool] Drawn punch through kinematics PCA components: PCA0 = "<< principal_component_0 <<" PCA1 = "<< principal_component_1 <<" PCA2 = "<< principal_component_2 <<" PCA3 = "<< principal_component_3 <<" PCA4 = "<< principal_component_4 );

  std::vector<double> principal_components {
          principal_component_0, 
          principal_component_1, 
          principal_component_2, 
          principal_component_3, 
          principal_component_4
   };

  transformed_variables = inversePCA(pcaCdfIterator,principal_components);

  energy = inverseCdfTransform(transformed_variables.at(0), m_variable0_inverse_cdf[pcaCdfIterator]);
  deltaTheta = inverseCdfTransform(transformed_variables.at(1), m_variable1_inverse_cdf[pcaCdfIterator]);
  deltaPhi = inverseCdfTransform(transformed_variables.at(2), m_variable2_inverse_cdf[pcaCdfIterator]);
  momDeltaTheta = inverseCdfTransform(transformed_variables.at(3), m_variable3_inverse_cdf[pcaCdfIterator]);
  momDeltaPhi = inverseCdfTransform(transformed_variables.at(4), m_variable4_inverse_cdf[pcaCdfIterator]);

  ATH_MSG_DEBUG("[PunchThroughG4Tool] Transformed punch through kinematics: energy = "<< energy <<" MeV deltaTheta = "<< deltaTheta <<" deltaPhi = "<< deltaPhi <<" momDeltaTheta = "<< momDeltaTheta <<" momDeltaPhi = "<< momDeltaPhi );

  energy *= p->getEnergyFactor(); // scale the energy if requested

  // safety: if energy less than minimum energy, add a small energy (10MeV to it to continue)
  if (energy < p->getMinEnergy()) {
    energy = p->getMinEnergy() + 10;
  }

  // (2.2) get the particles DeltaTheta relative to the incoming particle
  double theta = 0;
  // loop to keep theta within range [0,PI]
  do
  {
    // decide if delta positive/negative
    deltaTheta *=  ( CLHEP::RandFlat::shoot(rndmEngine) > 0.5 ) ? 1. : -1.;

    // calculate the exact theta value of the later created
    // punch-through particle
    theta = initParticleTheta + deltaTheta*p->getPosAngleFactor();
  }
  while ( (theta > M_PI) || (theta < 0.) );

  // (2.3) get the particle's delta phi relative to the incoming particle
  deltaPhi *=  ( CLHEP::RandFlat::shoot(rndmEngine) > 0.5 ) ? 1. : -1.;

  // keep phi within range [-PI,PI]
  double phi = initParticlePhi + deltaPhi*p->getPosAngleFactor();
  while ( std::fabs(phi) > 2*M_PI) phi /= 2.;
  while (phi >  M_PI) phi -= 2*M_PI;
  while (phi < -M_PI) phi += 2*M_PI;

  // (2.4) get the particle momentum delta theta, relative to its position
  //
  // loop to keep momTheta within range [0,PI]
  double momTheta = 0.;
  do
  {
    // decide if delta positive/negative
    momDeltaTheta *=  ( CLHEP::RandFlat::shoot(rndmEngine) > 0.5 ) ? 1. : -1.;
    // calculate the exact momentum theta value of the later created
    // punch-through particle
    momTheta = theta + momDeltaTheta*p->getMomAngleFactor();
  }
  while ( (momTheta > M_PI) || (momTheta < 0.) );

  // (2.5) get the particle momentum delta phi, relative to its position
  momDeltaPhi *=  ( CLHEP::RandFlat::shoot(rndmEngine) > 0.5 ) ? 1. : -1.;

  double momPhi = phi + momDeltaPhi*p->getMomAngleFactor();
  // keep momPhi within range [-PI,PI]
  while ( std::fabs(momPhi) > 2*M_PI) momPhi /= 2.;
  while (momPhi >  M_PI) momPhi -= 2*M_PI;
  while (momPhi < -M_PI) momPhi += 2*M_PI;

  // store the kinematics in a map and return it
  std::map<std::string, double> secondaryKinematicsMap;
  secondaryKinematicsMap.insert({ "anti"             , anti });
  secondaryKinematicsMap.insert({ "secondaryPDG"     , secondaryPDG });
  secondaryKinematicsMap.insert({ "energy"           , energy });
  secondaryKinematicsMap.insert({ "theta"            , theta });
  secondaryKinematicsMap.insert({ "phi"              , phi });
  secondaryKinematicsMap.insert({ "momTheta"         , momTheta });
  secondaryKinematicsMap.insert({ "momPhi"           , momPhi });

  return secondaryKinematicsMap;
}

std::vector<std::map<std::string, double>> PunchThroughG4Tool::checkEnergySumFromSecondaries(double mainEnergyInit, std::vector<std::map<std::string, double>> &secKinematicsMapVect){
  //initialize variables
  double energy;
  double totEnergySecondaries = 0;

  // Check energy conservation
  // Step 1: sum all of the energies from the vector of secondaries (map) together, to initialize procedure later  
  for (std::size_t i = 0; i < secKinematicsMapVect.size(); i++) {
    energy = secKinematicsMapVect[i].at("energy");
    totEnergySecondaries += energy;
  }

  // Step 2: Sort the vector based on "energy" in descending order using lambda expression
  std::sort(
      secKinematicsMapVect.begin(),
      secKinematicsMapVect.end(),
      [](const std::map<std::string, double>& lhs, const std::map<std::string, double>& rhs) {
          return lhs.at("energy") > rhs.at("energy");
      }
  );

  // Just for printing here
  if(totEnergySecondaries > mainEnergyInit){
      ATH_MSG_DEBUG("[PunchThroughG4Tool::checkEnergySumFromSecondaries] Case where energy of created secondaries more than parent track identified! ");
      ATH_MSG_DEBUG("[PunchThroughG4Tool::checkEnergySumFromSecondaries] ==> TotalSecondariesEnergy = " << totEnergySecondaries << ", ParentEnergy = "<< mainEnergyInit);
  }

  // Step 3: Adjust the energy and remove secondary with lowest energy to ensure total energy of secondaries <= mainEnergyInit
  while (totEnergySecondaries > mainEnergyInit && !secKinematicsMapVect.empty()) {
      // Get the last map in the vector (which has the lowest energy due to sorting)
      std::map<std::string, double> lastMap = secKinematicsMapVect.back();
      
      // Get the energy value of the last map
      double lastEnergy = lastMap.at("energy");
      
      // Remove the last map from the vector
      secKinematicsMapVect.pop_back();
      
      // Subtract the removed energy from totEnergySecondaries
      totEnergySecondaries -= lastEnergy;
  }

  //finally return the secKinematicsMapVect after the checks
  return secKinematicsMapVect;
}

void PunchThroughG4Tool::createAllSecondaryTracks(G4ParticleTable &ptable, G4FastStep& fastStep, const G4Track& g4PrimaryTrack, std::vector<std::map<std::string, double>> &secKinematicsMapVect, G4TrackVector& secTrackCont, std::vector<double> caloMSVars){
  // Get Geant4 particle definition
  const G4ParticleDefinition* mainG4Particle = g4PrimaryTrack.GetDefinition();
  float mainMomMag2 = g4PrimaryTrack.GetMomentum().mag2();
  float mainPartMass = mainG4Particle->GetPDGMass();
  double mainEnergyInit = std::sqrt(mainMomMag2 + mainPartMass * mainPartMass);  

  // Energy conservation: call checkEnergySumFromSecondaries to check/update secKinematicsMapVect
  secKinematicsMapVect = checkEnergySumFromSecondaries(mainEnergyInit, secKinematicsMapVect);

  // set number of secondary tracks to be produced
  int numSecondaries = secKinematicsMapVect.size();

  if(numSecondaries>0){ //safety
    // set number of secondary tracks from numSecondaries above
    fastStep.SetNumberOfSecondaryTracks(numSecondaries);

    // Get current (global) time from original main G4Track
    double currentTime = g4PrimaryTrack.GetGlobalTime();

    // create the secondaries given the number of secondaries to be created
    for(int i=0;i < numSecondaries; i++){
      int anti         = (int)(secKinematicsMapVect[i].at("anti"));  
      int secondaryPDG = (int)(secKinematicsMapVect[i].at("secondaryPDG"));  
      int signedPDG    = secondaryPDG*anti;  
      double energy    = secKinematicsMapVect[i].at("energy");
      double theta     = secKinematicsMapVect[i].at("theta");
      double phi       = secKinematicsMapVect[i].at("phi");
      double momTheta  = secKinematicsMapVect[i].at("momTheta");
      double momPhi    = secKinematicsMapVect[i].at("momPhi");

      // (**) finally create the punch-through particle as a G4Track (secondary particle track)    
      ATH_MSG_DEBUG("[PunchThroughG4Tool::createAllSecondaryTracks] createSecondaryTrack input parameters: currentTime = " << currentTime << " anti? = "<< anti <<" signedPDG = "<< signedPDG <<" energy = "<< energy <<" theta = "<< theta <<" phi = "<< phi <<" momTheta = "<< momTheta << " momPhi " << momPhi);
      G4Track* newSecTrack = createSecondaryTrack( ptable, fastStep, currentTime, signedPDG, energy, theta, phi, momTheta, momPhi, caloMSVars);

      // if something went wrong and the secondary track is simply not created
      if (!newSecTrack)
      {
        G4Exception("[PunchThroughG4Tool::createAllSecondaryTracks]", "ExceptionError", FatalException, "something went wrong while creating punch-through particle tracks");
        return;
      }

      // add this secondary track to the g4trackvector
      secTrackCont.push_back( newSecTrack );
    }
  } 

  //printout: fastStep.DumpInfo()
}

G4Track* PunchThroughG4Tool::createSecondaryTrack( G4ParticleTable &ptable, G4FastStep& fastStep, double currentTime, int secondarySignedPDG, 
                                                       double energy, double theta, double phi,double momTheta, double momPhi, std::vector<double> caloMSVars)
{
  //initialize to nullptr
  G4Track *newSecTrack = nullptr;

  G4ParticleDefinition* secG4Particle = ptable.FindParticle(secondarySignedPDG);
  double mass = secG4Particle->GetPDGMass();

  // the intersection point with Calo-MS surface
  G4ThreeVector posVec = punchTroughPosPropagator(theta, phi, caloMSVars[0], caloMSVars[1], caloMSVars[2], caloMSVars[3]);

  // set up the real punch-through particle at this position
  // set up the momentum vector of this particle as a GlobalMomentum
  // by using the given energy and mass of the particle and also using
  // the given theta and phi
  G4ThreeVector momVec;  
  double momMag = std::sqrt(energy*energy - mass*mass);
  momVec.setRThetaPhi(momMag,momTheta,momPhi); 
  ATH_MSG_DEBUG("[PunchThroughG4Tool]::createSecondaryTrack] setRThetaPhi pre input parameters: energy =  " << energy <<" mass = "<< mass);
  ATH_MSG_DEBUG("[PunchThroughG4Tool]::createSecondaryTrack] setRThetaPhi input parameters: std::sqrt(energy*energy - mass*mass) = " << std::sqrt(energy*energy - mass*mass) << " momTheta = "<< momTheta <<" momPhi = "<< momPhi);

  // create dynamic particle
  G4DynamicParticle dynParticle(secG4Particle,momVec);
  newSecTrack = fastStep.CreateSecondaryTrack(dynParticle, posVec, currentTime, false); // use position in global coordinates instead

  return newSecTrack;
}

double PunchThroughG4Tool::normal_cdf(double x) {
    return  0.5 * TMath::Erfc(-x * M_SQRT1_2);
}

std::vector<double> PunchThroughG4Tool::dotProduct(const std::vector<std::vector<double>> &m, const std::vector<double> &v) 
{
    std::vector<double> result;
    result.reserve(m.size());
    for (const auto& r : m){
        result.push_back(std::inner_product(v.begin(), v.end(), r.begin(), 0.0));
    }

    return result;
}

template<typename T>
std::vector<T> str_to_list(const std::string_view str)
{
    constexpr char delimiters = ',';
    std::vector<T> tokens;
    // Skip delimiters at beginning.
    std::string_view::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string_view::size_type pos = str.find_first_of(delimiters, lastPos);

    while (std::string_view::npos != pos || std::string_view::npos != lastPos) {
        // Found a token, add it to the vector.
        std::string_view numbStr = str.substr(lastPos, pos - lastPos);
        T num = -9999;
        std::from_chars(numbStr.data(), numbStr.data() + numbStr.size(), num);
        tokens.push_back(num);
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
    return tokens;
}

int PunchThroughG4Tool::passedParamIterator(int pid, double eta, const std::vector<std::map<std::string, std::string>> &mapvect) const
{
    //convert the pid to absolute value and string for query
    int pidStrSingle = std::abs(pid);
    //STEP 1
    //filter items matching pid first

    for (unsigned int i = 0; i < mapvect.size(); i++){
        const std::string &pidStr = mapvect[i].at("pidStr");
        auto v = str_to_list<int>(pidStr);
        if(std::find(v.begin(), v.end(),pidStrSingle)==v.end()) continue;
        const std::string &etaMinsStr = mapvect[i].at("etaMins");
        const std::string &etaMaxsStr = mapvect[i].at("etaMaxs");
        std::vector<double> etaMinsVect = str_to_list<double>(etaMinsStr);
        std::vector<double> etaMaxsVect = str_to_list<double>(etaMaxsStr);
        assert(etaMaxsVect.size() == etaMinsVect.size());
        for (unsigned int j = 0; j < etaMinsVect.size(); j++){ // assume size etaMinsVect == etaMaxsVect
          double etaMinToCompare = etaMinsVect[j];
          double etaMaxToCompare = etaMaxsVect[j];
          if((eta >= etaMinToCompare) && (eta < etaMaxToCompare)){
            //PASS CONDITION
            //then choose the passing one and note it's iterator
            return (i); //in case more than 1 match (ambiguous case)
          }
        }
    }
    return 0;
}

std::vector<std::map<std::string, std::string>> PunchThroughG4Tool::getInfoMap(const std::string& mainNode, const std::string &xmlFilePath){
    // Initialize pointers
    xmlDocPtr doc;
    xmlChar* xmlBuff = nullptr; 

    std::vector<std::map<std::string, std::string>>  xml_info;
    doc = xmlParseFile( xmlFilePath.c_str() );

    //check info first
    for( xmlNodePtr nodeRoot = doc->children; nodeRoot != nullptr; nodeRoot = nodeRoot->next) {
        if (xmlStrEqual( nodeRoot->name, BAD_CAST mainNode.c_str() )) {
            for( xmlNodePtr nodeRootChild = nodeRoot->children; nodeRootChild != nullptr; nodeRootChild = nodeRootChild->next ) {
                if (xmlStrEqual( nodeRootChild->name, BAD_CAST "info" )) {
                    if (nodeRootChild->children != NULL) {
                        for( xmlNodePtr infoNode = nodeRootChild->children; infoNode != nullptr; infoNode = infoNode->next) {
                            if(xmlStrEqual( infoNode->name, BAD_CAST "item" )){
                                std::map<std::string, std::string>  xml_info_item;

                                if ((xmlBuff = xmlGetProp(infoNode, BAD_CAST "name")) != nullptr) {
                                    xml_info_item.insert({"name", (const char*)xmlBuff});
                                }
                                if ((xmlBuff = xmlGetProp(infoNode, BAD_CAST "etaMins")) != nullptr) {
                                    xml_info_item.insert({"etaMins", (const char*)xmlBuff});
                                }
                                if ((xmlBuff = xmlGetProp(infoNode, BAD_CAST "etaMaxs")) != nullptr) {
                                    xml_info_item.insert({"etaMaxs", (const char*)xmlBuff});
                                }
                                if ((xmlBuff = xmlGetProp(infoNode, BAD_CAST "pidStr")) != nullptr) {
                                    xml_info_item.insert({"pidStr", (const char*)xmlBuff});
                                }

                                xml_info.push_back(xml_info_item);                                
                            }
                        }
                    }
                }
            }
        }
    }

    // free memory when done 
    xmlFreeDoc(doc);

    return xml_info;  
}

std::vector<double> PunchThroughG4Tool::inversePCA(int pcaCdfIterator, std::vector<double> &variables) const
{
    std::vector<double> transformed_variables = dotProduct(m_inverse_PCA_matrix[pcaCdfIterator], variables);

    std::transform (transformed_variables.begin(), transformed_variables.end(), m_PCA_means[pcaCdfIterator].begin(), transformed_variables.begin(), std::plus<double>()); // + means

    return transformed_variables;
}

StatusCode PunchThroughG4Tool::initializeInversePCA(const std::string & inversePCAConfigFile){
    // Initialize pointers
    xmlDocPtr doc;
    xmlChar* xmlBuff = nullptr; 

    doc = xmlParseFile( inversePCAConfigFile.c_str() );

    ATH_MSG_DEBUG( "[PunchThroughG4Tool] Loading inversePCA: " << inversePCAConfigFile);

    //check info first
    m_xml_info_pca = getInfoMap("PCAinverse",inversePCAConfigFile);

    //do the saving
    for (unsigned int i = 0; i < m_xml_info_pca.size(); i++) {
        std::vector<std::vector<double>> PCA_matrix;
        ATH_MSG_DEBUG( "[PunchThroughG4Tool] m_xml_info_pca[" << i << "].at('name') = " << m_xml_info_pca[i].at("name"));

        for( xmlNodePtr nodeRoot = doc->children; nodeRoot != nullptr; nodeRoot = nodeRoot->next) {
            if (xmlStrEqual( nodeRoot->name, BAD_CAST "PCAinverse" )) {
                for( xmlNodePtr nodePCAinverse = nodeRoot->children; nodePCAinverse != nullptr; nodePCAinverse = nodePCAinverse->next ) {

                    if (xmlStrEqual( nodePCAinverse->name, BAD_CAST m_xml_info_pca[i].at("name").c_str() )) {
                        if (nodePCAinverse->children != NULL) {
                            for( xmlNodePtr pcaNode = nodePCAinverse->children; pcaNode != nullptr; pcaNode = pcaNode->next) {

                                if (xmlStrEqual( pcaNode->name, BAD_CAST "PCAmatrix" )) {
                                  std::vector<double> PCA_matrix_row;
                                  for (int i = 0; i <= 4; ++i) {
                                      std::string propName = "comp_" + std::to_string(i); // Dynamically create property name
                                      if ((xmlBuff = xmlGetProp(pcaNode, BAD_CAST propName.c_str())) != nullptr) {
                                          PCA_matrix_row.push_back(atof((const char*)xmlBuff)); // Convert and push to the row
                                      }
                                  }
                                  PCA_matrix.push_back(PCA_matrix_row);          
                                }
                                else if (xmlStrEqual( pcaNode->name, BAD_CAST "PCAmeans" )) {
                                  std::vector<double> PCA_means_row;
                                  for (int i = 0; i <= 4; ++i) {
                                      std::string propName = "mean_" + std::to_string(i); // Dynamically create property name
                                      if ((xmlBuff = xmlGetProp(pcaNode, BAD_CAST propName.c_str())) != nullptr) {
                                          PCA_means_row.push_back(atof((const char*)xmlBuff)); // Convert and push to the row
                                      }
                                  }
                                  m_PCA_means.push_back(PCA_means_row);  
                                }

                            }

                        }
                    }

                }
            }
        }
        m_inverse_PCA_matrix.push_back(PCA_matrix);
    }
    
    // free memory when done 
    xmlFreeDoc(doc);

    return StatusCode::SUCCESS;
}

StatusCode PunchThroughG4Tool::initializeInverseCDF(const std::string & inverseCdfConfigFile){
    std::map<double, double>  variable0_inverse_cdf_row;
    std::map<double, double>  variable1_inverse_cdf_row;
    std::map<double, double>  variable2_inverse_cdf_row;
    std::map<double, double>  variable3_inverse_cdf_row;
    std::map<double, double>  variable4_inverse_cdf_row;

    // Initialize pointers
    xmlDocPtr doc;

    //parse xml that contains config for inverse CDF for each of punch through particle kinematics

    doc = xmlParseFile( inverseCdfConfigFile.c_str() );

    ATH_MSG_DEBUG( "[PunchThroughG4Tool] Loading inverse CDF: " << inverseCdfConfigFile);

    //check info first
    m_xml_info_cdf = getInfoMap("CDFMappings",inverseCdfConfigFile);

    //do the saving
    for (unsigned int i = 0; i < m_xml_info_cdf.size(); i++) {
        ATH_MSG_DEBUG( "[PunchThroughG4Tool] m_xml_info_cdf[" << i << "].at('name') = " << m_xml_info_cdf[i].at("name"));

        for( xmlNodePtr nodeRoot = doc->children; nodeRoot != nullptr; nodeRoot = nodeRoot->next) {
            if (xmlStrEqual( nodeRoot->name, BAD_CAST "CDFMappings" )) {
                for( xmlNodePtr typeMappings = nodeRoot->children; typeMappings != nullptr; typeMappings = typeMappings->next ) {
                    if (xmlStrEqual( typeMappings->name, BAD_CAST m_xml_info_cdf[i].at("name").c_str() )) {
                        if (typeMappings->children != NULL) {
                            for( xmlNodePtr nodeMappings = typeMappings->children; nodeMappings != nullptr; nodeMappings = nodeMappings->next) {

                                if (xmlStrEqual( nodeMappings->name, BAD_CAST "variable0" )) {
                                    variable0_inverse_cdf_row = getVariableCDFmappings(nodeMappings);
                                }
                                else if (xmlStrEqual( nodeMappings->name, BAD_CAST "variable1" )) {
                                    variable1_inverse_cdf_row = getVariableCDFmappings(nodeMappings);
                                }
                                else if (xmlStrEqual( nodeMappings->name, BAD_CAST "variable2" )) {
                                    variable2_inverse_cdf_row = getVariableCDFmappings(nodeMappings);
                                }
                                else if (xmlStrEqual( nodeMappings->name, BAD_CAST "variable3" )) {
                                    variable3_inverse_cdf_row = getVariableCDFmappings(nodeMappings);
                                }
                                else if (xmlStrEqual( nodeMappings->name, BAD_CAST "variable4" )) {
                                    variable4_inverse_cdf_row = getVariableCDFmappings(nodeMappings);
                                }
                            }

                        }
                    }
                }
            }
        }
        m_variable0_inverse_cdf.push_back(variable0_inverse_cdf_row);
        m_variable1_inverse_cdf.push_back(variable1_inverse_cdf_row);
        m_variable2_inverse_cdf.push_back(variable2_inverse_cdf_row);
        m_variable3_inverse_cdf.push_back(variable3_inverse_cdf_row);
        m_variable4_inverse_cdf.push_back(variable4_inverse_cdf_row);
    }

    // free memory when done 
    xmlFreeDoc(doc);

    return StatusCode::SUCCESS;
}

std::map<double, double> PunchThroughG4Tool::getVariableCDFmappings(xmlNodePtr& nodeParent){
    std::map<double, double>  mappings;
    xmlChar* xmlBuff = nullptr; 
    double ref = -1;
    double quant = -1;
    for( xmlNodePtr node = nodeParent->children; node != nullptr; node = node->next ) {
        //Get min and max values that we normalise values to
        if (xmlStrEqual( node->name, BAD_CAST "CDFmap" )) {
            if ((xmlBuff = xmlGetProp(node, BAD_CAST "ref")) != nullptr) {
              ref = atof( (const char*) xmlBuff );
            }
            if ((xmlBuff = xmlGetProp(node, BAD_CAST "quant")) != nullptr) {
              quant = atof( (const char*) xmlBuff );
            }
            mappings.insert(std::pair<double, double>(ref, quant) );
        }
    }

    return mappings;
}

double PunchThroughG4Tool::inverseCdfTransform(double variable, std::map<double, double> inverse_cdf_map) {

    double norm_cdf = normal_cdf(variable);

    auto upper = inverse_cdf_map.upper_bound(norm_cdf);
    auto lower = upper--;

    double m = (upper->second - lower->second)/(upper->first - lower->first);
    double c = lower->second - m * lower->first;
    double transformed = m * norm_cdf + c;

    return transformed;

}

double PunchThroughG4Tool::interpolateEnergy(const double &energy, CLHEP::HepRandomEngine* rndmEngine) const{

    ATH_MSG_DEBUG("[PunchThroughG4Tool] interpolating incoming energy: " << energy);

    std::string energyPointsString;
    for (auto element:m_energyPoints){
        energyPointsString += std::to_string(element) + " ";
    }

    ATH_MSG_DEBUG("[PunchThroughG4Tool] available energy points: " << energyPointsString);

    auto const upperEnergy = std::upper_bound(m_energyPoints.begin(), m_energyPoints.end(), energy);

    if(upperEnergy == m_etaPoints.end()){ //if no energy greater than input energy, choose greatest energy
        ATH_MSG_DEBUG("[PunchThroughG4Tool] incoming energy > largest energy point, returning greatest energy point: " << m_energyPoints.back());
        return m_energyPoints.back();
    }
    else if(upperEnergy == m_etaPoints.begin()){ //if smallest energy greater than input energy, choose smallest energy
        ATH_MSG_DEBUG("[PunchThroughG4Tool] incoming energy < smallest energy point, returning smallest energy point: " << *upperEnergy);
        return *upperEnergy;
    }

    ATH_MSG_DEBUG("[PunchThroughG4Tool] energy points upper_bound: "<< *upperEnergy);

    double randomShoot = CLHEP::RandFlat::shoot(rndmEngine);

    ATH_MSG_DEBUG("[PunchThroughG4Tool] Shooting random number: "<< randomShoot);

    double midPoint = *std::prev(upperEnergy)*M_SQRT2;

    if(energy <  midPoint){ //if energy smaller than mid point in log(energy)

        double distance = std::abs(energy - *std::prev(upperEnergy))/((midPoint) - *std::prev(upperEnergy));

        ATH_MSG_DEBUG( "[PunchThroughG4Tool] incoming energy is closest to prev(upper_bound) in log(energy), distance: " << distance );

        if(randomShoot < distance){
            ATH_MSG_DEBUG( "[PunchThroughG4Tool] randomShoot < distance, returning upper_bound " << *upperEnergy );
            return *upperEnergy;
        }
        ATH_MSG_DEBUG( "[PunchThroughG4Tool] randomShoot > distance, returning prev(upper_bound) " << *std::prev(upperEnergy) );

        return *std::prev(upperEnergy);
    }
    else if(energy >  midPoint){ //if energy greater than mid point in log(energy)

        double distance = std::abs(energy - *upperEnergy)/((*upperEnergy - midPoint));

        ATH_MSG_DEBUG( "[PunchThroughG4Tool] incoming energy is closest to upper_bound in log(energy), distance: " << distance );

        if(randomShoot < distance){
            ATH_MSG_DEBUG( "[PunchThroughG4Tool] randomShoot < distance, returning prev(upper_bound) " << *std::prev(upperEnergy) );
            return *std::prev(upperEnergy);
        }
        ATH_MSG_DEBUG( "[PunchThroughG4Tool] randomShoot > distance, returning upper_bound " << *upperEnergy );
        return *upperEnergy;
    }

    return *upperEnergy;
}

double PunchThroughG4Tool::interpolateEta(const double &eta, CLHEP::HepRandomEngine* rndmEngine) const{

    double absEta = std::abs(eta);

    ATH_MSG_DEBUG("[PunchThroughG4Tool] interpolating incoming abs(eta): " << absEta);

    std::string etaPointsString;
    for (auto element:m_etaPoints){
        etaPointsString += std::to_string(element) + " ";
    }

    ATH_MSG_DEBUG("[PunchThroughG4Tool] available eta points: " << etaPointsString);

    auto const upperEta = std::upper_bound(m_etaPoints.begin(), m_etaPoints.end(), absEta);

    if(upperEta == m_etaPoints.end()){
        ATH_MSG_DEBUG("[PunchThroughG4Tool] incoming abs(eta) > largest eta point, returning greatest eta point: " << m_etaPoints.back());
        return m_etaPoints.back();
    }


    ATH_MSG_DEBUG("[PunchThroughG4Tool] eta points upper_bound: "<< *upperEta);

    double randomShoot = CLHEP::RandFlat::shoot(rndmEngine);

    ATH_MSG_DEBUG("[PunchThroughG4Tool] Shooting random number: "<< randomShoot);

    if(std::abs(absEta - *upperEta) <  std::abs(absEta - *std::prev(upperEta))){

        double distance = std::abs(absEta - *upperEta)/((*upperEta - *std::prev(upperEta))/2);

        ATH_MSG_DEBUG( "[PunchThroughG4Tool] abs(eta) is closer to eta points upper_bound, distance: " << distance );

        if(randomShoot > distance){
            ATH_MSG_DEBUG( "[PunchThroughG4Tool] randomShoot > distance, returning upper_bound " << *upperEta );
            return *upperEta;
        }

        ATH_MSG_DEBUG( "[PunchThroughG4Tool] randomShoot < distance, returning prev(upper_bound) " << *std::prev(upperEta) );

        return *std::prev(upperEta);
    }
    else if(std::abs(absEta - *std::prev(upperEta)) <  std::abs(absEta - *upperEta)){

        if(std::prev(upperEta) == m_etaPoints.begin()){
            ATH_MSG_DEBUG( "[PunchThroughG4Tool] prev of upper bound is begin, returning that: " << *std::prev(upperEta) );
            return *std::prev(upperEta);
        }

        double distance = std::abs(absEta - *std::prev(upperEta))/((*std::prev(upperEta) - *std::prev(std::prev(upperEta)))/2);

        ATH_MSG_DEBUG( "[PunchThroughG4Tool] abs(eta) is closer to eta points prev(upper_bound), distance: " << distance );

        if(randomShoot > distance){
            ATH_MSG_DEBUG( "[PunchThroughG4Tool] randomShoot > distance, returning prev(prev(upper_bound)) " << *std::prev(std::prev(upperEta)) );

            return *std::prev(std::prev(upperEta));
        }
        ATH_MSG_DEBUG( "[PunchThroughG4Tool] randomShoot < distance, returning prev(upper_bound) " << *std::prev(upperEta) );

        return *std::prev(upperEta);
    }

    return *std::prev(upperEta);
}

StatusCode
PunchThroughG4Tool::registerPunchThroughParticle(G4ParticleTable &ptable, int pdg, bool doAntiparticle,
                                        double minEnergy, int maxNumParticles, double numParticlesFactor,
                                        double energyFactor, double posAngleFactor, double momAngleFactor)
{
      G4ParticleDefinition* secG4Particle = ptable.FindParticle(std::abs(pdg));
      double restMass = secG4Particle->GetPDGMass();

      // read in the data needed to construct the distributions for the number of punch-through particles
      // (1.) get the distribution function for the number of punch-through particles
      std::unique_ptr<PunchThroughPDFCreator> pdf_num(readLookuptablePDF(pdg, m_fileLookupTable, "FREQ_PDG"));
      if (!pdf_num ) return StatusCode::FAILURE; // return error if something went wrong

      // (2.) get the PDF for the punch-through energy
      std::unique_ptr<PunchThroughPDFCreator> pdf_pca0 (readLookuptablePDF(pdg, m_fileLookupTable, "PCA0_PDG"));
      if (!pdf_pca0)
      {
        return StatusCode::FAILURE; // return error if something went wrong
      }

      // (3.) get the PDF for the punch-through particles difference in
      //      theta compared to the incoming particle
      std::unique_ptr<PunchThroughPDFCreator> pdf_pca1 (readLookuptablePDF(pdg, m_fileLookupTable, "PCA1_PDG"));
      if (!pdf_pca1)
      {
        return StatusCode::FAILURE;
      }

      // (4.) get the PDF for the punch-through particles difference in
      //      phi compared to the incoming particle
      std::unique_ptr<PunchThroughPDFCreator> pdf_pca2 (readLookuptablePDF(pdg, m_fileLookupTable, "PCA2_PDG"));
      if (!pdf_pca2)
      {
        return StatusCode::FAILURE;
      }

      // (5.) get the PDF for the punch-through particle momentum delta theta angle
      std::unique_ptr<PunchThroughPDFCreator> pdf_pca3 (readLookuptablePDF(pdg, m_fileLookupTable, "PCA3_PDG"));
      if (!pdf_pca3)
      {
        return StatusCode::FAILURE;
      }

      // (6.) get the PDF for the punch-through particle momentum delta phi angle
      std::unique_ptr<PunchThroughPDFCreator> pdf_pca4 (readLookuptablePDF(pdg, m_fileLookupTable, "PCA4_PDG"));
      if (!pdf_pca4)
      {
        return StatusCode::FAILURE;
      }

      // (7.) now finally store all this in the right std::map
      PunchThroughParticle *particle = new PunchThroughParticle(pdg, doAntiparticle);
      particle->setNumParticlesPDF(std::move(pdf_num));
      particle->setPCA0PDF(std::move(pdf_pca0));
      particle->setPCA1PDF(std::move(pdf_pca1));
      particle->setPCA2PDF(std::move(pdf_pca2));
      particle->setPCA3PDF(std::move(pdf_pca3));
      particle->setPCA4PDF(std::move(pdf_pca4));

      // (8.) set some additional particle and simulation properties      
      minEnergy = ( minEnergy > restMass ) ? minEnergy : restMass;
      particle->setMinEnergy(minEnergy);
      particle->setMaxNumParticles(maxNumParticles);
      particle->setNumParticlesFactor(numParticlesFactor);
      particle->setEnergyFactor(energyFactor);
      particle->setPosAngleFactor(posAngleFactor);
      particle->setMomAngleFactor(momAngleFactor);

      // (9.) insert this PunchThroughParticle instance into the std::map class member
      m_particles[pdg] = particle;

      return StatusCode::SUCCESS;
}

StatusCode PunchThroughG4Tool::registerCorrelation(int pdgID1, int pdgID2,
                                                      double minCorrEnergy, double fullCorrEnergy)
{
  // find the given pdgs in the registered particle ids
  std::map<int, PunchThroughParticle*>::iterator location1 = m_particles.find(pdgID1);
  std::map<int, PunchThroughParticle*>::iterator location2 = m_particles.find(pdgID2);

  // if at least one of the given pdgs was not registered yet -> return an error
  if ( (location1 == m_particles.end()) || (location2 == m_particles.end()) )
    return StatusCode::FAILURE;

  // now look for the correlation histograms
  std::stringstream name;
  name << "NumExitCorrelations/x_PDG" << std::abs(pdgID1) << "__y_PDG" << std::abs(pdgID2) << "__lowE";
  TH2F *hist1_2_lowE = (TH2F*)m_fileLookupTable->Get(name.str().c_str());
  name.str("");
  name << "NumExitCorrelations/x_PDG" << std::abs(pdgID1) << "__y_PDG" << std::abs(pdgID2) << "__highE";
  TH2F *hist1_2_highE = (TH2F*)m_fileLookupTable->Get(name.str().c_str());
  name.str("");
  name << "NumExitCorrelations/x_PDG" << std::abs(pdgID2) << "__y_PDG" << std::abs(pdgID1) << "__lowE";
  TH2F *hist2_1_lowE = (TH2F*)m_fileLookupTable->Get(name.str().c_str());
  name.str("");
  name << "NumExitCorrelations/x_PDG" << std::abs(pdgID2) << "__y_PDG" << std::abs(pdgID1) << "__highE";
  TH2F *hist2_1_highE = (TH2F*)m_fileLookupTable->Get(name.str().c_str());
  // check if the histograms exist
  if ( (!hist1_2_lowE) || (!hist2_1_lowE) || (!hist1_2_highE) || (!hist2_1_highE) )
  {
    ATH_MSG_ERROR("[PunchThroughG4Tool] unable to retrieve the correlation data for PDG IDs " << pdgID1 <<  " and " << pdgID2);
    return StatusCode::FAILURE;
  }

  // TODO: if only one of the two histograms exists, create the other one
  //       by mirroring the data
  const double lowE = getFloatAfterPatternInStr( hist1_2_lowE->GetTitle(), "elow_");
  const double midE = getFloatAfterPatternInStr( hist1_2_lowE->GetTitle(), "ehigh_");
  //TODO: check if the same:
  // double midE = getFloatAfterPatternInStr( hist1_2_lowE->GetTitle(), "elow_");
  const double upperE = getFloatAfterPatternInStr( hist1_2_highE->GetTitle(), "ehigh_");
  // now store the correlation either way  id1->id2 and id2->id1
  m_particles[pdgID1]->setCorrelation(pdgID2, hist2_1_lowE, hist2_1_highE,
                                      minCorrEnergy, fullCorrEnergy,
                                      lowE, midE, upperE);
  m_particles[pdgID2]->setCorrelation(pdgID1, hist1_2_lowE, hist1_2_highE,
                                      minCorrEnergy, fullCorrEnergy,
                                      lowE, midE, upperE);
  return StatusCode::SUCCESS;
}

std::unique_ptr<PunchThroughPDFCreator> PunchThroughG4Tool::readLookuptablePDF(int pdg, TFile* fileLookupTable, const std::string& folderName)
{

      // will hold the PDFcreator class which will be returned at the end
      // this will store the distributions for the punch through particles
      // (as map of energy & eta of the incoming particle)
      //PunchThroughPDFCreator *pdf = new PunchThroughPDFCreator();
      std::unique_ptr<PunchThroughPDFCreator> pdf = std::make_unique<PunchThroughPDFCreator>();

          //Get directory object
          std::stringstream dirName;
          dirName << folderName << pdg;
          pdf->setName(dirName.str().c_str());

          TDirectory * dir = (TDirectory*)fileLookupTable->Get(dirName.str().c_str());
          if(! dir)
          {
            ATH_MSG_ERROR( "[PunchThroughG4Tool] unable to retrieve directory object ("<< folderName << pdg << ")" );
            return nullptr;
          }

          //Get list of all objects in directory
          TIter keyList(dir->GetListOfKeys());
          TKey *key;

          while ((key = (TKey*)keyList())) {

            //Get histogram object from key and its name
            TH1* hist = nullptr;

            std::string histName;
            if(strcmp(key->GetClassName(), "TH1F") == 0){
              hist = (TH1*)key->ReadObj();
              histName = hist->GetName();
            }

            //extract energy and eta from hist name 6 and 1 to position delimeters correctly
            std::string strEnergy = histName.substr( histName.find_first_of('E') + 1, histName.find_first_of('_')-histName.find_first_of('E') - 1 );
            histName.erase(0, histName.find_first_of('_') + 1);
            std::string strEtaMin = histName.substr( histName.find("etaMin") + 6, histName.find_first_of('_') - histName.find("etaMin") - 6 );
            histName.erase(0, histName.find('_') + 1);
            std::string strEtaMax = histName.substr( histName.find("etaMax") + 6, histName.length());

            //create integers to store in map
            const int energy = std::stoi(strEnergy);
            const int etaMin = std::stoi(strEtaMin);

            //Add entry to pdf map
            pdf->addToEnergyEtaHist1DMap(energy, etaMin, hist);

            //create doubles to store energy and eta points for interpolation
            const double energyDbl = static_cast<double>(energy);
            const double etaDbl = static_cast<double>(etaMin)/100.;

            //create vectors to store the eta and energy points, this allows us to interpolate
            if (std::find(m_energyPoints.begin(), m_energyPoints.end(), energyDbl) == m_energyPoints.end()) {
                m_energyPoints.push_back(energyDbl);
            }
            if (std::find(m_etaPoints.begin(), m_etaPoints.end(), etaDbl) == m_etaPoints.end()) {
                m_etaPoints.push_back(etaDbl);
            }

          }

      return pdf;
}

double PunchThroughG4Tool::getFloatAfterPatternInStr(const char *cstr, const char *cpattern)
{
  double num = 0.;

  const std::string_view str( cstr);
  const std::string_view pattern( cpattern);
  const size_t pos = str.find(pattern);

  if ( pos == std::string::npos)
    {
      ATH_MSG_WARNING("[PunchThroughG4Tool] unable to retrieve floating point number from string");
      return -999999999999.;
    }
  const std::string_view substring = str.substr(pos+pattern.length());
  std::from_chars(substring.data(), substring.data() + substring.size(), num);
  return num;
}


G4ThreeVector PunchThroughG4Tool::punchTroughPosPropagator(double theta, double phi, double R1, double R2, double z1, double z2) const
{
  // phi, theta - direction of the punch-through particle coming into calo
  // particle propagates inside the calorimeter along the straight line
  // coordinates of this particles when exiting the calo (on calo-MS boundary)

  double x, y, z, r;

  // cylinders border angles
  const double theta1 = atan (R1 / z1);
  const double theta2 = atan (R1 / z2);
  const double theta3 = atan (R2 / z2);
  //where is the particle

  if (theta >= 0 && theta < theta1)
  {
    z = z1;
    r = std::fabs (z1 * tan(theta));
  }
  else if (theta >= theta1 && theta < theta2)
  {
    z = R1 / tan(theta);
    r = R1;
  }
  else if (theta >= theta2 && theta < theta3)
  {
    z = z2;
    r = std::fabs(z2 * tan(theta));;
  }
  else if (theta >= theta3 && theta < (TMath::Pi() - theta3) )
  {
    z = R2 / tan(theta);
    r = R2;
  }
  else if (theta >= (TMath::Pi() - theta3) && theta < (TMath::Pi() - theta2) )
  {
    z = -z2;
    r = std::fabs(z2 * tan(theta));
  }
  else if (theta >= (TMath::Pi() - theta2) && theta < (TMath::Pi() - theta1) )
  {
    z = R1 / tan(theta);
    r = R1;
  }
  else if (theta >= (TMath::Pi() - theta1) && theta <= TMath::Pi() )
  {
    z = -z1;
    r = std::fabs(z1 * tan(theta));
  }

  //parallel universe
  else
  {
    ATH_MSG_WARNING("[PunchThroughG4Tool::punchTroughPosPropagator] Given theta angle is incorrect, setting particle position to (0, 0, 0)");
    x = 0.0; y = 0.0; z = 0.0; r = 0.0;
  }

  x = r * cos(phi);
  y = r * sin(phi);
  G4ThreeVector posVec(x, y, z);

  ATH_MSG_DEBUG("[PunchThroughG4Tool::punchTroughPosPropagator] position of produced punch-through particle: x = " << x <<" y = "<< y <<" z = "<< z<<" r = "<< posVec.perp() <<"std::sqrt(x^2+y^2) = "<< std::sqrt(x * x + y * y));

  return posVec;
}