///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// McEventCollectionCnv_p4.cxx
// Implementation file for class McEventCollectionCnv_p4
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////


// STL includes
#include <utility>
#include <cmath>
#include <cfloat> // for DBL_EPSILON

// GeneratorObjectsTPCnv includes
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p4.h"
#include "HepMcDataPool.h"

#include "GenInterfaces/IHepMCWeightSvc.h"

#include "McEventCollectionCnv_utils.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "TruthUtils/MagicNumbers.h"

///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////


McEventCollectionCnv_p4::McEventCollectionCnv_p4() :
  Base_t( ),
  m_isPileup(false),m_hepMCWeightSvc("HepMCWeightSvc","McEventCollectionCnv_p4")
{}

McEventCollectionCnv_p4::McEventCollectionCnv_p4( const McEventCollectionCnv_p4& rhs ) :
  Base_t( rhs ),
  m_isPileup(false),m_hepMCWeightSvc("HepMCWeightSvc","McEventCollectionCnv_p4")
{}

McEventCollectionCnv_p4&
McEventCollectionCnv_p4::operator=( const McEventCollectionCnv_p4& rhs )
{
  if ( this != &rhs ) {
    Base_t::operator=( rhs );
    m_isPileup=rhs.m_isPileup;
    m_hepMCWeightSvc = rhs.m_hepMCWeightSvc;
  }
  return *this;
}

// Destructor
///////////////

McEventCollectionCnv_p4::~McEventCollectionCnv_p4()
= default;

///////////////////////////////////////////////////////////////////
// Const methods:
///////////////////////////////////////////////////////////////////

void McEventCollectionCnv_p4::persToTrans( const McEventCollection_p4* persObj,
                                           McEventCollection* transObj,
                                           MsgStream& msg )
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  msg << MSG::DEBUG << "Loading McEventCollection from persistent state..."
      << endmsg;

  // elements are managed by DataPool
  if (!m_isPileup)
  {
    transObj->clear(SG::VIEW_ELEMENTS);
  }
  HepMC::DataPool datapools;
  const unsigned int nVertices = persObj->m_genVertices.size();
  datapools.vtx.prepareToAdd(nVertices);
  const unsigned int nParts = persObj->m_genParticles.size();
  datapools.part.prepareToAdd(nParts);
  const unsigned int nEvts = persObj->m_genEvents.size();
  datapools.evt.prepareToAdd(nEvts);

  transObj->reserve( nEvts );
  for ( std::vector<GenEvent_p4>::const_iterator
          itr = persObj->m_genEvents.begin(),
          itrEnd = persObj->m_genEvents.end();
        itr != itrEnd;
        ++itr )
    {
      const GenEvent_p4& persEvt = *itr;
      HepMC::GenEvent * genEvt(nullptr);
      if(m_isPileup)
        {
          genEvt = new HepMC::GenEvent();
        }
      else
        {
          genEvt        =  datapools.getGenEvent();
        }
      genEvt->add_attribute (HepMCStr::barcodes, std::make_shared<HepMC::GenEventBarcodes>());
      genEvt->add_attribute(HepMCStr::signal_process_id, std::make_shared<HepMC3::IntAttribute>(persEvt.m_signalProcessId));
      genEvt->set_event_number(persEvt.m_eventNbr);
      genEvt->add_attribute(HepMCStr::event_scale, std::make_shared<HepMC3::DoubleAttribute>(persEvt.m_eventScale));
      genEvt->add_attribute(HepMCStr::alphaQCD, std::make_shared<HepMC3::DoubleAttribute>(persEvt.m_alphaQCD));
      genEvt->add_attribute(HepMCStr::alphaQED, std::make_shared<HepMC3::DoubleAttribute>(persEvt.m_alphaQED));
      genEvt->weights() = persEvt.m_weights;
      genEvt->add_attribute(HepMCStr::random_states, std::make_shared<HepMC3::VectorLongIntAttribute>(persEvt.m_randomStates));
      //restore weight names from the dedicated svc (which was keeping them in metadata for efficiency)
      if(!genEvt->run_info()) genEvt->set_run_info(std::make_shared<HepMC3::GenRunInfo>());
      if(genEvt->run_info()) genEvt->run_info()->set_weight_names(m_hepMCWeightSvc->weightNameVec(ctx));


       // pdfinfo restore
      if (!persEvt.m_pdfinfo.empty())
        {
          const std::vector<double>& pdf = persEvt.m_pdfinfo;
              HepMC3::GenPdfInfoPtr pi = std::make_shared<HepMC3::GenPdfInfo>();
              pi->set(
              static_cast<int>(pdf[6]), // id1
              static_cast<int>(pdf[5]), // id2
              pdf[4],                   // x1
              pdf[3],                   // x2
              pdf[2],                   // scalePDF
              pdf[1],                   // pdf1
              pdf[0] );                 // pdf2
              genEvt->set_pdf_info(std::move(pi));
        }

      transObj->push_back( genEvt );

      // create a temporary map associating the barcode of an end-vtx to its
      // particle.
      // As not all particles are stable (d'oh!) we take 50% of the number of
      // particles as an initial size of the hash-map (to prevent re-hash)
      ParticlesMap_t partToEndVtx( (persEvt.m_particlesEnd-persEvt.m_particlesBegin)/2 );
      // This is faster than the HepMC::barcode_to_vertex
      std::map<int, HepMC::GenVertexPtr> brc_to_vertex;
      // create the vertices
      const unsigned int endVtx = persEvt.m_verticesEnd;
      for ( unsigned int iVtx= persEvt.m_verticesBegin; iVtx != endVtx; ++iVtx )
        {
         auto vtx = createGenVertex( *persObj, persObj->m_genVertices[iVtx], partToEndVtx, datapools, genEvt );
         brc_to_vertex[persObj->m_genVertices[iVtx].m_barcode] = std::move(vtx);
        } //> end loop over vertices

        // set the signal process vertex
        const int sigProcVtx = persEvt.m_signalProcessVtx;
        if ( sigProcVtx != 0 && brc_to_vertex.count(sigProcVtx) ) {
          HepMC::set_signal_process_vertex(genEvt, brc_to_vertex[sigProcVtx] );
        }

        // connect particles to their end vertices
        for (auto & p : partToEndVtx) {
          if ( brc_to_vertex.count(p.second) ) {
            auto decayVtx = brc_to_vertex[p.second];
            decayVtx->add_particle_in( p.first );
          } else {
          msg << MSG::ERROR << "GenParticle points to null end vertex !!" << endmsg;
          }
         }

    } //> end loop over m_genEvents

  msg << MSG::DEBUG << "Loaded McEventCollection from persistent state [OK]"
      << endmsg;
}

void McEventCollectionCnv_p4::transToPers( const McEventCollection* transObj,
                                           McEventCollection_p4* persObj,
                                           MsgStream& msg )
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  msg << MSG::DEBUG << "Creating persistent state of McEventCollection..."
      << endmsg;
  persObj->m_genEvents.reserve( transObj->size() );

  const std::pair<unsigned int,unsigned int> stats = nbrParticlesAndVertices( transObj );
  persObj->m_genParticles.reserve( stats.first  );
  persObj->m_genVertices.reserve ( stats.second );

  const McEventCollection::const_iterator itrEnd = transObj->end();
  for ( McEventCollection::const_iterator itr = transObj->begin();
        itr != itrEnd;
        ++itr )
    {
      const unsigned int nPersVtx   = persObj->m_genVertices.size();
      const unsigned int nPersParts = persObj->m_genParticles.size();
      const HepMC::GenEvent* genEvt = *itr;
      //save the weight names to metadata via the HepMCWeightSvc
      if (genEvt->run_info()) {
        if (!genEvt->run_info()->weight_names().empty()) {
          m_hepMCWeightSvc->setWeightNames(  names_to_name_index_map(genEvt->weight_names()), ctx ).ignore();
        } else {
          //AV : This to be decided if one would like to have default names.
          //std::vector<std::string> names{"0"};
          //m_hepMCWeightSvc->setWeightNames( names_to_name_index_map(names), ctx );
        }
      }
      auto A_signal_process_id=genEvt->attribute<HepMC3::IntAttribute>(HepMCStr::signal_process_id);
      auto A_event_scale=genEvt->attribute<HepMC3::DoubleAttribute>(HepMCStr::event_scale);
      auto A_alphaQCD=genEvt->attribute<HepMC3::DoubleAttribute>(HepMCStr::alphaQCD);
      auto A_alphaQED=genEvt->attribute<HepMC3::DoubleAttribute>(HepMCStr::alphaQED);
      auto signal_process_vertex = HepMC::signal_process_vertex(genEvt);
      auto A_random_states=genEvt->attribute<HepMC3::VectorLongIntAttribute>(HepMCStr::random_states);

      persObj->m_genEvents.
      emplace_back( A_signal_process_id?(A_signal_process_id->value()):0,
                                genEvt->event_number(),
                                A_event_scale?(A_event_scale->value()):0.0,
                                A_alphaQCD?(A_alphaQCD->value()):0.0,
                                A_alphaQED?(A_alphaQED->value()):0.0,
                                signal_process_vertex?HepMC::barcode(signal_process_vertex):0,
                                genEvt->weights(),
                                std::vector<double>(),//No idea why it is empty
                                A_random_states?(A_random_states->value()):std::vector<long>(),
                                nPersVtx,
                                nPersVtx + genEvt->vertices().size(),
                                nPersParts,
                                nPersParts + genEvt->particles().size() );

      //PdfInfo encoding
   if (genEvt->pdf_info())
        {
          auto pi=genEvt->pdf_info();
          GenEvent_p4& persEvt = persObj->m_genEvents.back();
          std::vector<double>& pdfinfo = persEvt.m_pdfinfo;
          pdfinfo.resize(7);
          pdfinfo[6] = static_cast<double>(pi->parton_id[0]);
          pdfinfo[5] = static_cast<double>(pi->parton_id[1]);
          pdfinfo[4] = pi->x[0];
          pdfinfo[3] = pi->x[1];
          pdfinfo[2] = pi->scale;
          pdfinfo[1] = pi->xf[0];
          pdfinfo[0] = pi->xf[1];
        }
      // create vertices
      for ( const auto& v: genEvt->vertices())
        {
          writeGenVertex( v, *persObj );
        }

    } //> end loop over GenEvents

  msg << MSG::DEBUG << "Created persistent state of HepMC::GenEvent [OK]"
      << endmsg;
}


HepMC::GenVertexPtr
McEventCollectionCnv_p4::createGenVertex( const McEventCollection_p4& persEvt,
                                          const GenVertex_p4& persVtx,
                                          ParticlesMap_t& partToEndVtx,
                                          HepMC::DataPool& datapools, HepMC::GenEvent* parent ) const
{
  HepMC::GenVertexPtr vtx(nullptr);
  if(m_isPileup)
    {
      vtx=HepMC::newGenVertexPtr();
    }
  else
    {
      vtx = datapools.getGenVertex();
    }
  if (parent) parent->add_vertex(vtx);
  vtx->set_position(HepMC::FourVector( persVtx.m_x , persVtx.m_y , persVtx.m_z ,persVtx.m_t ));
  vtx->set_status(HepMC::new_vertex_status_from_old(persVtx.m_id, persVtx.m_barcode)); // UPDATED STATUS VALUE TO NEW SCHEME
  // cast from std::vector<float> to std::vector<double>
  std::vector<double> weights( persVtx.m_weights.begin(), persVtx.m_weights.end() );
  vtx->add_attribute(HepMCStr::weights,std::make_shared<HepMC3::VectorDoubleAttribute>(weights));
  HepMC::suggest_barcode(vtx,persVtx.m_barcode);

  // handle the in-going (orphans) particles
  //Is this needed in HepMC3?
  const unsigned int nPartsIn = persVtx.m_particlesIn.size();
  for ( unsigned int i = 0; i != nPartsIn; ++i )
    {
      createGenParticle( persEvt.m_genParticles[persVtx.m_particlesIn[i]], partToEndVtx, datapools, vtx, false );
    }

  // now handle the out-going particles
  const unsigned int nPartsOut = persVtx.m_particlesOut.size();
  for ( unsigned int i = 0; i != nPartsOut; ++i )
    {
      createGenParticle( persEvt.m_genParticles[persVtx.m_particlesOut[i]], partToEndVtx, datapools, vtx );
    }

  return vtx;
}

HepMC::GenParticlePtr
McEventCollectionCnv_p4::createGenParticle( const GenParticle_p4& persPart,
                                            ParticlesMap_t& partToEndVtx,
                                            HepMC::DataPool& datapools, const HepMC::GenVertexPtr& parent, bool add_to_output ) const
{
  HepMC::GenParticlePtr p(nullptr);
  if (m_isPileup)
    {
      p = HepMC::newGenParticlePtr();
    }
  else
    {
      p    = datapools.getGenParticle();
    }
  if (parent) add_to_output?parent->add_particle_out(p):parent->add_particle_in(p);
  p->set_pdg_id(              persPart.m_pdgId);
  p->set_status(HepMC::new_particle_status_from_old(persPart.m_status, persPart.m_barcode)); // UPDATED STATUS VALUE TO NEW SCHEME
  p->add_attribute(HepMCStr::phi,std::make_shared<HepMC3::DoubleAttribute>(persPart.m_phiPolarization));
  p->add_attribute(HepMCStr::theta,std::make_shared<HepMC3::DoubleAttribute>(persPart.m_thetaPolarization));
  HepMC::suggest_barcode(p,persPart.m_barcode);

  // Note: do the E calculation in extended (long double) precision.
  // That happens implicitly on x86 with optimization on; saying it
  // explicitly ensures that we get the same results with and without
  // optimization.  (If this is a performance issue for platforms
  // other than x86, one could change to double for those platforms.)
  if ( 0 == persPart.m_recoMethod )
    {
      double temp_e = std::sqrt( (long double)(persPart.m_px)*persPart.m_px +
                            (long double)(persPart.m_py)*persPart.m_py +
                            (long double)(persPart.m_pz)*persPart.m_pz +
                            (long double)(persPart.m_m) *persPart.m_m );
      p->set_momentum( HepMC::FourVector(persPart.m_px,persPart.m_py,persPart.m_pz,temp_e));
    }
  else
    {
      const int signM2 = ( persPart.m_m >= 0. ? 1 : -1 );
      const double persPart_ene =
        std::sqrt( std::abs((long double)(persPart.m_px)*persPart.m_px +
                  (long double)(persPart.m_py)*persPart.m_py +
                  (long double)(persPart.m_pz)*persPart.m_pz +
                  signM2* (long double)(persPart.m_m)* persPart.m_m));
      const int signEne = ( persPart.m_recoMethod == 1 ? 1 : -1 );
     p->set_momentum( HepMC::FourVector( persPart.m_px,
                         persPart.m_py,
                         persPart.m_pz,
                         signEne * persPart_ene ));
    }

  // setup flow
  std::vector<int> flows;
  const unsigned int nFlow = persPart.m_flow.size();
  for ( unsigned int iFlow= 0; iFlow != nFlow; ++iFlow ) {
  flows.push_back(persPart.m_flow[iFlow].second );
  }
  //We construct it here as vector w/o gaps.
  p->add_attribute(HepMCStr::flows, std::make_shared<HepMC3::VectorIntAttribute>(flows));

  if ( persPart.m_endVtx != 0 )
    {
      partToEndVtx[p] = persPart.m_endVtx;
    }

  return p;
}

void McEventCollectionCnv_p4::writeGenVertex( const HepMC::ConstGenVertexPtr& vtx,
                                              McEventCollection_p4& persEvt )
{
  const HepMC::FourVector& position = vtx->position();
  auto A_weights=vtx->attribute<HepMC3::VectorDoubleAttribute>(HepMCStr::weights);
  auto A_barcode=vtx->attribute<HepMC3::IntAttribute>(HepMCStr::barcode);
  std::vector<float> weights;
  if (A_weights) {
    auto weights_d = A_weights->value();
    for (auto& w: weights_d) weights.push_back(w);
  }
  persEvt.m_genVertices.emplace_back( position.x(),
                                                position.y(),
                                                position.z(),
                                                position.t(),
                                                HepMC::old_vertex_status_from_new(vtx->status()), // REVERTED STATUS VALUE TO OLD SCHEME
                                                weights.begin(),
                                                weights.end(),
                                                A_barcode?(A_barcode->value()):vtx->id()
                                                );
  GenVertex_p4& persVtx = persEvt.m_genVertices.back();
  // we write only the orphans in-coming particles and beams
  persVtx.m_particlesIn.reserve(vtx->particles_in().size());
  for ( const auto& p: vtx->particles_in())
    {
      if ( !p->production_vertex() || p->production_vertex()->id() == 0 )
        {
          persVtx.m_particlesIn.push_back( writeGenParticle(p, persEvt ));
        }
    }
  persVtx.m_particlesOut.reserve(vtx->particles_out().size());
  for ( const auto& p: vtx->particles_out())
    {
      persVtx.m_particlesOut.push_back( writeGenParticle(p, persEvt ) );
    }
  }

int McEventCollectionCnv_p4::writeGenParticle( const HepMC::ConstGenParticlePtr& p,
                                               McEventCollection_p4& persEvt )
{
  const HepMC::FourVector& mom = p->momentum();
  const double ene = mom.e();
  const double m2  = mom.m2();

  // Definitions of Bool isTimeLilike, isSpacelike and isLightlike according to HepLorentzVector definition
  const bool useP2M2 = !(m2 > 0) &&   // !isTimelike
    (m2 < 0) &&   //  isSpacelike
    !(std::abs(m2) < 2.0*DBL_EPSILON*ene*ene); // !isLightlike

    const short recoMethod = ( !useP2M2 ? 0: ( ene >= 0. ? 1 : 2 ) );
    auto A_theta=p->attribute<HepMC3::DoubleAttribute>(HepMCStr::theta);
    auto A_phi=p->attribute<HepMC3::DoubleAttribute>(HepMCStr::phi);
    auto A_flows=p->attribute<HepMC3::VectorIntAttribute>(HepMCStr::flows);


    persEvt.m_genParticles.emplace_back( mom.px(),
                               mom.py(),
                               mom.pz(),
                               mom.m(),
                               p->pdg_id(),
                               HepMC::old_particle_status_from_new(p->status()), // REVERTED STATUS VALUE TO OLD SCHEME
                               A_flows?(A_flows->value().size()):0,
                               A_theta?(A_theta->value()):0.0,
                               A_phi?(A_phi->value()):0.0,
                               p->production_vertex()?(HepMC::barcode(p->production_vertex())):0,
                               p->end_vertex()?(HepMC::barcode(p->end_vertex())):0,
                               HepMC::barcode(p),
                               recoMethod );

  std::vector< std::pair<int,int> > flow_hepmc2;
  if(A_flows) flow_hepmc2=vector_to_vector_int_int(A_flows->value());
  persEvt.m_genParticles.back().m_flow.assign( flow_hepmc2.begin(),flow_hepmc2.end() );
  // we return the index of the particle in the big vector of particles
  // (contained by the persistent GenEvent)
  return (persEvt.m_genParticles.size() - 1);
}

void McEventCollectionCnv_p4::setPileup()
{
  m_isPileup = true;
}
