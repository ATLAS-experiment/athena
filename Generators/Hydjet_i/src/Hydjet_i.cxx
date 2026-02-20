/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// -------------------------------------------------------------
// Generators/Hydjet.cxx Description: Allows the user
// to generate Hydjet events and store the result in the
// Transient Store.
//
// AuthorList:
//   Andrzej Olszewski:  Initial Code February 2007
//
// m_hyipar used only for stoing precalculated AW nad RA

#include "Hydjet_i/Hydjet_i.h"

#include "GaudiKernel/MsgStream.h"

#include "CxxUtils/StringParse.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/HepMCHelpers.h"

#include <stdlib.h>

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Vector/LorentzVector.h"


// calls to fortran routines
extern "C"
{
  void luedit_(int*);

  void hyinit_(double*, double*, int*, double*, double*, double*, int*);

  void hyevnt_();
}

Hydjet::Hydjet(const std::string& name,
               ISvcLocator* pSvcLocator): GenModule(name,pSvcLocator)
{
  declareProperty("Initialize", m_InitializeVector);
}

StatusCode Hydjet::genInitialize()
{

  ATH_CHECK(m_event_paramsKey.initialize());
  // Initialisation of input parameters
  //
  ATH_MSG_INFO( "===> March 13 2008 HYDJET INTERFACE VERSION.  \n" );
  ATH_MSG_INFO( "===> HYDJET INITIALISING.  \n" );

  // Set the users' initialisation parameters choices
  set_user_params();

  /*
    std::cout << " PRINTING HYFLOW " << std::endl;
    std::cout << "ytfl,ylfl,tf,fpart" << ", "
    << m_hyflow.ytfl() << ", " << m_hyflow.ylfl() << ", "
    << m_hyflow.tf() << ", " << m_hyflow.fpart()
    << std::endl;
    std::cout << " PRINTING HYJPAR " << std::endl;
    std::cout << "ptmin,sigin,sigjet,nhsel,ishad,njet" << ", "
    << m_hyjpar.ptmin() << ", " << m_hyjpar.sigin() << ", "
    << m_hyjpar.sigjet() << ", " << m_hyjpar.nhsel() << ", "
    << m_hyjpar.ishad() << ", " << m_hyjpar.njet()
    << std::endl;
    std::cout << " PRINTING PYQPAR " << std::endl;
    std::cout << "ienglu,ianglu,t0,tau0,nf" << ", "
    << m_pyqpar.ienglu() << ", " << m_pyqpar.ianglu() << ", "
    << m_pyqpar.t0() << ", " << m_pyqpar.tau0() << ", "
    << m_pyqpar.nf() << std::endl;
  */

  hyinit_(&m_e,&m_a,&m_ifb,&m_bmin,&m_bmax,&m_bfix,&m_nh);

  /*
    std::cout << " PRINTING MSTU " << std::endl;
    for (int i = 1; i <= m_pydat1.lenMstu(); ++i)
    std::cout << i << ":" << m_pydat1.mstu(i) << ", ";
    std::cout << std::endl;
    std::cout << " PRINTING PARU " << std::endl;
    for (int i = 1; i <= m_pydat1.lenParu(); ++i)
    std::cout << i << ":" << m_pydat1.paru(i) << ", ";
    std::cout << std::endl;
    std::cout << " PRINTING MSTJ " << std::endl;
    for (int i = 1; i <= m_pydat1.lenMstj(); ++i)
    std::cout << i << ":" << m_pydat1.mstj(i) << ", ";
    std::cout << std::endl;
    std::cout << " PRINTING PARJ " << std::endl;
    for (int i = 1; i <= m_pydat1.lenParj(); ++i)
    std::cout << i << ":" << m_pydat1.parj(i) << ", ";
    std::cout << std::endl;

    std::cout << " PRINTING MSTP " << std::endl;
    for (int i = 1; i <= m_pypars.lenMstp(); ++i)
    std::cout << i << ":" << m_pypars.mstp(i) << ", ";
    std::cout << std::endl;
    std::cout << " PRINTING PARP " << std::endl;
    for (int i = 1; i <= m_pypars.lenParp(); ++i)
    std::cout << i << ":" << m_pypars.parp(i) << ", ";
    std::cout << std::endl;
    std::cout << " PRINTING MSTI " << std::endl;
    for (int i = 1; i <= m_pypars.lenMsti(); ++i)
    std::cout << i << ":" << m_pypars.msti(i) << ", ";
    std::cout << std::endl;
    std::cout << " PRINTING PARI " << std::endl;
    for (int i = 1; i <= m_pypars.lenPari(); ++i)
    std::cout << i << ":" << m_pypars.pari(i) << ", ";
    std::cout << std::endl;

    std::cout << " PRINTING PYSUBS " << std::endl;
    std::cout << "msel,mselpd" << ", "
    << m_pysubs.msel() << ", " << m_pysubs.mselpd() << std::endl;
    std::cout << " PRINTING MSUB " << std::endl;
    for (int i = 1; i <= m_pysubs.lenMsub(); ++i)
    std::cout << i << ":" << m_pysubs.msub(i) << ", ";
    std::cout << std::endl;
    std::cout << " PRINTING KFIN " << std::endl;
    for (int i = 1; i <= m_pysubs.leniKfin(); ++i)
    for (int j = 1; j <= m_pysubs.lenjKfin(); ++j)
    std::cout << i << ":" << j << ":" << m_pysubs.kfin(i,j) << ", ";
    std::cout << std::endl;
    std::cout << " PRINTING CKIN " << std::endl;
    for (int i = 1; i <= m_pysubs.lenCkin(); ++i)
    std::cout << i << ":" << m_pysubs.ckin(i) << ", ";
    std::cout << std::endl;
  */

  return StatusCode::SUCCESS;
}

StatusCode Hydjet::callGenerator()
{
  ATH_MSG_INFO( " HYDJET generating.  \n" );

  // Generate event
  hyevnt_();

  // Remove unstable particles and partons
  int luedit_init = 2;
  luedit_(&luedit_init);

  // update event counter
  ++m_events;

  /*
    std::cout << " PRINTING HYIPAR " << std::endl;
    std::cout << "bminh,bmaxh,AW,RA,npar0,nbco0,Apb,Rpb,np,init,ipr" << ", "
    << m_hyipar.bminh() << ", " << m_hyipar.bmaxh() << ", "
    << m_hyipar.AW() << ", " << m_hyipar.RA() << ", "
    << m_hyipar.npar0() << ", " << m_hyipar.nbco0() << ", "
    << m_hyipar.Apb() << ", " << m_hyipar.Rpb() << ", "
    << m_hyipar.np() << ", " << m_hyipar.init() << ", "
    << m_hyipar.ipr() << std::endl;
    std::cout << " PRINTING HYFPAR " << std::endl;
    std::cout << "bgen,nbcol,npart,npyt,nhyd" << ", "
    << m_hyfpar.bgen() << ", " << m_hyfpar.nbcol() << ", "
    << m_hyfpar.npart() << ", " << m_hyfpar.npyt() << ", "
    << m_hyfpar.nhyd() << std::endl;
  */

  // store hijing event parameters
  // -----------------------------
  ATH_MSG_DEBUG( "New HijingEventParams" );
  int np = (int) round(m_hyfpar.npart()/2);
  int nt = (int) round(m_hyfpar.npart()/2);
  int n0 =  (int) round(m_hyfpar.nbcol());
  int n01 = -1;
  int n10 = -1;
  int n11 = -1;
  int natt = m_hyfpar.nhyd() + m_hyfpar.npyt();
  int jatt = m_hyjpar.njet();
  float b = m_hyfpar.bgen() * m_hyipar.RA();
  float bphi = 0;

  SG::WriteHandle<HijingEventParams> event_params(m_event_paramsKey);
  event_params = std::make_unique<HijingEventParams>(np, nt, n0, n01, n10, n11, natt, jatt, b, bphi);

  ATH_MSG_INFO( "\n=================================================\n"
                << "  HYDJET event description: \n"
                << "    b                    = " << b << " fm \n"
                << "    # total participants = " << m_hyfpar.npart() << "\n"
                << "    # collisions         = " << m_hyfpar.nbcol() << "\n"
                << "    # jets               = " << m_hyjpar.njet()  << "\n"
                << "    # pythia particles   = " << m_hyfpar.npyt()  << "\n"
                << "    # hydro particles    = " << m_hyfpar.nhyd()  << "\n"
                << "=================================================\n" );

  ATH_MSG_DEBUG( " HIJING generating done.  \n" );
  return StatusCode::SUCCESS;
}

StatusCode
Hydjet::genFinalize()
{
  ATH_MSG_INFO( " HIJING Ending.  \n" );
  return StatusCode::SUCCESS;
}

StatusCode
Hydjet::fillEvt(HepMC::GenEvent* evt)
{
  ATH_MSG_INFO( " HYDJET Filing.  \n" );

  // Set the event number
  evt->set_event_number( m_events );

  // Set the random generator seeds
  // evt->set_random_states(m_seeds);

  // Set the generator id
  HepMC::set_signal_process_id(evt,100000000 + int(m_a));

  // Create the event vertex
  HepMC::GenVertexPtr v1 = HepMC::newGenVertexPtr();
  evt->add_vertex( v1 );

  double eproj = m_e/2.0;
  int proj_id = (int) m_a;
  v1->add_particle_in( HepMC::newGenParticlePtr( HepMC::FourVector(0., 0., eproj, eproj), proj_id, 101 ) );

  double etarg = m_e/2.0;
  int targ_id = (int) m_a;
  v1->add_particle_in( HepMC::newGenParticlePtr( HepMC::FourVector(0., 0., -etarg, etarg), targ_id, 102 ) );

  // Loop on all final particles and
  // put them all as outgoing from the event vertex
  for (int i = 1; i <= m_lujets.n(); ++i)
    {
      v1->add_particle_out( HepMC::newGenParticlePtr(
                                                     HepMC::FourVector(m_lujets.p(i, 1), m_lujets.p(i, 2),
                                                                       m_lujets.p(i, 3), m_lujets.p(i, 4)), m_lujets.k(i, 2), 1 ) );
    }

  // Set the generator id
  HepMC::set_signal_process_id(evt,100000000 + int(m_a));

  MC::GeVToMeV(evt); //Only scales momenta and masses

  return StatusCode::SUCCESS;
}

void
Hydjet::set_user_params	(void)
{
  m_e         = 5520.0;
  m_a         = 208;

  // copy of HYDJET calculations
  m_hyipar.AW() = m_a;
  m_hyipar.RA() = 1.15 * std::pow(m_a,0.333333);

  m_ifb       = 1;
  m_bmin      = 0;
  m_bmax      = 1;
  m_bfix      = 0;
  m_nh        = 20000;

  // set hydro parameters
  // nhsel=0 - hydro (no jets), nhsel=1 - hydro + pythia jets,
  // nhsel=2 - hydro + pyquen jets, nhsel=3 - pythia jets (no hydro),
  // nhsel=4 - pyquen jets (no hydro)
  m_hyjpar.nhsel()=2;             // flag to include hard scatterings
  m_hyjpar.ishad()=1;             // flag to include "nuclear shadowing"
  m_hyflow.ylfl()=4.;             // maximum longitudinal flow rapidity
  m_hyflow.ytfl()=1.5;             // maximum transverse flow rapidity
  m_hyflow.tf()=0.1;              // freeze-out temperature in GeV
  m_hyflow.fpart()=1.;            // fraction of soft multiplicity proportional
  // # of nucleons-participants

  // set quenching parameters
  // ienglu=0 - radiative and collisional loss,
  // ienglu=1 - only radiative loss, ienglu=2 - only collisional loss;
  // ianglu=0 - small-angular radiation, ianglu=1 - wide angular radiation,
  // inanglu=2 - collinear radiation
  m_pyqpar.ienglu()=0;           // radiative and collisional loss
  m_pyqpar.ianglu()=0;           // radiative and collisional loss
  m_pyqpar.t0()=0;               // initial QGP temperature
  m_pyqpar.tau0()=0.1;             // proper time of QGP formation
  m_pyqpar.nf()=0;                 // number of active quark flavours in QGP

  // set input PYTHIA parameters
  m_pysubs.msel()=1;              // QCD-dijet production
  m_hyjpar.ptmin()=10.;
  // hydjet recommendation
  m_pysubs.ckin(3)=m_hyjpar.ptmin();// minimum pt in initial hard sub-process
  m_pypars.mstp(51)=7;          // CTEQ5M pdf
  m_pypars.mstp(81)=0;          // pp multiple scattering off
  m_pydat1.mstu(21)=1;          // avoid stopping run
  m_pydat1.paru(14)=1.;         // tolerance parameter to adjust fragmentation

  // Set user Initialization parameters
  for(CommandVector::iterator i = m_InitializeVector.begin(); i != m_InitializeVector.end(); ++i )
    {
      ATH_MSG_INFO( " Command is: " << *i );
      StringParse mystring(*i);
      std::string myparam = mystring.piece<std::string>(1);
      if (myparam == "e")
        {
          m_e         = mystring.piece<double>(2);
        }
      else if (myparam == "a")
        {
          m_a         = mystring.piece<int>(2);
        }
      else if (myparam == "ifb")
        {
          m_ifb       = mystring.piece<int>(2);
        }
      else if (myparam == "bmin")
        {
          m_bmin = mystring.piece<double>(2);
          m_bmin      /= m_hyipar.RA();
        }
      else if (myparam == "bmax")
        {
          m_bmax = mystring.piece<double>(2);
          m_bmax      /= m_hyipar.RA();
        }
      else if (myparam == "bfix")
        {
          m_bfix = mystring.piece<double>(2);
          m_bfix      /= m_hyipar.RA();
        }
      else if (myparam == "nh")
        {
          m_nh = mystring.piece<int>(2);
        }
      else if (myparam == "nseed")
        {
          m_ludatr.mrlu(1)    = mystring.piece<int>(2);
        }
      else if (myparam == "ytfl")
        {
          m_hyflow.ytfl()     = mystring.piece<double>(2);
        }
      else if (myparam == "ylfl")
        {
          m_hyflow.ylfl()     = mystring.piece<double>(2);
        }
      else if (myparam == "tf")
        {
          m_hyflow.tf()       = mystring.piece<double>(2);
        }
      else if (myparam == "fpart")
        {
          m_hyflow.fpart()    = mystring.piece<double>(2);
        }
      else if (myparam == "nhsel")
        {
          m_hyjpar.nhsel()    = mystring.piece<int>(2);
        }
      else if (myparam == "ishad")
        {
          m_hyjpar.ishad()    = mystring.piece<int>(2);
        }
      else if (myparam == "ptmin")
        {
          m_hyjpar.ptmin()    = mystring.piece<double>(2);
          // hydjet recommendation
          m_pysubs.ckin(3)    = mystring.piece<double>(2);
        }
      else if (myparam == "ienglu")
        {
          m_pyqpar.ienglu()  = mystring.piece<int>(2);
        }
      else if (myparam == "ianglu")
        {
          m_pyqpar.ianglu()  = mystring.piece<int>(2);
        }
      else if (myparam == "t0")
        {
          m_pyqpar.t0()  = mystring.piece<double>(2);
        }
      else if (myparam == "tau0")
        {
          m_pyqpar.tau0()  = mystring.piece<double>(2);
        }
      else if (myparam == "nf")
        {
          m_pyqpar.nf()  = mystring.piece<int>(2);
        }
      else if (myparam == "mstu")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pydat1.mstu(myelem) = mystring.piece<int>(3);
        }
      else if (myparam == "paru")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pydat1.paru(myelem) = mystring.piece<double>(3);
        }
      else if (myparam == "mstj")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pydat1.mstj(myelem) = mystring.piece<int>(3);
        }
      else if (myparam == "parj")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pydat1.parj(myelem) = mystring.piece<double>(3);
        }
      else if (myparam == "msel")
        {
          m_pysubs.msel()       = mystring.piece<int>(2);
        }
      else if (myparam == "mselpd")
        {
          m_pysubs.mselpd()   = mystring.piece<int>(2);
        }
      else if (myparam == "msub")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pysubs.msub(myelem) = mystring.piece<int>(3);
        }
      else if (myparam == "kfin")
        {
          int		myelem1  = mystring.piece<int>(2);
          int		myelem2  = mystring.piece<int>(3);
          m_pysubs.kfin(myelem1,myelem2) = mystring.piece<int>(4);
        }
      else if (myparam == "ckin")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pysubs.ckin(myelem) = mystring.piece<double>(3);
        }
      else if (myparam == "mstp")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pypars.mstp(myelem) = mystring.piece<int>(3);
        }
      else if (myparam == "parp")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pypars.parp(myelem) = mystring.piece<double>(3);
        }
      else if (myparam == "msti")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pypars.msti(myelem) = mystring.piece<int>(3);
        }
      else if (myparam == "pari")
        {
          int		myelem  = mystring.piece<int>(2);
          m_pypars.pari(myelem) = mystring.piece<double>(3);
        }
      else
        {
          ATH_MSG_ERROR( " ERROR in HIJING INITIALIZATION PARAMETERS   " << myparam
                         << " is an invalid parameter !" );
        }
    }
}
