/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "TruthUtils/HepMCHelpers.h"
#include "HepMC3/GenCrossSection.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "reportingUtils.h"
#include "starlightconstants.h"
#include "starlightparticlecodes.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cmath>

namespace{
    static const std::string starlight_stream = "STARLIGHT";
}

typedef std::vector<std::string> CommandVector;

Starlight_i::Starlight_i(const std::string& name, ISvcLocator* pSvcLocator):
             GenModule(name,pSvcLocator)
{
}

Starlight_i::~Starlight_i() {
}

StatusCode Starlight_i::genInitialize() {
    // Initialisation of input parameters
    //
    ATH_MSG_INFO( "\n STARLIGHT initializing.\n" );

    // Import initialisation parameters from `m_InitializeVector`
    if (!load_user_params())
        return StatusCode::FAILURE;

    // Re-seed the random number stream
    long seeds[7];
    ATHRNG::calculateSeedsMC21(seeds, starlight_stream, 0, m_dsid, m_randomSeedOffset);
    // Set random generator
    m_randomGenerator = std::make_shared<randomGenerator>();
    m_randomGenerator->SetSeed(seeds[0]);  // seed used for generation
    m_inputParameters.setRandomSeed(seeds[0]);  // update for parameter consistency

    // Create and setup the starlight object
    m_starlight = std::make_unique<starlight>();
    m_starlight->setRandomGenerator(m_randomGenerator.get());
    m_starlight->setInputParameters(&m_inputParameters);
    m_starlight->init();

    // Dump events to lhef (neeDed for QED showering with Pythia8
    if (m_lheOutput){
        ATH_MSG_INFO( "===> dumping starlight events to lhef format. \n" );
        if (!starlight2lhef()) return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode Starlight_i::callGenerator() {
    if (m_lheOutput) return StatusCode::SUCCESS;

    ATH_MSG_DEBUG( " STARLIGHT generating. \n" );

    // Re-seed the random number stream
    long seeds[7];
    const EventContext& ctx = Gaudi::Hive::currentContext();
    ATHRNG::calculateSeedsMC21(seeds, starlight_stream, ctx.eventID().event_number(),
                               m_dsid, m_randomSeedOffset);
    m_randomGenerator->SetSeed(seeds[0]);  // seed used for generation
    m_inputParameters.setRandomSeed(seeds[0]);  // update for parameter consistency

    // Generate event
    if (m_productionMode >= starlightConstants::PHOTONUCLEARSINGLE)
        m_event = std::make_unique<upcEvent>(m_starlight->produceUpcEvent());
    else
        m_event = std::make_unique<upcXEvent>(m_starlight->produceEvent());

    // Get particle vector from m_event whether it is upcEvent or upcXEvent
    const std::vector<starlightParticle>* particleCollection = std::visit(
      [](auto& _evt) { return _evt->getParticles(); }, m_event
    );

    // Update event counter
    ++m_events;

    int numberofTracks = particleCollection->size();
    int numberOfVertices = 1; // m_event->getVertices()->size();

    ATH_MSG_DEBUG( "EVENT: " << m_events << " "
                   << " with " << numberOfVertices << " vertices "
                   << " and " << numberofTracks << " tracks"  );
    ATH_MSG_DEBUG( "VERTEX: "<< 0. << " " << 0. << " " << 0.
                   << " with " << numberofTracks << " tracks"  );

    int ipart = 0;
    std::vector<starlightParticle>::const_iterator part =
        particleCollection->begin();
    for (part = particleCollection->begin();
         part != particleCollection->end(); ++part, ++ipart) {
        ATH_MSG_DEBUG( "TRACK: " << " "
                     << starlightParticleCodes::jetsetToGeant((*part).getCharge() * (*part).getPdgCode()) << " "
                     << (*part).GetPx() << " " << (*part).GetPy() << " "<< (*part).GetPz()
                     << " " << m_events << " " << ipart << " " << 0 << " "
                     << (*part).getCharge() * (*part).getPdgCode()  );
    }

    ATH_MSG_DEBUG( " STARLIGHT generating done.\n" );

    return StatusCode::SUCCESS;
}

StatusCode
Starlight_i::genFinalize()
{
    ATH_MSG_INFO( "Adding cross-section of generated sample to MetaData." );
    ATH_MSG_INFO( "MetaData: cross-section (mb) = " << getAcceptedCrossSection()/Gaudi::Units::millibarn );
    ATH_MSG_INFO( "MetaData: generator = Starlight " << STARLIGHTVER );

    ATH_MSG_DEBUG( " STARLIGHT Ending.\n" );

    return StatusCode::SUCCESS;
}

StatusCode
Starlight_i::fillEvt(HepMC::GenEvent* evt) {
    if (m_lheOutput) return StatusCode::SUCCESS;

    ATH_MSG_DEBUG( " STARLIGHT Filing.\n" );

    // Set the event number
    evt->set_event_number( m_events );

    // Create the event vertex
    HepMC::GenVertexPtr v1 = HepMC::newGenVertexPtr();
    evt->add_vertex( v1 );

    // Get all final particles
    const std::vector<starlightParticle>* particleCollection = std::visit(
        [](auto& _evt) { return _evt->getParticles(); }, m_event
    );

    // Loop on all final particles and
    // put them all as outgoing from the event vertex
    int ipart = 0;
    std::vector<starlightParticle>::const_iterator part =
        particleCollection->begin();
    double px_tot=0;
    double py_tot=0;
    double pz_tot=0;
    for (part = particleCollection->begin();
         part != particleCollection->end(); ++part, ++ipart) {

        int pid = (*part).getPdgCode();
        int charge = (*part).getCharge();
        //AO special for pid sign stored in charge
        int pidsign = pid/std::abs(pid);
        int chsign = 0;
        if (charge !=0) chsign = charge/std::abs(charge);
        if (chsign != pidsign && chsign != 0) pid = -pid;

        double px = (*part).GetPx();
        double py = (*part).GetPy();
        double pz = (*part).GetPz();
        double e  = (*part).GetE();
        // Mass fix implemented only for muons
        if (std::abs(pid)==13) {
            float mass = m_inputParameters.muonMass(); //0.1056583715;// starlightConstants::muonMass;
            e  = std::sqrt(px*px + py*py + pz*pz + mass*mass);
        }
        // Mass fix for photons (ALPs)
        if (std::abs(pid)==22) {
            e  = std::sqrt(px*px + py*py + pz*pz);
        }
	
        ATH_MSG_DEBUG( "saving particle " << ipart );
        px_tot+=px;
        py_tot+=py;
        pz_tot+=pz;

        if(!m_suppressVMdecay)
            v1->add_particle_out(HepMC::newGenParticlePtr(HepMC::FourVector(px, py, pz, e), pid, 1));
    }

    if (m_suppressVMdecay) {
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
        v1->add_particle_out(HepMC::newGenParticlePtr(HepMC::FourVector(px_tot, py_tot, pz_tot, e), pid, 1));
    }

    ATH_MSG_DEBUG( "Saved " << ipart << " tracks "  );

    // Add beam particles
    int beam1pid = 1000000000 + m_beam1Z*10000 + m_beam1A*10;
    int beam2pid = 1000000000 + m_beam2Z*10000 + m_beam2A*10;

    HepMC::GenParticlePtr b1 = std::make_shared<HepMC::GenParticle>(HepMC::FourVector(0, 0,  m_beam1_energy.value(), m_beam1_energy.value()), beam1pid, 4);
    HepMC::GenParticlePtr b2 = std::make_shared<HepMC::GenParticle>(HepMC::FourVector(0, 0, -m_beam2_energy.value(), m_beam2_energy.value()), beam2pid, 4);

    v1->add_particle_in(std::move(b1));
    v1->add_particle_in(std::move(b2));

    // Conversion GeV -> MeV to ensure correct units
    evt->set_units(HepMC3::Units::MEV, HepMC3::Units::MM);

    double nAccepted = m_starlight->nmbAccepted();
    double nAttempted = m_starlight->nmbAttempts();
    auto csc = std::make_shared<HepMC3::GenCrossSection>();
    csc->set_cross_section(getAcceptedCrossSection()/Gaudi::Units::picobarn, 0.0, nAccepted, nAttempted); // Save cross-section in pb to HepMC
    evt->set_cross_section(std::move(csc));

    return StatusCode::SUCCESS;
}

bool
Starlight_i::starlight2lhef() {
    std::string lheFilename = "events.lhe";
    std::ofstream lheStream;

    lheStream.open(lheFilename.c_str(), std::ofstream::trunc);
    if (!lheStream) {
        ATH_MSG_ERROR("error: Failed to open  file "+lheFilename);
        return false;
    }

    lheStream << "<LesHouchesEvents version=\"1.0\">\n";
    lheStream << "<!--\n";
    lheStream << "File generated using Starlight \n";
    lheStream << "-->\n";
    
    lheStream << "<init>\n";
    lheStream << "  13  -13  "<<m_beam1_energy.value()<<"  "<<m_beam2_energy.value()<<"  0  0  0  0  3  1\n";

    const std::streampos pos = lheStream.tellp();
    lheStream << std::string(70, ' ') << "\n"; // Reserve one empty line for the cross-section (filled after loop)
    lheStream << "</init>\n";

    // Starting from version r330 uevent can be either
    // upcEvent or upcXEvent depending on the process
    std::variant<
        std::unique_ptr<upcEvent>,
        std::unique_ptr<upcXEvent>> uevent;

    for (unsigned int i=0; i<m_maxevents; i++) {
        lheStream << "<event>\n";

        // Generate event
        if (m_productionMode >= starlightConstants::PHOTONUCLEARSINGLE)
            uevent = std::make_unique<upcEvent>(m_starlight->produceUpcEvent());
        else
            uevent = std::make_unique<upcXEvent>(m_starlight->produceEvent());

        // Get particle vector from m_event whether it is upcEvent or upcXEvent
        const std::vector<starlightParticle>* particleCollection = std::visit(
            [](auto& _evt) { return _evt->getParticles(); }, uevent
        );
  
        int ipart = 0;
        CLHEP::HepLorentzVector photon_system(0);
        double ptscale = 0;
        std::vector<starlightParticle>::const_iterator part = particleCollection->begin();
        for (part = particleCollection->begin(); part != particleCollection->end(); ++part, ++ipart) {
            CLHEP::HepLorentzVector particle_sl((*part).GetPx(), (*part).GetPy(), (*part).GetPz(), (*part).GetE());
            photon_system += particle_sl;
            ptscale += std::sqrt((*part).GetPx()*(*part).GetPx() + (*part).GetPy()*(*part).GetPy());
        }

        // Avg pt is the correct scale here
        if (ipart == 0) [[unlikely]] {
            ATH_MSG_WARNING( "Starlight_i::starlight2lhef: ipart is zero.");
            ptscale = 0;
        } else {
            ptscale /= static_cast<float> (ipart);
        }
    
        lheStream << "  4  9999  1.000000e+00  "<<ptscale<<"  7.297e-03  2.569093e-01\n";

        if (m_doTauolappLheFormat) {
            lheStream << "    -11    -1     0     0     0     0  0.0000000000e+00  0.0000000000e+00  "
                        << photon_system.m()/2.*std::exp(photon_system.rapidity())<<"  "
                        <<photon_system.m()/2.*std::exp(photon_system.rapidity())
                        << "  0.0000000000e+00 0. 9.\n";
            lheStream << "     11    -1     0     0     0     0  0.0000000000e+00  0.0000000000e+00  "
                        << -photon_system.m()/2.*std::exp(-photon_system.rapidity())<<"  "
                        <<photon_system.m()/2.*std::exp(-photon_system.rapidity())
                        << "  0.0000000000e+00 0. 9.\n";
        }

        else {
            lheStream << "     22    -1     0     0     0     0  0.0000000000e+00  0.0000000000e+00  "
                << photon_system.m()/2.*std::exp(photon_system.rapidity())<<"  "
                <<photon_system.m()/2.*std::exp(photon_system.rapidity())
                <<"  0.0000000000e+00 0. 9.\n";
            lheStream << "     22    -1     0     0     0     0  0.0000000000e+00  0.0000000000e+00  "
                << -photon_system.m()/2.*std::exp(-photon_system.rapidity())<<"  "
                <<photon_system.m()/2.*std::exp(-photon_system.rapidity())
                <<"  0.0000000000e+00 0. 9.\n";
        }

        for (part = particleCollection->begin(); part != particleCollection->end(); ++part, ++ipart) {
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

            lheStream <<"    "<<(pid>0?" ":"")<<pid<<"     1     1     2     0     0  "<<px<<"  "<<py<<"  "<<pz<<"  "<<e<<"  "<<mass<<"  0. 9.\n";
        }

        lheStream << "</event>\n";
    }

    // Go back to the header and swap cross-section line
    lheStream.seekp(pos);
    lheStream <<"  "<<std::scientific<<std::setprecision(6)<<getAcceptedCrossSection()/Gaudi::Units::picobarn;
    lheStream <<"  0.000000e+00  1.000000e+00   9999\n";
    lheStream.seekp(0, std::ios::end);

    lheStream << "</LesHouchesEvents>";
    lheStream.close();

    return true;
}

bool Starlight_i::load_user_params() {
    // Set starlight user initialization parameters

    m_configFileName = "slight.in";
    if (!parse_init_vector() || !write_params_file()) {
        ATH_MSG_WARNING( "Problem in initializing input parameters. Cannot initialize Starlight." );
        return false;
    }

    if (!m_inputParameters.configureFromFile(m_configFileName)) {
        ATH_MSG_ERROR( "Problem in Starlight configuration from file." );
        return false;
    }

    if (!m_inputParameters.init()) {
        ATH_MSG_WARNING( "Problem in Starlight 'inputParameters' initialization." );
        return false;
    }

    return true;
}

bool Starlight_i::parse_init_vector() {
    // Parse parameters from `m_InitializeVector` to the interface variables

    for(CommandVector::iterator param = m_InitializeVector.begin(); param != m_InitializeVector.end(); ++param ) {
        ATH_MSG_INFO( " Command is: " << *param );

        StringParse paramStr(*param);
        std::string paramKey = paramStr.piece<std::string>(1);
        double paramVal = paramStr.piece<double>(2);
        if (paramKey == "beam1Z")
            m_beam1Z  = paramVal;
        else if (paramKey == "beam1A")
            m_beam1A  = paramVal;
        else if (paramKey == "beam2Z")
            m_beam2Z  = paramVal;
        else if (paramKey == "beam2A")
            m_beam2A  = paramVal;
        else if (paramKey == "beam1Gamma")
            m_beam1Gamma  = paramVal;
        else if (paramKey == "beam2Gamma")
            m_beam2Gamma  = paramVal;
        else if (paramKey == "maxW")
            m_maxW  = paramVal;
        else if (paramKey == "minW")
            m_minW  = paramVal;
        else if (paramKey == "nmbWBins")
            m_nmbWBins  = paramVal;
        else if (paramKey == "maxRapidity")
            m_maxRapidity  = paramVal;
        else if (paramKey == "nmbRapidityBins")
            m_nmbRapidityBins  = paramVal;
        else if (paramKey == "accCutPt")
            m_accCutPt = paramVal;
        else if (paramKey == "minPt")
            m_minPt = paramVal;
        else if (paramKey == "maxPt")
            m_maxPt = paramVal;
        else if (paramKey == "accCutEta")
            m_accCutEta = paramVal;
        else if (paramKey == "minEta")
            m_minEta = paramVal;
        else if (paramKey == "maxEta")
            m_maxEta = paramVal;
        else if (paramKey == "productionMode")
            m_productionMode  = paramVal;
        else if (paramKey == "axionMass")
            m_axionMass  = paramVal;
        else if (paramKey == "nmbEventsTot")
            m_nmbEventsTot  = paramVal;
        else if (paramKey == "prodParticleId")
            m_prodParticleId  = paramVal;
        else if (paramKey == "beamBreakupMode")
            m_beamBreakupMode  = paramVal;
        else if (paramKey == "interferenceEnabled")
            m_interferenceEnabled  = paramVal;
        else if (paramKey == "interferenceStrength")
            m_interferenceStrength  = paramVal;
        else if (paramKey == "coherentProduction")
            m_coherentProduction = paramVal;
        else if (paramKey == "incoherentFactor")
            m_incoherentFactor  = paramVal;
        else if (paramKey == "maxPtInterference")
            m_maxPtInterference  = paramVal;
        else if (paramKey == "nmbPtBinsInterference")
            m_nmbPtBinsInterference  = paramVal;
        else if (paramKey == "xsecMethod")
            m_xsecMethod = paramVal;
        else if (paramKey == "nThreads")
            m_nThreads = paramVal;
        else if (paramKey == "pythFullRec")
            m_pythFullRec = paramVal;
        else {
            ATH_MSG_ERROR( " ERROR in STARLIGHT INITIALIZATION PARAMETERS  "
                            << paramKey << " is an invalid parameter !"  );
            return false;
        }
    }

    return true;
}


bool Starlight_i::write_params_file() {
    // Write initialization parameters to file
    // to call `configureFromFile()` on it

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
    configFile << "RND_SEED = " << m_inputParameters.randomSeed() << std::endl;
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

double Starlight_i::getAcceptedCrossSection() {
    // Returns cross-section of the generated sample after acceptance cuts.
    // The units are converted to Gaudi::Units.
    return (static_cast<double>(m_starlight->nmbAccepted()) / m_starlight->nmbAttempts()) *
        m_inputParameters.inputBranchingRatio() * m_starlight->getTotalCrossSection() * Gaudi::Units::barn;
}
