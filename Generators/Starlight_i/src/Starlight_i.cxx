/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// -------------------------------------------------------------
// Generators/Starlight_i.cxx Description: Allows the user
// to generate Starlight events and store the result in the
// Transient Store.
//
// AuthorList:
//   Andrzej Olszewski:  Initial Code January 2011
//   Andrzej Olszewski:  Update for Starlight "r193" March 2016
//
// Random seed set via jo
// Random numbers not saved by atlas engine mechanism event by event.

#include "Starlight_i/Starlight_i.h"

#include "CxxUtils/StringParse.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "reportingUtils.h"
#include "starlightconstants.h"
#include "starlightparticlecodes.h"

namespace{
  static const std::string starlight_stream = "STARLIGHT";
}

typedef std::vector<std::string> CommandVector;

Starlight_i::Starlight_i(const std::string& name, ISvcLocator* pSvcLocator):
             GenModule(name,pSvcLocator)
{
}

Starlight_i::~Starlight_i(){
  if (m_starlight) delete m_starlight;
  if (m_event) delete m_event;
}

StatusCode Starlight_i::genInitialize()
{
    // Initialisation of input parameters
    //
    ATH_MSG_INFO( "===> January 20 2011 STARLIGHT INTERFACE VERSION. \n"   );
    ATH_MSG_INFO( "===> STARLIGHT INITIALISING. \n"   );

    //Re-seed the random number stream
    long seeds[7];
    ATHRNG::calculateSeedsMC21(seeds, starlight_stream,  0, m_dsid, m_randomSeed);

    // Create inputParameters and
    // set the users' initialisation parameters choices
    bool res = set_user_params();
    if( !res ) {
      return StatusCode::FAILURE;
    }

    // create the starlight object
    m_starlight = new starlight();
    // Set random generator to prevent crash in tests.
    m_randomGenerator = std::make_shared<randomGenerator>();
    m_randomGenerator->SetSeed(seeds[0]);
    m_starlight->setRandomGenerator(m_randomGenerator.get());
    // set input parameters
    m_starlight->setInputParameters(&m_inputParameters);
    // and initialize
    m_starlight->init();

    // dump events to lhef (needed for QED showering with Pythia8
    if(m_lheOutput){
      ATH_MSG_INFO("===> dumping starlight events to lhef format. \n"  );
      if(!starlight2lhef()) return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode Starlight_i::callGenerator()
{
    if(m_lheOutput) return StatusCode::SUCCESS;
    ATH_MSG_DEBUG( " STARLIGHT generating. \n"   );

    //Re-seed the random number stream
    long seeds[7];
    const EventContext& ctx = Gaudi::Hive::currentContext();
    ATHRNG::calculateSeedsMC21(seeds, starlight_stream,  ctx.eventID().event_number(),
                               m_dsid, m_randomSeed);
    m_randomGenerator->SetSeed(seeds[0]);

    // Generate event
    m_event = new upcEvent;
#ifdef UPCXEVENT_H
    (*m_event) = m_starlight->produceUpcEvent();
#else
    (*m_event) = m_starlight->produceEvent();
#endif

    // update event counter
    ++m_events;

    int numberofTracks = m_event->getParticles()->size();
    int numberOfVertices = 1; //m_event->getVertices()->size();

    ATH_MSG_DEBUG( "EVENT: " << m_events << " "
                   << " with " << numberOfVertices << " vertices "
                   << " and " << numberofTracks << " tracks"  );
    ATH_MSG_DEBUG( "VERTEX: "<< 0. << " " << 0. << " " << 0.
                   << " with " << numberofTracks << " tracks"  );

    int ipart = 0;
    std::vector<starlightParticle>::const_iterator part =
      (m_event->getParticles())->begin();
    for (part = m_event->getParticles()->begin();
         part != m_event->getParticles()->end(); ++part, ++ipart) {
      ATH_MSG_DEBUG( "TRACK: " << " "
                     << starlightParticleCodes::jetsetToGeant((*part).getCharge() * (*part).getPdgCode()) << " "
                     << (*part).GetPx() << " " << (*part).GetPy() << " "<< (*part).GetPz()
                     << " " << m_events << " " << ipart << " " << 0 << " "
                     << (*part).getCharge() * (*part).getPdgCode()  );
    }

    ATH_MSG_DEBUG( " Starlight generating done.  \n"   );

    return StatusCode::SUCCESS;
}

StatusCode
Starlight_i::genFinalize()
{
    ATH_MSG_DEBUG( "  STARLIGHT Ending.  \n"   );

    return StatusCode::SUCCESS;
}

StatusCode
Starlight_i::fillEvt(HepMC::GenEvent* evt)
{
    if(m_lheOutput) return StatusCode::SUCCESS;
    ATH_MSG_DEBUG( "  STARLIGHT Filing.  \n"   );

    // Set the event number
    evt->set_event_number( m_events );

    // Create the event vertex
    HepMC::GenVertexPtr v1 = HepMC::newGenVertexPtr();
    evt->add_vertex( v1 );

    // Loop on all final particles and
    // put them all as outgoing from the event vertex
    int ipart = 0;
    std::vector<starlightParticle>::const_iterator part =
      (m_event->getParticles())->begin();
    double px_tot=0;
    double py_tot=0;
    double pz_tot=0;
    for (part = m_event->getParticles()->begin();
         part != m_event->getParticles()->end(); ++part, ++ipart)
      {
        int pid = (*part).getPdgCode();
        int charge = (*part).getCharge();
        //AO special for pid sign stored in charge
        int pidsign = pid/std::abs(pid);
        int chsign = 0;
        if (charge !=0) chsign = charge/std::abs(charge);
        if( chsign != pidsign && chsign != 0) pid = -pid;

        double px = (*part).GetPx();
        double py = (*part).GetPy();
        double pz = (*part).GetPz();
        double e  = (*part).GetE();
        // mass fix implemented only for muons
        if(std::abs(pid)==13) {
          float mass = m_inputParameters.muonMass();//0.1056583715;// starlightConstants::muonMass;
          e  = std::sqrt(px*px + py*py + pz*pz + mass*mass);
        }
        // mass fix for photons (ALPs)
        if(std::abs(pid)==22) {
          e  = std::sqrt(px*px + py*py + pz*pz);
        }
	
        ATH_MSG_DEBUG( "saving particle " << ipart  );
        px_tot+=px;
        py_tot+=py;
        pz_tot+=pz;

        if(!m_suppressVMdecay) v1->add_particle_out(
                             HepMC::newGenParticlePtr(HepMC::FourVector(px, py, pz, e), pid, 1) );
      }
      if(m_suppressVMdecay) {
        int pid = 113;
        double mass  = 0.770;
        if(m_prodParticleId == 443011 || m_prodParticleId == 443013){
          pid = 443;
          mass  = 3.0969;
        } 
        if(m_prodParticleId == 444011 || m_prodParticleId == 444013){
          pid = 100443;
          mass  = 3.6861;
        } 
        if(m_prodParticleId == 553011 || m_prodParticleId == 553013){
          pid = 553;
          mass  = 9.4604;
        }
        if(m_prodParticleId == 554011 || m_prodParticleId == 554013){
          pid = 100553;
          mass  = 10.023;
        }
        if(m_prodParticleId == 555011 || m_prodParticleId == 555013){
          pid = 200553;
          mass  = 10.355;
        }
        double e  = sqrt(px_tot*px_tot + py_tot*py_tot + pz_tot*pz_tot + mass*mass);
        v1->add_particle_out(
                             HepMC::newGenParticlePtr(HepMC::FourVector(px_tot, py_tot, pz_tot, e), pid, 1) );
      }
    ATH_MSG_DEBUG( "Saved " << ipart << " tracks "  );

    // Convert cm->mm and GeV->MeV
    //
    GeVToMeV(evt);

    return StatusCode::SUCCESS;
}

bool
Starlight_i::starlight2lhef()
{

    std::string lheFilename    = "events.lhe";
    std::ofstream lheStream;
    lheStream.open(lheFilename.c_str(), std::ofstream::trunc);
    if(!lheStream) {
      ATH_MSG_ERROR("error: Failed to open  file "+lheFilename);
      return false;
    }

    lheStream << "<LesHouchesEvents version=\"1.0\">\n";
    lheStream << "<!--\n";
    lheStream << "File generated using Starlight \n";
    lheStream << "-->\n";
    
    float beam_energy=0.;
    // run-3 beam energy (allowing approximate value of 2877 for gamma of the beams)
    if(m_beam1Gamma>2870 && m_beam1Gamma<2880) beam_energy = 2.68e+03;
    // run-2 beam energy
    else beam_energy = 2.51e+03;
    
    lheStream << "<init>\n";
    lheStream << "  13  -13  "<<beam_energy<<"  "<<beam_energy<<"  0  0  0  0  3  1\n";
    lheStream << "  1.000000e+00  0.000000e+00  1.000000e+00   9999\n";
    lheStream << "</init>\n";


    std::unique_ptr<upcEvent> uevent(new upcEvent);

    for(unsigned int i=0; i<m_maxevents; i++) {
      lheStream << "<event>\n";
#ifdef UPCXEVENT_H
      (*uevent) = m_starlight->produceUpcEvent();
#else
      (*uevent) = m_starlight->produceEvent();
#endif
      int ipart = 0;
      CLHEP::HepLorentzVector photon_system(0);
      double ptscale =0;
      std::vector<starlightParticle>::const_iterator part = (uevent->getParticles())->begin();
      for (part = uevent->getParticles()->begin(); part != uevent->getParticles()->end(); ++part, ++ipart)
      {
         CLHEP::HepLorentzVector particle_sl((*part).GetPx(), (*part).GetPy(), (*part).GetPz(), (*part).GetE());
         photon_system += particle_sl;
         ptscale += std::sqrt((*part).GetPx()*(*part).GetPx() + (*part).GetPy()*(*part).GetPy());
           }

      // avg pt is the correct scale here
      ptscale /= static_cast<float> (ipart);
      lheStream << "     4  9999  1.000000e+00  "<<ptscale<<"  7.297e-03  2.569093e-01\n";

      if(m_doTauolappLheFormat){
      lheStream << " -11    -1     0     0     0     0  0.0000000000e+00  0.0000000000e+00  "
                  << photon_system.m()/2.*std::exp(photon_system.rapidity())<<"  "
                  <<photon_system.m()/2.*std::exp(photon_system.rapidity())
                  << "  0.0000000000e+00 0. 9.\n";
      lheStream << " 11    -1     0     0     0     0  0.0000000000e+00  0.0000000000e+00  "
                  << -photon_system.m()/2.*std::exp(-photon_system.rapidity())<<"  "
                  <<photon_system.m()/2.*std::exp(-photon_system.rapidity())
                  << "  0.0000000000e+00 0. 9.\n";
      }

      else{
              lheStream << " 22    -1     0     0     0     0  0.0000000000e+00  0.0000000000e+00  "
                  << photon_system.m()/2.*std::exp(photon_system.rapidity())<<"  "
                  <<photon_system.m()/2.*std::exp(photon_system.rapidity())
                  <<"  0.0000000000e+00 0. 9.\n";
              lheStream << " 22    -1     0     0     0     0  0.0000000000e+00  0.0000000000e+00  "
                  << -photon_system.m()/2.*std::exp(-photon_system.rapidity())<<"  "
                  <<photon_system.m()/2.*std::exp(-photon_system.rapidity())
                  <<"  0.0000000000e+00 0. 9.\n";
      }

      for (part = uevent->getParticles()->begin(); part != uevent->getParticles()->end(); ++part, ++ipart)
      {
        int pid = (*part).getPdgCode();
        int charge = (*part).getCharge();
        //AO special for pid sign stored in charge
        int pidsign = pid/std::abs(pid);
        int chsign = charge/std::abs(charge);
        if( chsign != pidsign ) pid = -pid;

        double px = (*part).GetPx();
        double py = (*part).GetPy();
        double pz = (*part).GetPz();
        double e  = (*part).GetE();
        double mass  = (*part).getMass();
        if(std::abs(pid)==11) mass = m_inputParameters.mel();
        else if(std::abs(pid)==13) mass = m_inputParameters.muonMass();
        else if(std::abs(pid)==15) mass = m_inputParameters.tauMass();

        lheStream << pid<<"     1     1     2     0     0  "<<px<<"  "<<py<<"  "<<pz<<"  "<<e<<"  "<<mass<<"  0. 9.\n";

       }
    lheStream << "</event>\n";
    }


    lheStream << "</LesHouchesEvents>";
    lheStream.close();

    return true;
}

bool Starlight_i::set_user_params()
{
  // Set starlight user initialization parameters

  // write python starlight config parameters to tmp file
  // if external config file not specified
  if (m_configFileName.empty()) {
    m_configFileName = "tmp.slight.in";
    if (!prepare_params_file()) {
      printWarn <<
        "problems initializing input parameters. cannot initialize starlight.";
      return false;
    }
  }

  m_inputParameters.configureFromFile(m_configFileName);
  if (!m_inputParameters.init()) {
    ATH_MSG_WARNING( "problems initializing input parameters. cannot initialize starlight. "  );
    return false;
  }

  return true;
}

bool Starlight_i::prepare_params_file()
{
    // Write initialization parameters to tmp file

    for(CommandVector::iterator i = m_InitializeVector.begin(); i != m_InitializeVector.end(); ++i )
    {
        ATH_MSG_INFO( "  Command is: " << *i  );

        StringParse mystring(*i);
        std::string myparam = mystring.piece<std::string>(1);
        if (myparam == "beam1Z")
        {
          m_beam1Z  = mystring.piece<double>(2);
        }
        else if (myparam == "beam1A")
        {
          m_beam1A  = mystring.piece<double>(2);
        }
        else if (myparam == "beam2Z")
        {
          m_beam2Z  = mystring.piece<double>(2);
        }
        else if (myparam == "beam2A")
        {
          m_beam2A  = mystring.piece<double>(2);
        }
        else if (myparam == "beam1Gamma")
        {
          m_beam1Gamma  = mystring.piece<double>(2);
        }
        else if (myparam == "beam2Gamma")
        {
          m_beam2Gamma  = mystring.piece<double>(2);
        }
        else if (myparam == "maxW")
        {
          m_maxW  = mystring.piece<double>(2);
        }
        else if (myparam == "minW")
        {
          m_minW  = mystring.piece<double>(2);
        }
        else if (myparam == "nmbWBins")
        {
          m_nmbWBins  = mystring.piece<double>(2);
        }
        else if (myparam == "maxRapidity")
        {
          m_maxRapidity  = mystring.piece<double>(2);
        }
        else if (myparam == "nmbRapidityBins")
        {
          m_nmbRapidityBins  = mystring.piece<double>(2);
        }
        else if (myparam == "accCutPt")
        {
          m_accCutPt = mystring.piece<double>(2);
        }
        else if (myparam == "minPt")
        {
          m_minPt = mystring.piece<double>(2);
        }
        else if (myparam == "maxPt")
        {
          m_maxPt = mystring.piece<double>(2);
        }
        else if (myparam == "accCutEta")
        {
          m_accCutEta = mystring.piece<double>(2);
        }
        else if (myparam == "minEta")
        {
          m_minEta = mystring.piece<double>(2);
        }
        else if (myparam == "maxEta")
        {
          m_maxEta = mystring.piece<double>(2);
        }
        else if (myparam == "productionMode")
        {
          m_productionMode  = mystring.piece<double>(2);
        }
        else if (myparam == "axionMass")
        {
          m_axionMass  = mystring.piece<double>(2);
        }
        else if (myparam == "nmbEventsTot")
        {
          m_nmbEventsTot  = mystring.piece<double>(2);
        }
        else if (myparam == "prodParticleId")
        {
          m_prodParticleId  = mystring.piece<double>(2);
        }
        else if (myparam == "randomSeed")
        {
          m_randomSeed  = mystring.piece<double>(2);
        }
        else if (myparam == "outputFormat")
        {
          m_outputFormat  = mystring.piece<double>(2);
        }
        else if (myparam == "beamBreakupMode")
        {
          m_beamBreakupMode  = mystring.piece<double>(2);
        }
        else if (myparam == "interferenceEnabled")
        {
          m_interferenceEnabled  = mystring.piece<double>(2);
        }
        else if (myparam == "interferenceStrength")
        {
          m_interferenceStrength  = mystring.piece<double>(2);
        }
        else if (myparam == "coherentProduction")
        {
          m_coherentProduction = mystring.piece<double>(2);
        }
        else if (myparam == "incoherentFactor")
        {
          m_incoherentFactor  = mystring.piece<double>(2);
        }
        else if (myparam == "maxPtInterference")
        {
          m_maxPtInterference  = mystring.piece<double>(2);
        }
        else if (myparam == "nmbPtBinsInterference")
        {
          m_nmbPtBinsInterference  = mystring.piece<double>(2);
        }
        else if (myparam == "xsecMethod")
        {
          m_xsecMethod = mystring.piece<double>(2);
        }
        else if (myparam == "nThreads")
        {
          m_nThreads = mystring.piece<double>(2);
        }
        else if (myparam == "pythFullRec")
        {
          m_pythFullRec = mystring.piece<double>(2);
        }
        else
        {
          ATH_MSG_ERROR( " ERROR in STARLIGHT INITIALIZATION PARAMETERS  "
                         << myparam << " is an invalid parameter !"  );
            return false;
        }
    }

    std::ofstream configFile;
    configFile.open(m_configFileName.value().c_str());

    configFile << "BEAM_1_Z = " << m_beam1Z << std::endl;
    configFile << "BEAM_1_A = " << m_beam1A << std::endl;
    configFile << "BEAM_2_Z = " << m_beam2Z << std::endl;
    configFile << "BEAM_2_A = " << m_beam2A << std::endl;
    configFile << "BEAM_1_GAMMA = " << m_beam1Gamma << std::endl;
    configFile << "BEAM_2_GAMMA = " << m_beam2Gamma << std::endl;
    configFile << "W_MAX = " << m_maxW << std::endl;
    configFile << "W_MIN = " << m_minW << std::endl;
    configFile << "W_N_BINS = " << m_nmbWBins << std::endl;
    configFile << "RAP_MAX = " << m_maxRapidity << std::endl;
    configFile << "RAP_N_BINS = " << m_nmbRapidityBins << std::endl;
    configFile << "CUT_PT = " << m_accCutPt << std::endl;
    configFile << "PT_MIN = " << m_minPt << std::endl;
    configFile << "PT_MAX = " << m_maxPt << std::endl;
    configFile << "CUT_ETA = " << m_accCutEta << std::endl;
    configFile << "ETA_MIN = " << m_minEta << std::endl;
    configFile << "ETA_MAX = " << m_maxEta << std::endl;
    configFile << "PROD_MODE = " << m_productionMode << std::endl;
    configFile << "AXION_MASS = " << m_axionMass << std::endl;
    configFile << "N_EVENTS = " << m_nmbEventsTot << std::endl;
    configFile << "PROD_PID = " << m_prodParticleId << std::endl;
    configFile << "RND_SEED = " << m_randomSeed << std::endl;
    configFile << "BREAKUP_MODE = " << m_beamBreakupMode << std::endl;
    configFile << "INTERFERENCE = " << m_interferenceEnabled << std::endl;
    configFile << "IF_STRENGTH = " << m_interferenceStrength << std::endl;
    configFile << "INT_PT_MAX = " << m_maxPtInterference << std::endl;
    configFile << "INT_PT_N_BINS = " << m_nmbPtBinsInterference << std::endl;
    configFile << "COHERENT = " << m_coherentProduction << std::endl;
    configFile << "INCO_FACTOR = " << m_incoherentFactor << std::endl;
    configFile << "XSEC_METHOD = " << m_xsecMethod << std::endl;
    configFile << "N_THREADS = " << m_nThreads << std::endl;
    configFile << "PYTHIA_FULL_EVENTRECORD = " << m_pythFullRec << std::endl;

    configFile.close();
    return true;
}
