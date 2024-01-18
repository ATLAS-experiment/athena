/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <boost/test/tools/old/interface.hpp>
#undef NDEBUG
#define BOOST_TEST_MODULE MultiTrajectoryBasic_test
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>
#include <Acts/EventData/TrackContainer.hpp>

#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackStorageContainer.h"
#include "xAODTracking/TrackSummaryContainer.h"
#include "xAODTracking/TrackSummaryAuxContainer.h"

#include "xAODTracking/TrackSummary.h"
#include "xAODTracking/TrackSurface.h"
#include "xAODTracking/TrackSurfaceContainer.h"
#include "xAODTracking/TrackSurfaceAuxContainer.h"


BOOST_AUTO_TEST_SUITE(EventDataTrackStorage)

BOOST_AUTO_TEST_CASE(ConstCompilesWithInterface) {
  ACTS_STATIC_CHECK_CONCEPT(Acts::ConstTrackContainerBackend,
                            ActsTrk::TrackStorageContainer);
}

BOOST_AUTO_TEST_CASE(MutableCompilesWithInterface) {
  ACTS_STATIC_CHECK_CONCEPT(Acts::TrackContainerBackend,
                            ActsTrk::MutableTrackStorageContainer);

  using MutableTrackContainer = Acts::TrackContainer<ActsTrk::MutableTrackStorageContainer, ActsTrk::MutableMultiTrajectory>;
}

struct EmptyBackend {
  EmptyBackend() {
    m = std::make_unique<ActsTrk::MutableTrackStorageContainer>();
  }
  std::unique_ptr<ActsTrk::MutableTrackStorageContainer> m;
};

BOOST_FIXTURE_TEST_CASE(AllStaticxAODVaraiblesAreKnown, EmptyBackend) {
  for (auto id : m->trackBackend()->getConstStore()->getAuxIDs()) {
    const std::string name = SG::AuxTypeRegistry::instance().getName(id);
    BOOST_CHECK( ActsTrk::TrackStorageContainer::staticVariables.count(name) == 1);
  }
  BOOST_CHECK( m->trackBackend()->getConstStore()->getAuxIDs().size() == ActsTrk::TrackStorageContainer::staticVariables.size());
}


struct FilledBackend : public EmptyBackend {
  FilledBackend() 
  {
    m->addColumn_impl<short>("author");
    m->addTrack_impl();

    using namespace Acts::HashedStringLiteral;
    *std::any_cast<float*>(m->component_impl("chi2"_hash, 0)) = 2.5f;
    *std::any_cast<ActsTrk::IndexType*>(m->component_impl("tipIndex"_hash, 0)) = 8;
    *std::any_cast<unsigned int*>(m->component_impl("nHoles"_hash, 0)) = 2;
    *std::any_cast<short*>(m->component_impl("author"_hash, 0)) = 77;
    m->addTrack_impl();
  }
};



BOOST_FIXTURE_TEST_CASE(BareContainerFill, FilledBackend) {
  using namespace Acts::HashedStringLiteral;

  BOOST_CHECK_EQUAL(m->size_impl(), 2);
  BOOST_CHECK(m->hasColumn_impl("author"_hash));
  BOOST_CHECK(m->hasColumn_impl("tipIndex"_hash));

  BOOST_CHECK(m->hasColumn_impl("z0"_hash) == false);

  BOOST_CHECK_EQUAL(*std::any_cast<float*>(m->component_impl("chi2"_hash, 0)), 2.5f);
  BOOST_CHECK_EQUAL(*std::any_cast<unsigned int*>(m->component_impl("nHoles"_hash, 0)), 2);
  BOOST_CHECK_EQUAL(*std::any_cast<short*>(m->component_impl("author"_hash, 0)), 77);
  BOOST_CHECK_EQUAL(*std::any_cast<ActsTrk::IndexType*>(m->component_impl("tipIndex"_hash, 0)), 8);
    
  BOOST_CHECK_EQUAL(*std::any_cast<float*>(m->component_impl("chi2"_hash, 1)), 0.0f);
  BOOST_CHECK_EQUAL(*std::any_cast<unsigned int*>(m->component_impl("nHoles"_hash, 1)), 0);
  BOOST_CHECK_EQUAL(*std::any_cast<short*>(m->component_impl("author"_hash, 1)), 0);
  BOOST_CHECK_EQUAL(*std::any_cast<ActsTrk::IndexType*>(m->component_impl("tipIndex"_hash, 1)), 0);

}


BOOST_FIXTURE_TEST_CASE(ImmutableAccess, FilledBackend) {
  using namespace Acts::HashedStringLiteral;

  auto c = std::make_unique<ActsTrk::TrackStorageContainer>(m->trackBackend());
  c->restoreDecorations();
  BOOST_CHECK_EQUAL(c->size_impl(), 2);
  BOOST_CHECK(c->hasColumn_impl("author"_hash));
  BOOST_CHECK(c->hasColumn_impl("z0"_hash) == false);

  BOOST_CHECK_EQUAL(*std::any_cast<const float*>(c->component_impl("chi2"_hash, 0)), 2.5f);
  BOOST_CHECK_EQUAL(*std::any_cast<const unsigned int*>(c->component_impl("nHoles"_hash, 0)), 2);

  BOOST_CHECK_EQUAL(*std::any_cast<const short*>(c->component_impl("author"_hash, 0)), 77);

  BOOST_CHECK_EQUAL(*std::any_cast<const float*>(c->component_impl("chi2"_hash, 1)), 0.0f);
  BOOST_CHECK_EQUAL(*std::any_cast<const unsigned int*>(c->component_impl("nHoles"_hash, 1)), 0);
  BOOST_CHECK_EQUAL(*std::any_cast<const short*>(c->component_impl("author"_hash, 1)), 0);


}


/////////////////////////////////////////////////////////////////////
// Test of TrackSurfaceContainer in mutable TrackStorageContainer  

BOOST_FIXTURE_TEST_CASE(MutableSurfaceBackend_test, FilledBackend){

//TODO: Here the empty surface is created. Tests of surfase filling are needed

  auto i = m->addSurface_impl();
  BOOST_CHECK_EQUAL(i, 0);

  i = m->addSurface_impl();
  BOOST_CHECK_EQUAL(i, 1); 

  m->removeSurface_impl(i);
  i = m->addSurface_impl();
  BOOST_CHECK_EQUAL(i, 1);
};


template<typename surfType>
void testSurface(surfType surf, std::shared_ptr<const Acts::Surface> outSurf, const ActsGeometryContext& gctx) {
    BOOST_CHECK_EQUAL(int(surf->type()), int(outSurf->type()));
    BOOST_CHECK_EQUAL(surf->center(gctx.context()), outSurf->center(gctx.context()));
    BOOST_CHECK_EQUAL(surf->transform(gctx.context()).rotation().eulerAngles(2, 1, 0), 
                  outSurf->transform(gctx.context()).rotation().eulerAngles(2, 1, 0));  
    BOOST_CHECK_EQUAL(size(surf->bounds().values()), size(outSurf->bounds().values()));
    for (unsigned int i=0; i<size(surf->bounds().values()); i++)  {  
      BOOST_TEST(surf->bounds().values()[i] == outSurf->bounds().values()[i], boost::test_tools::tolerance(0.001));
    }      
} 


// Test of const TrackStorageContainer with TrackSurfaceContainer
BOOST_AUTO_TEST_CASE(ConstSurfaceBackend_test){

    // Create filled xAOD::TrackSummaryContainer
    constexpr static size_t sz = 6;

    xAOD::TrackSummaryContainer backend;
    xAOD::TrackSummaryAuxContainer aux;
    backend.setStore(&aux);

    std::vector<double> semirandoms = {0.12, 0.92};
    for (const double sr : semirandoms) {    
        auto par = new xAOD::TrackSummary();    
        backend.push_back(par);
        par->resize();
        for ( size_t i = 0; i < sz; ++i) {
            par->paramsEigen()(i) = i * sr;
            for ( size_t j = 0; j < sz; ++j) {
                par->covParamsEigen()(i, j) = (i+j) * sr;
            }
        }
    }

  
  // Create filled xAOD::TrackSurfaceContainer
  const ActsGeometryContext& gctx{};

  float layerZ = 30.;
  Acts::Transform3 transform(Acts::Translation3(0., 0., -layerZ));
  float rotation[3] = {2.1, 1.2, 0.4};
  transform *= Acts::AngleAxis3(rotation[0], Acts::Vector3(0., 0., 1.));  //rotZ
  transform *= Acts::AngleAxis3(rotation[1], Acts::Vector3(0., 1., 0.));  //rotY
  transform *= Acts::AngleAxis3(rotation[2], Acts::Vector3(1., 0., 0.));  //rotX

  xAOD::TrackSurfaceContainer surfBackend;
  xAOD::TrackSurfaceAuxContainer aux0;
  surfBackend.setStore(&aux0);
  
  auto surfCurr = new xAOD::TrackSurface();
  surfBackend.push_back(surfCurr);

  // create the ConeSurface
  double alpha(M_PI/4.), minZ(5.), maxZ(25), halfPhi(M_PI);
  auto surf = Acts::Surface::makeShared<Acts::ConeSurface>(
              transform, alpha, minZ, maxZ, halfPhi);                   
  ActsTrk::encodeSurface(surfCurr, surf.get(), gctx);

  // Create constant ActsTrk::TrackStorageContainer
  std::unique_ptr<ActsTrk::TrackStorageContainer> ms = std::make_unique<ActsTrk::TrackStorageContainer>(&backend, &aux0);

  // Read the ActsTrk::TrackStorageContainer and check track storage and track surfaces
  auto cc = std::make_unique<ActsTrk::TrackStorageContainer>(ms->trackBackend());
  BOOST_CHECK_EQUAL(cc->size_impl(), 2);
  
  auto outSurf = ActsTrk::decodeSurface(surfBackend[0], gctx);
  testSurface(surf, outSurf, gctx);
  
};


BOOST_AUTO_TEST_SUITE_END()
