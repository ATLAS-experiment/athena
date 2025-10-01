/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ----------------------------------------------------------------------
// Generators/Epos.cxx
//
// Description: Allows the user to generate epos events and store the result
// in the Transient Store.
//
// AuthorList:
//   Sami Kama:       Initial code.
//   Sebastian Piec:  Adaptation for Epos 1.99.crmc.r2790.
//   Andrii Verbytskyi: 2.0.1+
// ----------------------------------------------------------------------

#include "GaudiKernel/MsgStream.h"
#include "CLHEP/Random/RandFlat.h"
#include "AthenaKernel/RNGWrapper.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/HeavyIon.h"
#include "AtlasHepMC/SimpleVector.h"

#include "Epos_i/Epos.h"
#include "CRMChepevt.h"
#define CRMC_STATIC
#include "CRMCinterface.h"


namespace {
static std::string epos_rndm_stream = "EPOS_INIT";
static CLHEP::HepRandomEngine* p_rndmEngine{};
}

extern "C" double atl_epos_rndm_( int* )
{
    return CLHEP::RandFlat::shoot(p_rndmEngine);
}
// ----------------------------------------------------------------------
// Epos Fortran bindings.
// ----------------------------------------------------------------------
extern "C"
{
    // generator initialization
    void crmc_init_f_( double &m_degymx, int &iSeed, int &model, int &itab, int &itypout, const char *paramFile, const char *output, int &lout);

    void crmc_set_f_( int &nEvents,double &beamMomentum, double &targetMomentum, int &primaryParticle, int &targetParticle);

    // event generation
    void crmc_f_( int &iout, int &ievent, int &nParticles, double &impactParam, int &partPdg,
                  double &partPx, double &partPy, double &partPz, double &partEnergy, double &partMass, int &outstat );

    // cross section info
    void crmc_xsection_f_(double &xsigtot, double &xsigine, double &xsigela, double &xsigdd,
                          double &xsigsd, double &xsloela, double &xsigtotaa, double &xsigineaa, double &xsigelaaa);

}
// ----------------------------------------------------------------------
Epos::Epos( const std::string &name, ISvcLocator *pSvcLocator ):
    GenModule( name, pSvcLocator )
{
    m_interface = nullptr;
    epos_rndm_stream = "EPOS_INIT";

    declareProperty( "BeamMomentum",    m_beamMomentum    = -6500.0 );      // GeV
    declareProperty( "TargetMomentum",  m_targetMomentum  = 6500.0 );
    declareProperty( "Model",           m_model           = 7 );            // 0=EPOS 1.99 LHC, 1=EPOS 1.99
    declareProperty( "PrimaryParticle", m_primaryParticle = 1 );            // 1=p, 12=C, 120=pi+, 207=Pb
    declareProperty( "TargetParticle",  m_targetParticle  = 1 );
    declareProperty( "ParamFile",       m_paramFile       = "crmc.param" );
    declareProperty( "LheOutput",       m_ilheout       = 0 );
    declareProperty( "LheFile",         m_lheout       = "epos.lhe" );
    declareProperty( "TabCreate",       m_itab       = 0 );
    declareProperty( "nEvents",         m_nEvents    = 5500 );
    declareProperty( "maxCMEnergy",     m_degymx     = 13000.0 ); // maximum center-of-mass energy which will be call in the run [GeV]

    m_events = 0; // current event number (counted by interface)
    m_ievent = 0;  // current event number counted by Epos
    m_iout = 0; // output type (output)

    // initialize internally used arrays
    ATH_MSG_INFO( "max number of Particles  " << kMaxParticles );
    m_partID.resize (kMaxParticles);
    m_partPx.resize (kMaxParticles);
    m_partPy.resize (kMaxParticles);
    m_partPz.resize (kMaxParticles);
    m_partEnergy.resize (kMaxParticles);
    m_partMass.resize (kMaxParticles);
    m_partStat.resize (kMaxParticles);
    for (size_t i = 0; i < kMaxParticles; ++i) {
        m_partID[i] = 0;
        m_partPx[i] = m_partPy[i] = m_partPz[i] = m_partEnergy[i] = m_partMass[i] = 0.0;
        m_partStat[i] = 0;
    }

}

// ----------------------------------------------------------------------
StatusCode Epos::genInitialize()
{
    ATH_MSG_INFO( " CRMC INITIALISING.\n" );
    ATH_MSG_INFO( "signal_rocess_id 101-ND, 105-DD, 102-CD, 103 AB->XB, 104 AB->AX \n");

    p_rndmEngine = getRandomEngineDuringInitialize(epos_rndm_stream, m_randomSeed, m_dsid); // NOT THREAD-SAFE
    const long *sip = p_rndmEngine->getSeeds();
    long int si1 = sip[0];

    int iSeed = si1%1000000000;     // FIXME ?
    int lout = 50;     //lenght of the output string (useful only for LHE output)
    // initialise Epos and set up initial values

    crmc_init_f_( m_degymx, iSeed, m_model, m_itab, m_ilheout, (m_paramFile + " ").c_str(), m_lheout.c_str(), lout);
    crmc_set_f_(m_nEvents,m_beamMomentum, m_targetMomentum, m_primaryParticle, m_targetParticle);

    epos_rndm_stream = "EPOS";

    m_events = 0;

    return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
StatusCode Epos::callGenerator()
{
    // ATH_MSG_INFO( " EPOS Generating." );

    //Re-seed the random number stream
    long seeds[7];
    const EventContext& ctx = Gaudi::Hive::currentContext();
    ATHRNG::calculateSeedsMC21(seeds, epos_rndm_stream, ctx.eventID().event_number(), m_dsid, m_randomSeed);
    p_rndmEngine->setSeeds(seeds, 0); // NOT THREAD-SAFE

    // save the random number seeds in the event
    const long *s = p_rndmEngine->getSeeds();
    m_seeds.clear();
    m_seeds.push_back(s[0]);
    m_seeds.push_back(s[1]);

    ++m_events;

    // as in crmchep.h
    int nParticles = 0;
    double impactParameter = -1.0;

    // generate event
    crmc_f_( m_iout, m_ievent,nParticles, impactParameter, m_partID[0], m_partPx[0], m_partPy[0], m_partPz[0],
             m_partEnergy[0], m_partMass[0], m_partStat[0]  );

    return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
StatusCode Epos::genFinalize()
{
    ATH_MSG_INFO("EPOS finalizing.");

    std::cout << "MetaData: generator = Epos " << std::endl;

    // retrieve information about the total cross-section from Epos
    double xsigtot, xsigine, xsigela, xsigdd, xsigsd, xsloela, xsigtotaa, xsigineaa, xsigelaaa;
    xsigtot = xsigine = xsigela = xsigdd = xsigsd = xsloela = xsigtotaa = xsigineaa = xsigelaaa = 0.0;

    crmc_xsection_f_(xsigtot, xsigine, xsigela, xsigdd, xsigsd, xsloela, xsigtotaa, xsigineaa, xsigelaaa);

    xsigtot *= 1000000;         // [mb] to [nb] conversion
    std::cout << "MetaData: cross-section (nb) = " << xsigtot << std::endl;
    xsigine *= 1000000;        //[mb] to [nb] conversion
    std::cout << "MetaData: cross-section inelastic (cut + projectile diffraction)[nb] = " << xsigine << std::endl;
    xsigela *= 1000000;         // [mb] to [nb] conversion
    std::cout << "MetaData: cross-section elastic (includes target diffraction)[nb] = " << xsigela << std::endl;
    xsigdd *= 1000000;         // [mb] to [nb] conversion
    std::cout << "MetaData: cross-section dd (nb) = " << xsigdd << std::endl;
    xsigsd *= 1000000;         // [mb] to [nb] conversion
    std::cout << "MetaData: cross-section sd (nb) = " << xsigsd << std::endl;

    //  m_eposEventInfo.close();

    return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
StatusCode Epos::fillEvt( HepMC::GenEvent* evt )
{
    CRMChepevt<HepMC::GenParticlePtr, HepMC::GenVertexPtr, HepMC::FourVector, HepMC::GenEvent> hepevtconverter;
#ifdef HEPMC3
    hepevtconverter.convert(*evt);
    evt->set_event_number(m_events);
    HepMC::fillBarcodesAttribute(evt);
    HepMC::set_random_states(evt, m_seeds );
    evt->weights().push_back(1.0);
    m_runinfo = std::make_shared<HepMC3::GenRunInfo>();
    std::vector<std::string> names = {"Default"};
    m_runinfo->set_weight_names(names);
    evt->set_run_info(m_runinfo);
    evt->set_units(HepMC3::Units::MEV, HepMC3::Units::MM);
#else
    /// We use the old approach for HepMC2, as the CRMC 2.0.1 has a bug that prevents us from using the same approach as for HepMC3.
    /// This should be changed once the bug is fixed.
    hepevtconverter.convert();
    for (auto& v: hepevtconverter.vertices()) evt->add_vertex(v);
    if  (hepevtconverter.beams().size() == 2) evt->set_beam_particles(hepevtconverter.beams()[0],hepevtconverter.beams()[1]);
    if  (hepevtconverter.beams().size() == 1) evt->set_beam_particles(hepevtconverter.beams()[0],nullptr);
    evt->set_event_number(m_events);
    HepMC::set_random_states(evt, m_seeds );
    evt->weights().push_back(1.0);
    GeVToMeV(evt);
#endif

    std::vector<HepMC::GenParticlePtr> beams;

    for (auto p: *evt) {
        if (p->status() == 4) {
            beams.push_back(std::move(p));
        }
    }

    if (beams.size() >= 2) {
        evt->set_beam_particles(beams[0], beams[1]);
    } else {
        ATH_MSG_INFO( "EPOS event has only " << beams.size() << " beam particles" );
    }

    // Heavy Ion and Signal ID from Epos to HepMC

#ifdef HEPMC3
    HepMC::GenHeavyIonPtr ion= std::make_shared<HepMC::GenHeavyIon>();
    ion->Ncoll_hard=cevt_.kohevt;
    ion->Npart_proj=cevt_.npjevt;
    ion->Npart_targ=cevt_.ntgevt;
    ion->Ncoll=cevt_.kolevt;
    ion->spectator_neutrons=cevt_.npnevt + cevt_.ntnevt;
    ion->spectator_protons=cevt_.nppevt + cevt_.ntpevt;
    ion->N_Nwounded_collisions=-1;
    ion->Nwounded_N_collisions=-1;
    ion->Nwounded_Nwounded_collisions=-1;
    ion->impact_parameter= cevt_.bimevt;
    ion->event_plane_angle=cevt_.phievt;
    ion->eccentricity=-1;  //c2evt_.fglevt,  //correct name but not defined
    ion->sigma_inel_NN=1e9*hadr5_.sigine;
#else
    HepMC::HeavyIon ion(cevt_.kohevt,
                        cevt_.npjevt,
                        cevt_.ntgevt,
                        cevt_.kolevt,
                        cevt_.npnevt + cevt_.ntnevt,
                        cevt_.nppevt + cevt_.ntpevt,
                        -1,
                        -1,
                        -1,
                        cevt_.bimevt,
                        cevt_.phievt,
                        -1,  //c2evt_.fglevt,  //correct name but not defined
                        1e9*hadr5_.sigine);
#endif

    evt->set_heavy_ion(std::move(ion));

    //an integer ID uniquely specifying the signal process (i.e. MSUB in Pythia)
    // Epos 1-ND, 2-DD, 3-CD, 4 AB->XB, -4 AB->AX translated into signal_proces_id 101-ND, 105 - DD, 102 - CD, 103 -AB->XB, 104 AB->AX
    int sig_id = -1;
    switch (int(c2evt_.typevt))
    {
    case  1: sig_id = 101; break;
    case -1: sig_id = 101; break;
    case  2: sig_id = 105; break;
    case -2: sig_id = 105; break;
    case  3: sig_id = 102; break;
    case -3: sig_id = 102; break;
    case  4: sig_id = 103; break;
    case -4: sig_id = 104; break;
    default: ATH_MSG_INFO( "Signal ID not recognised for setting HEPEVT \n");
    }

    HepMC::set_signal_process_id(evt,sig_id);

    double xsigtot, xsigine, xsigela, xsigdd, xsigsd, xsloela, xsigtotaa, xsigineaa, xsigelaaa;
    xsigtot = xsigine = xsigela = xsigdd = xsigsd = xsloela = xsigtotaa = xsigineaa = xsigelaaa = 0.0;
    crmc_xsection_f_(xsigtot, xsigine, xsigela, xsigdd, xsigsd, xsloela, xsigtotaa, xsigineaa, xsigelaaa);
    xsigtot *= 1000000;         // [mb] to [nb] conversion
#ifdef HEPMC3
    std::shared_ptr<HepMC3::GenCrossSection> xsec = std::make_shared<HepMC3::GenCrossSection>();
    xsec->set_cross_section(xsigine, 0.0);
#else
    HepMC::GenCrossSection xsec;
    xsec.set_cross_section(xsigine, 0.0);
#endif
    evt->set_cross_section(std::move(xsec));

    return StatusCode::SUCCESS;
}

