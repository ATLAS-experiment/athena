/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// -------------------------------------------------------------
// Generators/Hijing.cxx Description: Allows the user
// to generate hijing events and store the result in the
// Transient Store.
//
// AuthorList:
//     Georgios Stavropoulos:  Initial Code October 2002
//     Brian Cole, Mikhail Leltchouk, Andrzej Olszewski: 2006, 2007
// Modified:
//     2007-Feb Alden Stradling:  Added vertex shifting and made options settable in jO.
//     2007-Sep Aaron Angerami: keepSpectators option.
//     2007-Nov Brian Cole, Mikhail Leltchouk: each vertex is added to the event before particles are added to the vertex
//          - this fixed bug #30991 for release 13.1.0 and for HepMC 2.3.0 where the 'set's comparison operates
//            on the bcode rather than on the pointer.
//     2008-Jul Borut Kersevan:  randomizing the left<->right direction by mirroring momenta settable in jobOpts for beamgas
//     2023-Aug Aaron Angerami: adding keepAllDecayVertices option to keep decay vertices for strong decays (default=true) 

#include "Hijing_i/Hijing.h"
#include "Hijing_i/VertexShift.h"

#include "GaudiKernel/MsgStream.h"

#include "CxxUtils/StringParse.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/HeavyIon.h"

#include <stdlib.h>

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Geometry/Point3D.h"

#include "AthenaKernel/RNGWrapper.h"

namespace {
  static CLHEP::HepRandomEngine *p_Engine;
  static std::string hijing_stream = "HIJING_INIT";
}

// calls to fortran routines
extern "C"
{
  float atl_ran_( int* )
  {
    return (float) CLHEP::RandFlat::shoot(p_Engine);
  }

  void hijset_(float*,
              const char*,
              const char*,
              const char*,
              int*,
              int*,
              int*,
              int*,
              long int,
              long int,
              long int);
  void hijing_(const char*, float*, float*, long int);
}

Hijing::Hijing(const std::string& name,
      ISvcLocator* pSvcLocator): GenModule(name,pSvcLocator)
{
  declareProperty("Initialize",       m_InitializeVector);
  declareProperty("partonStoreMinPt", m_partonStoreMinPt);
  declareProperty("vertexOffsetCut", m_vertexOffsetCut);
  declareProperty("randomizeVertices", m_rand); // Randomize for beam gas
  declareProperty("selectVertex", m_sel); // Select vertex location (off for random)
  declareProperty("wide", m_wide); // Allow randoms off the beamline (out to the pipe)
  declareProperty("x", m_x);
  declareProperty("y", m_y);
  declareProperty("z", m_z);
  declareProperty("keepSpectators", m_spec);
  declareProperty("randomizeP", m_prand); //BPK randomizes the left<->right direction by mirroring momenta for beam gas
  declareProperty("keepAllDecayVertices", m_keepAllDecayVertices);
}

StatusCode Hijing::genInitialize()
{
    ATH_CHECK(m_event_paramsKey.initialize());

    // Initialisation of input parameters
    std::cout << "MetaData: generator = Hijing "
              << HIJINGVERSION << std::endl;
    ATH_MSG_INFO( "===> Hijing initialising  \n" );
    if( m_rand ){
      ATH_MSG_INFO( "===> Vertex randomization ON." );
    }
    if( m_wide ){
      ATH_MSG_INFO( "===> Vertex rand covers whole beampipe." );
    }
    //BPK
    if( m_prand ) ATH_MSG_INFO( "===> Random Momentum Mirroring enabled" );
    if( m_keepAllDecayVertices ) ATH_MSG_INFO( "===> Keeping all decay vertices" );
    else ATH_MSG_INFO( "===> NOT keeping all decay vertices" );

#ifdef HEPMC3
    ATH_MSG_INFO( "===> HEPMC3 is used" );
#else
    ATH_MSG_INFO( "===> HEPMC3 is not used" );
#endif

    //CLHEP::HepRandomEngine* engine
    p_Engine = getRandomEngineDuringInitialize(hijing_stream, m_randomSeed, m_dsid); // NOT THREAD-SAFE
    hijing_stream       =       "HIJING";

    // Set the users' initialisation parameters choices
    set_user_params();

    const char* frame       = m_frame.c_str();
    const char* proj        = m_proj.c_str();
    const char* targ        = m_targ.c_str();

    hijset_( &m_efrm, frame, proj, targ, &m_iap, &m_izp, &m_iat, &m_izt,
            strlen(frame), strlen(proj), strlen(targ) );

    ATH_MSG_INFO( "\n=================================================\n"
           << "  HIJING initialization results: \n"
           << "    Total sigma     = " << m_hiparnt.hint1(13) << " mb\n"
           << "    Inelastic sigma = " << m_hiparnt.hint1(12) << " mb\n"
           << "    Jet sigma       = " << m_hiparnt.hint1(11) << " mb\n"
      << "=================================================\n" );


    return StatusCode::SUCCESS;
}

StatusCode Hijing::callGenerator()
{
    ATH_MSG_INFO( " HIJING generating.  \n" );

    // Save the random number seeds in the event
    //Re-seed the random number stream
    long seeds[7];
    const EventContext& ctx = Gaudi::Hive::currentContext();
    ATHRNG::calculateSeedsMC21(seeds, hijing_stream, ctx.eventID().event_number(), m_dsid, m_randomSeed);
    p_Engine->setSeeds(seeds, 0); // NOT THREAD-SAFE
    m_seeds.clear();
    m_seeds.push_back(seeds[0]);
    m_seeds.push_back(seeds[1]);

    // Generate event
    const char* frame       = m_frame.c_str();
    hijing_(frame, &m_bmin, &m_bmax, strlen(frame));

    // Check for error using the new (custom) error return
    //
    int errorStatus =  m_himain1.ierrstat();
    if (errorStatus != 0) {
      ATH_MSG_ERROR( "Error returned from HIJING, value = " << errorStatus );
      return  StatusCode::FAILURE;
    }

    // update event counter
    ++m_events;

    // store hijing event parameters
    // -----------------------------
    ATH_MSG_DEBUG( "New HijingEventParams" );
    int np = m_himain1.np();
    int nt = m_himain1.nt();
    int n0 = m_himain1.n0();
    int n01 = m_himain1.n01();
    int n10 = m_himain1.n10();
    int n11 = m_himain1.n11();
    int natt = m_himain1.natt();
    int jatt = m_himain1.jatt();
    float b = m_hiparnt.hint1(19);
    float bphi = m_hiparnt.hint1(20);

    SG::WriteHandle<HijingEventParams> event_params(m_event_paramsKey);
    event_params = std::make_unique<HijingEventParams>(np, nt, n0, n01, n10, n11, natt, jatt, b, bphi);

    ATH_MSG_INFO( "\n=================================================\n"
           << "  HIJING event description: \n"
           << "    b                   = " << b << " fm \n"
           << "    # proj participants = " << np << "\n"
           << "    # targ participants = " << nt << "\n"
      << "    # final particles   = " << natt << "\n"
      << "=================================================\n" );

    ATH_MSG_DEBUG( " HIJING generating done.  \n" );
    return StatusCode::SUCCESS;
}

StatusCode
Hijing::genFinalize()
{
    ATH_MSG_INFO( " HIJING Ending.  \n" );
    return StatusCode::SUCCESS;
}

StatusCode
Hijing::fillEvt(HepMC::GenEvent* evt)
{
  //  These two parameters need to be made user settable (done - AS)
  //
    MsgStream log(msgSvc(), name());
    ATH_MSG_INFO( " HIJING Filing.  \n" );

    // Set the event number
    evt->set_event_number( m_events );

    // Set the random generator seeds
    HepMC::set_random_states(evt,m_seeds);

    // Set the generator id
    HepMC::set_signal_process_id(evt,50000000 + m_iap);

    // Store collision parameters
    int np = m_himain1.np();
    int nt = m_himain1.nt();
    int n0 = m_himain1.n0();
    int n01 = m_himain1.n01();
    int n10 = m_himain1.n10();
    int n11 = m_himain1.n11();
    //int natt = m_himain1.natt();
    int jatt = m_himain1.jatt();
    float b = m_hiparnt.hint1(19);
    float bphi = m_hiparnt.hint1(20);

    float sigmainel =  m_hiparnt.hint1(12);

 #ifdef HEPMC3
     HepMC::GenHeavyIonPtr ion= std::make_shared<HepMC::GenHeavyIon>();
                      ion->Ncoll_hard=static_cast<int>(jatt);
                      ion->Npart_proj=static_cast<int>(np);
                      ion->Npart_targ=static_cast<int>(nt);
                      ion->Ncoll=static_cast<int>(n0+n10+n01+n11);
                      ion->N_Nwounded_collisions=static_cast<int>(n01);
                      ion->Nwounded_N_collisions=static_cast<int>(n10);
                      ion->Nwounded_Nwounded_collisions=static_cast<int>(n11);
                      ion->spectator_neutrons=-1;
                      ion->spectator_protons=-1;
                      ion->impact_parameter= b;
                      ion->event_plane_angle=bphi;
                      ion->event_plane_angle=-1;
                      ion->sigma_inel_NN=sigmainel;
    evt->set_heavy_ion(std::move(ion));                      
#else
    HepMC::HeavyIon ion
      (
       static_cast<int>(jatt), // Ncoll_hard
       static_cast<int>(np),   // Npart_proj
       static_cast<int>(nt),   // Npart_targ
       static_cast<int>(n0+n10+n01+n11), // Ncoll
       static_cast<int>(-1),   // spectator_neutrons
       static_cast<int>(-1),   // spectator_protons
       static_cast<int>(n01),  // N_Nwounded_collisions
       static_cast<int>(n10),  // Nwounded_N_collisions
       static_cast<int>(n11),  // Nwounded_Nwounded_collisions
       b,                      // impact_parameter
       bphi,                   // event_plane_angle
       -1,                     // eccentricity
       sigmainel     );        // sigma_inel_NN

    evt->set_heavy_ion(std::move(ion));
    std::cout << " heavy ion " << evt->heavy_ion() << std::endl;
#endif

    //  Did we keep decay history?
    //
    bool keptHistory = (m_hiparnt.ihpr2(21) == 1);

    //  The number of particles in the hijing output
    //
    int numHijingPart = m_himain1.natt();

    // Vectors that will keep track of where particles originate from and die
    //
    std::vector<int> partOriginVertex_vec(numHijingPart, 0);
    std::vector<int> partDecayVertex_vec(numHijingPart, -1);
    std::vector<HepMC::GenParticlePtr> particleHepPartPtr_vec(numHijingPart, nullptr);

    // Vector that will keep pointers to generated vertices
    //
    std::vector<HepMC::GenVertexPtr> vertexPtrVec;

    // Create the event vertex
    //
    CLHEP::HepLorentzVector newVertex;
    newVertex = CLHEP::HepLorentzVector(0.,0.,0.,0.);

    if( m_rand )newVertex = randomizeVertex(p_Engine); // Create a random vertex along the pipe
    else if(m_sel) newVertex = CLHEP::HepLorentzVector(m_x, m_y, m_z, 0.); // Create vertex at selected point - preempted by m_rand

    HepMC::GenVertexPtr v1 = HepMC::newGenVertexPtr(HepMC::FourVector(newVertex.x(),newVertex.y(),newVertex.z(),newVertex.t()));

    HepMC::set_signal_process_vertex(evt,v1);
    vertexPtrVec.push_back(v1);

    double eproj = (double) m_efrm;
    if ( m_frame == "CMS     " ) eproj = eproj / 2.;

    int proj_id = 2212;
    if (  m_proj == "PBAR    " ) {
       proj_id = -2212;
    } else if (  m_proj == "N       " ) {
       proj_id = 2112;
    } else if (  m_proj == "NBAR    " ) {
       proj_id = -2112;
    } else if (  m_proj == "PI+     " ) {
       proj_id = 211;
    } else if (  m_proj == "PI-     " ) {
       proj_id = -211;
    } else if (  m_proj == "A       " ) {
       proj_id = 3000000 + m_iap;
    }
    HepMC::GenParticlePtr part_p = HepMC::newGenParticlePtr(HepMC::FourVector(0., 0., eproj, eproj), proj_id, 101 );
    v1->add_particle_in( part_p );

    double etarg = 0.;
    if ( m_frame == "CMS     " ) etarg = ( (double) m_efrm ) / 2.;

    int       targ_id = 2212;
    if ( m_targ == "PBAR    " ) {
       targ_id = -2212;
    } else if (  m_targ == "N       " ) {
       targ_id = 2112;
    } else if (  m_targ == "NBAR    " ) {
       targ_id = -2112;
    } else if (  m_targ == "PI+     " ) {
       targ_id = 211;
    } else if (  m_targ == "PI-     " ) {
       targ_id = -211;
    } else if (  m_targ == "A       " ) {
       targ_id = 3000000 + m_iat;
    }
    HepMC::GenParticlePtr part_t = HepMC::newGenParticlePtr(HepMC::FourVector(0., 0., -etarg, etarg), targ_id, 102 );
    v1->add_particle_in( part_t );

    evt->set_beam_particles(part_p,part_t);

    ATH_MSG_DEBUG( "Hijing particles for event # " << m_events << ":\n"
           << " px,      "
           << " py,      "
           << " pz,      "
           << " Id,      "
           << " Source,  "
           << " Parent,  "
           << " Status,  " );

    bool inconsistency = false;

    // Loop on all Hijing particles and put them all as outgoing from the event vertex
    //
    for (int i = 1; i <= m_himain1.natt(); ++i)
    {
      //  Skip non-interacting projectile and target nucleons
      //
      if (!m_spec && ((m_himain2.katt(i, 2) == 0) ||  (m_himain2.katt(i, 2)) == 10)) continue;

      //  Find the vertex of the parent particle
      //
      int parentIndex = m_himain2.katt(i, 3) - 1;
      int parentOriginIndex = 0;
      int parentDecayIndex = -1;

      //  If the particle has a true parent, record its vertex info
      //
      if (parentIndex >= 0)
      {
       parentOriginIndex = partOriginVertex_vec[parentIndex];
       parentDecayIndex = partDecayVertex_vec[parentIndex];
      }

      //  A CLHEP vector containing the particle start point
      //
      // If the particle has been shifted, offset the particleStart accordingly
      //
      if( m_rand || m_sel ){ // Either way, we need to shift the particle vertex
       m_himain2.vatt(i,1) += newVertex(0);
       m_himain2.vatt(i,2) += newVertex(1);
       m_himain2.vatt(i,3) += newVertex(2);
       m_himain2.vatt(i,4) += newVertex(3);
      }

      ATH_MSG_DEBUG( std::fixed << std::setprecision(2)
         << std::setw(5) << i << ","
         << std::setw(7) << m_himain2.patt(i, 1) << ", "
         << std::setw(7) << m_himain2.patt(i, 2) << ", "
         << std::setw(7) << m_himain2.patt(i, 3) << ", "
         << std::setw(7) << m_himain2.katt(i, 1) << ", "
         << std::setw(7) << m_himain2.katt(i, 2) << ", "
         << std::setw(7) << m_himain2.katt(i, 3) << ", "
         << std::setw(7) << m_himain2.katt(i, 4) << ", "
         << std::setw(7) << m_himain2.vatt(i, 1) << ", "
         << std::setw(7) << m_himain2.vatt(i, 2) << ", "
         << std::setw(7) << m_himain2.vatt(i, 3) );

      CLHEP::HepLorentzVector particleStart(m_himain2.vatt(i, 1), m_himain2.vatt(i, 2),
                                       m_himain2.vatt(i, 3), m_himain2.vatt(i, 4));

      //  By default, the particle originates from the primary vertex
      //
      int particleVertexIndex = 0;

      //  Have we kept the history?
      //
      if (keptHistory)
       {
         //  Check to see if we've already generated a decay vertex for this parent
         //
         //     std::cout << "ML-> keptHistory == True" << std::endl;
         if (parentDecayIndex != -1)
           {
             //  Make sure it is consistent with this track origin
             //
             HepGeom::Point3D<double> vertex_pos(vertexPtrVec[parentDecayIndex]->position().x(),
                                  vertexPtrVec[parentDecayIndex]->position().y(),
                                  vertexPtrVec[parentDecayIndex]->position().z());
              double distance = vertex_pos.distance(particleStart.vect());

              //std::cout << "ML-> distance = " << distance <<"   parentDecayIndex = "<<parentDecayIndex<< std::endl;
             if (distance > m_vertexOffsetCut)
              {
                //  We have an inconsistency, print a message
                //
                ATH_MSG_WARNING( " Inconsistency in Hijing particle vertexing, particle # " << i
                    << " starting point (x,y,z) = ("
                    << particleStart.x() << ", "
                    << particleStart.y() << ", "
                    << particleStart.z() << ") "
                    << " a distance " << distance << " away  from parent decay point " );


                // Dump the parent decay vertex
                //
                log << MSG::WARNING << " Parent decay vertex: (x,y,z) = " << vertexPtrVec[parentDecayIndex]->position().x()
                    << ", " << vertexPtrVec[parentDecayIndex]->position().y()
                    << ", " << vertexPtrVec[parentDecayIndex]->position().z()
                    << ", associated daughter IDs = ";

#ifdef HEPMC3
                auto vertexPtrVec_particles_out_const_begin=vertexPtrVec[parentDecayIndex]->particles_out().begin();
                auto vertexPtrVec_particles_out_const_end=vertexPtrVec[parentDecayIndex]->particles_out().end();
#else
                auto vertexPtrVec_particles_out_const_begin=vertexPtrVec[parentDecayIndex]->particles_out_const_begin();
                auto vertexPtrVec_particles_out_const_end=vertexPtrVec[parentDecayIndex]->particles_out_const_end();
#endif
                for (auto iter = vertexPtrVec_particles_out_const_begin;
                     iter != vertexPtrVec_particles_out_const_end;
                     iter++)
                  {
                    log << (*iter) << ", ";
                  }

                log << endmsg;
                inconsistency = true;
              }

             //  Nonetheless set the parent decay vertex to be this particle's vertex
             //
             particleVertexIndex = parentDecayIndex;
           }
         else
           {
             //  Now compare the distance between the vertex FROM which the parent originates and the
             //    start of this particle
             //
             HepGeom::Point3D<double> vertex_pos(vertexPtrVec[parentOriginIndex]->position().x(),
                                      vertexPtrVec[parentOriginIndex]->position().y(),
                                      vertexPtrVec[parentOriginIndex]->position().z());
                double distance = vertex_pos.distance(particleStart.vect());

             if (distance > m_vertexOffsetCut && parentIndex == -1)
              {
                //  *** Explicitly handle Hijing bug which generates particle with displaced vertex
                //  ***   but no parent ????
                //

                ATH_MSG_WARNING( "HIJING BUG:: Particle found with displaced vertex but no parent " );
                ATH_MSG_WARNING( "   Particle parameters: " << std::fixed << std::setprecision(2)
                    << std::setw(5) << i << ","
                    << std::setw(7) << m_himain2.patt(i, 1) << ", "
                    << std::setw(7) << m_himain2.patt(i, 2) << ", "
                    << std::setw(7) << m_himain2.patt(i, 3) << ", "
                    << std::setw(7) << m_himain2.katt(i, 1) << ", "
                    << std::setw(7) << m_himain2.katt(i, 2) << ", "
                    << std::setw(7) << m_himain2.katt(i, 3) << ", "
                    << std::setw(7) << m_himain2.katt(i, 4) << ", "
                    << std::setw(7) << m_himain2.vatt(i, 1) << ", "
                    << std::setw(7) << m_himain2.vatt(i, 2) << ", "
                    << std::setw(7) << m_himain2.vatt(i, 3) );

                //  Assign the particle to its parent's vertex
                //
                particleVertexIndex = parentOriginIndex;
                //std::cout << "ML-> case 2 distane = " << distance <<"  particleVertexIndex = "<<particleVertexIndex<< std::endl;
              }
             if( parentIndex!=-1 && ((distance > m_vertexOffsetCut || parentOriginIndex !=0) || m_keepAllDecayVertices ) )
              {
                // We need to create a new vertex
                //
                HepMC::GenVertexPtr newVertex_p = HepMC::newGenVertexPtr(HepMC::FourVector(particleStart.x(),particleStart.y(),particleStart.z(),particleStart.t()));
		evt->add_vertex(newVertex_p);
                vertexPtrVec.push_back(newVertex_p);
                particleVertexIndex = vertexPtrVec.size() - 1;

                //  Now we indicate that the parent has a decay vertex
                //
                partDecayVertex_vec[parentIndex] = particleVertexIndex;

                //  Now tell the vertex about the particle that created it
                //

                newVertex_p->add_particle_in(particleHepPartPtr_vec[parentIndex]);
              }
             else {
              //  Assign the particle to its parent's vertex
              //
              particleVertexIndex = parentOriginIndex;
             }
           }
       }
      else
      {
       //  We have to brute-force search for a vertex that might match this particle
       //
       int foundVert = -1;
       for (unsigned int ivert = 0; ivert < vertexPtrVec.size(); ivert++)
       {

         HepGeom::Point3D<double> vertex_pos(vertexPtrVec[ivert]->position().x(),
                              vertexPtrVec[ivert]->position().y(),
                              vertexPtrVec[ivert]->position().z());
         double distance = vertex_pos.distance(particleStart.vect());
         if (distance < m_vertexOffsetCut)
         {
           foundVert = ivert;
           break;
         }
       }

       if (foundVert == -1)
       {
         //  We need to create a new vertex
         //
         HepMC::GenVertexPtr newVertex_p = HepMC::newGenVertexPtr(HepMC::FourVector(particleStart.x(),particleStart.y(),particleStart.z(),particleStart.t()));

          evt->add_vertex(newVertex_p);
         vertexPtrVec.push_back(newVertex_p);
         particleVertexIndex = vertexPtrVec.size() - 1;
       }
       else
       {
         particleVertexIndex = foundVert;
       }
        //std::cout << "ML-> case 3 particleVertexIndex = "<<particleVertexIndex<< std::endl;
      }

      //  If the Hijing particle has decayed, set the status appropriately
      //
      int particleId = m_himain2.katt(i, 1);
      int particleStatus = 1;
      if (m_himain2.katt(i, 4) == 11) particleStatus = 2;

      // Create the new particle
      //
      HepMC::FourVector particleP4(m_himain2.patt(i, 1),
                              m_himain2.patt(i, 2),
                              m_himain2.patt(i, 3),
                              m_himain2.patt(i, 4));

      HepMC::GenParticlePtr newParticle_p = HepMC::newGenParticlePtr(particleP4, particleId,
                                                         particleStatus);

      //  Record the particle in the vector of pointers
      //    (ostensibly we only need this when we have the
      //     history but for simplicity always do it)
      //
      particleHepPartPtr_vec[i-1] = newParticle_p;

      // Now add the particle to its vertex
      //
      vertexPtrVec[particleVertexIndex]->add_particle_out(newParticle_p);
      partOriginVertex_vec[i-1] = particleVertexIndex;

    }

    if (inconsistency)
      {
       for (int i = 1; i <= m_himain1.natt(); ++i)
         {
           //  Skip non-interacting projectile and target nucleons
           //
           if (!m_spec && ((m_himain2.katt(i, 2) == 0) ||  (m_himain2.katt(i, 2)) == 10)) continue;

           ATH_MSG_WARNING( "  Hijing.cxx  inconsistency: " <<  std::fixed << std::setprecision(2)
             //  std::cout << std::fixed << std::setprecision(2)
              << std::setw(5) << i << ","
              << std::setw(7) << m_himain2.patt(i, 1) << ", "
              << std::setw(7) << m_himain2.patt(i, 2) << ", "
              << std::setw(7) << m_himain2.patt(i, 3) << ", "
              << std::setw(7) << m_himain2.katt(i, 1) << ", "
              << std::setw(7) << m_himain2.katt(i, 2) << ", "
              << std::setw(7) << m_himain2.katt(i, 3) << ", "
              << std::setw(7) << m_himain2.katt(i, 4) << ", "
              << std::setw(7) << m_himain2.vatt(i, 1) << ", "
              << std::setw(7) << m_himain2.vatt(i, 2) << ", "
              << std::setw(7) << m_himain2.vatt(i, 3) );
         }
      }

    // Generate documentation lines for high-pt partons
    //
    for (int isg = 1; isg <= m_hijjet2.nsg(); isg++)
    {
      for (int jparton = 1; jparton <= m_hijjet2.njsg(isg); jparton++)
      {
       double px = m_hijjet2.pxsg(isg, jparton);
       double py = m_hijjet2.pysg(isg, jparton);
       double pz = m_hijjet2.pzsg(isg, jparton);
       double mass = m_hijjet2.pmsg(isg, jparton);

       double ptsq = px*px + py*py;
       double e = std::sqrt(ptsq + pz*pz + mass*mass);
       double mt = std::sqrt(ptsq + mass*mass);
       double pt = std::sqrt(ptsq);

       double pseud = 0.5*std::log((e + pz)/(e - pz));

       int partonId = m_hijjet2.k2sg(isg, jparton);

       if (mt > m_partonStoreMinPt)
       {
         ATH_MSG_DEBUG( "hijjet2 entry: isg = " << isg
             << ", pt = " << pt
             << ", mt = " << mt
             << ", eta = " << pseud );

         //  Add aaa non-tracked entry in the hepmc event with status code 103 (temporary)
         //
         v1->add_particle_out( HepMC::newGenParticlePtr( HepMC::FourVector(px, py, pz, e), partonId, 103 ) );
       }
      }
    }

    // Generate documentation lines for high-pt partons
    //
    int iap = m_hiparnt.ihnt2(1);  // # of projectile nucleons
    int iat = m_hiparnt.ihnt2(3);  // # of target nucleons

    for (int iproj = 1; iproj <= iap; iproj++)
    {
      for (int jparton = 1; jparton <= m_hijjet1.npj(iproj); jparton++)
      {
       double px = m_hijjet1.pjpx(iproj, jparton);
       double py = m_hijjet1.pjpy(iproj, jparton);
       double pz = m_hijjet1.pjpz(iproj, jparton);
       double mass = m_hijjet1.pjpm(iproj, jparton);

       double ptsq = px*px + py*py;
       double e = std::sqrt(ptsq + pz*pz + mass*mass);
       double mt = std::sqrt(ptsq + mass*mass);
       double pt = std::sqrt(ptsq);

       double pseud = 0.5*std::log((e + pz)/(e - pz));

       int partonId = m_hijjet1.kfpj(iproj, jparton);

       if (mt > m_partonStoreMinPt)
       {
         ATH_MSG_DEBUG( "hijjet1 entry: iproj = " << iproj
             << ", pt = " << pt
             << ", mt = " << mt
             << ", eta = " << pseud );

         //  Add aaa non-tracked entry in the hepmc event with status code 103 (temporary)
         //
         v1->add_particle_out( HepMC::newGenParticlePtr( HepMC::FourVector(px, py, pz, e), partonId, 103 ) );
       }
      }
    }

    //  Now for target nucleons
    //
    for (int itarg = 1; itarg <= iat; itarg++)
    {
      for (int jparton = 1; jparton <= m_hijjet1.ntj(itarg); jparton++)
      {
       double px = m_hijjet1.pjtx(itarg, jparton);
       double py = m_hijjet1.pjty(itarg, jparton);
       double pz = m_hijjet1.pjtz(itarg, jparton);
       double mass = m_hijjet1.pjtm(itarg, jparton);

       double ptsq = px*px + py*py;
       double e = std::sqrt(ptsq + pz*pz + mass*mass);
       double mt = std::sqrt(ptsq + mass*mass);
       double pt = std::sqrt(ptsq);

       double pseud = 0.5*std::log((e + pz)/(e - pz));

       int partonId = m_hijjet1.kftj(itarg, jparton);

       if (mt > m_partonStoreMinPt)
       {
         ATH_MSG_DEBUG( "hijjet1 entry: itarg = " << itarg
             << ", pt = " << pt
             << ", mt = " << mt
             << ", eta = " << pseud );

         //  Add aaa non-tracked entry in the hepmc event with status code 103 (temporary)
         //
         v1->add_particle_out( HepMC::newGenParticlePtr( HepMC::FourVector(px, py, pz, e), partonId, 103 ) );
       }
      }
    }

    // Convert CLHEP::cm->CLHEP::mm and CLHEP::GeV->CLHEP::MeV
    //
    GeVToMeV(evt);

    //BPK-> Loop over the particles in the event, if p needs to be mirrored:
    if( m_prand ){
      HepMC::FourVector tmpmom(0.,0.,0.,0.);
      double ranz = CLHEP::RandFlat::shoot(p_Engine);
      //      std::cout <<"random="<<ranz <<std::endl;
      if (ranz < 0.5) {
       //      std::cout <<"flip="<<ranz <<std::endl;
      for(auto  pitr: *evt){
       tmpmom= pitr->momentum();
       tmpmom.setX(-tmpmom.x());
       tmpmom.setY(-tmpmom.y());
       tmpmom.setZ(-tmpmom.z());
       tmpmom.setT(tmpmom.t());
       pitr->set_momentum(tmpmom);
       }
      }
    }
    //BPK-<

    //ARA -- ATLHI-483, clean up unstable particles with no decay vertex
#ifdef HEPMC3
    if(m_keepAllDecayVertices)
    {
      const std::vector <HepMC::GenParticlePtr> allParticles=evt->particles();
      for(auto p : allParticles)
      {
        HepMC::ConstGenVertexPtr end_v=p->end_vertex();
        if(p->status() == 2 && !end_v) evt->remove_particle(p);
      }
    }
#else
    if(m_keepAllDecayVertices)  
    {
      for (HepMC::GenParticle* p : *evt) {
        HepMC::ConstGenVertexPtr end_v = p->end_vertex();
        if (p->status() == 2 && !end_v) delete p->production_vertex()->remove_particle(p);
      }
    }
#endif
    return StatusCode::SUCCESS;
}

CLHEP::HepLorentzVector
Hijing::randomizeVertex(CLHEP::HepRandomEngine* engine)
{
  // Check the range in Z for the correct pipe diameter
  // Definitions of constants are in VertexShift.h

  using namespace VertexShift;

  double ranx, rany, xmax, ymax;
  double ranz = CLHEP::RandFlat::shoot(engine, -Zmax, Zmax);
  if( m_wide ){ // Allow the whole pipe
    if( std::abs(ranz) < Start1 ) {
      xmax = Xmin + Delta1;
      ymax = xmax;
    } else if( std::abs(ranz) < Start2 ) {
      xmax = Xmin + Delta2;
      ymax = xmax;
    } else if( std::abs(ranz) < Start3 ) {
      xmax = Xmin + Delta3;
      ymax = xmax;
    } else if ( std::abs(ranz) <= Envelope ){
      xmax = Xmin;
      ymax = xmax;
    } else {
      ATH_MSG_ERROR( "**** Hijing::randomizeVertex()  " << ranz << " (z) is outside the detector (units of mm). Returning a centered event." );
      return CLHEP::HepLorentzVector(0.,0.,0.,0.);
    }
  } else {
    ATH_MSG_INFO( "New Coordinates: x=0., y=0., z=" << ranz       );
    return CLHEP::HepLorentzVector(0., 0., ranz, 0); // Allow distribution just along the beam (no "width", m_wide is false (default))
  }
  ranx = CLHEP::RandFlat::shoot(engine, -xmax, xmax);
  rany = CLHEP::RandFlat::shoot(engine, -ymax, ymax);

  ATH_MSG_INFO( "New Coordinates: x=" << ranx << ", y=" << rany << ", z=" << ranz );

  return CLHEP::HepLorentzVector(ranx, rany, ranz, 0);
}

void
Hijing::set_user_params       (void)
{
    // set up the DEFAULT input parameters to hijset: these can be changed by the user ...
    m_efrm       = 14000.;
    m_frame       = "CMS     ";
    m_proj       = "P       ";
    m_targ       = "P       ";
    m_iap             = 1;
    m_iat             = 1;
    m_izp             = 1;
    m_izt             = 1;
    // ... and to hijing
    m_bmin       = 0.;
    m_bmax       = 0.;

    // Set user Initialization parameters
    for(CommandVector::iterator i = m_InitializeVector.begin(); i != m_InitializeVector.end(); ++i )
    {
       ATH_MSG_INFO( " Hijing init. Command is: " << *i );
       StringParse mystring(*i);
       std::string myparam = mystring.piece<std::string>(1);
       if (myparam == "efrm")
       {
           m_efrm = mystring.piece<double>(2);
       }
       else if (myparam == "frame")
       {
           m_frame       = mystring.piece<std::string>(2);
           if (m_frame.size() < 8)
           {
              unsigned nbl = 8 - m_frame.size();
              for (unsigned i = 0; i < nbl; ++i) m_frame += ' ';
           }
       }
       else if (myparam == "proj")
       {
           m_proj       = mystring.piece<std::string>(2);
           if (m_proj.size() < 8)
           {
              unsigned nbl = 8 - m_proj.size();
              for (unsigned i = 0; i < nbl; ++i) m_proj += ' ';
           }
       }
       else if (myparam == "targ")
       {
           m_targ       = mystring.piece<std::string>(2);
           if (m_targ.size() < 8)
           {
              unsigned nbl = 8 - m_targ.size();
              for (unsigned i = 0; i < nbl; ++i) m_targ += ' ';
           }
       }
       else if (myparam == "iap")
       {
           m_iap       = mystring.piece<int>(2);
       }
       else if (myparam == "izp")
       {
           m_izp       = mystring.piece<int>(2);
       }
       else if (myparam == "iat")
       {
           m_iat       = mystring.piece<int>(2);
       }
       else if (myparam == "izt")
       {
           m_izt       = mystring.piece<int>(2);
       }
       else if (myparam == "bmin")
       {
           m_bmin       = mystring.piece<double>(2);
       }
       else if (myparam == "bmax")
       {
           m_bmax       = mystring.piece<double>(2);
       }
       else if (myparam == "nseed")
       {
           m_ranseed.nseed()         = mystring.piece<int>(2);
       }
       else if (myparam == "hipr1")
       {
           int              myelem  = mystring.piece<int>(2);
           m_hiparnt.hipr1(myelem) = mystring.piece<double>(3);
       }
       else if (myparam == "ihpr2")
       {
           int              myelem  = mystring.piece<int>(2);
           m_hiparnt.ihpr2(myelem) = mystring.piece<int>(3);
       }
       else if (myparam == "hint1")
       {
           int              myelem  = mystring.piece<int>(2);
           m_hiparnt.hint1(myelem) = mystring.piece<double>(3);
       }
       else if (myparam == "ihnt2")
       {
           int              myelem  = mystring.piece<int>(2);
           m_hiparnt.ihnt2(myelem) = mystring.piece<int>(3);
       }
       else
       {
           ATH_MSG_ERROR( " ERROR in HIJING INITIALIZATION PARAMETERS   " << myparam << " is an invalid parameter !" );
       }
    }
}
