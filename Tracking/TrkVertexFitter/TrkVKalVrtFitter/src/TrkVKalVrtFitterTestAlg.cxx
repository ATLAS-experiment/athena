/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file TrkVKalVrtFitter/src/TrkVKalVrtFitterTestAlg.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2019
 * @brief Algorithm for testing TrkVKalVrtFitter.
 */

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticle.h"
#include "TrkTrack/Track.h"
#include "TrkParticleBase/TrackParticleBase.h"
//undef NDEBUG after EDM
#undef NDEBUG
#include "TrkVKalVrtFitter/VxCascadeInfo.h"
#include "TrkVKalVrtFitterTestAlg.h"
#include "TrkVKalVrtFitter/IVertexCascadeFitter.h"
#include "TestTools/FLOATassert.h"
#include "GaudiKernel/SystemOfUnits.h"
#include  "TrkVKalVrtFitter/IVKalState.h" //unique_ptr needs the deleter
#include "AthContainers/ConstAccessor.h"

#include <cassert>
#include <cmath>

#include "CLHEP/Vector/LorentzVector.h"

using Gaudi::Units::mm;
using Gaudi::Units::MeV;
using Gaudi::Units::GeV;


namespace {


template <class T>
std::vector<const T*> asVec (const std::vector<std::unique_ptr<T> >& v)
{
  std::vector<const T*> ret;
  ret.reserve(v.size());

  for (const std::unique_ptr<T>& p : v) {
    ret.push_back (p.get());
  }
  return ret;
}


std::vector<const Trk::NeutralParameters*>
asVec (const std::vector<std::unique_ptr<Trk::NeutralPerigee> >& v)
{
  std::vector<const Trk::NeutralParameters*> ret;
  ret.reserve(v.size());

  for (const std::unique_ptr<Trk::NeutralPerigee>& p : v) {
    ret.push_back (p.get());
  }
  return ret;
}


AmgSymMatrix(5) cov5()
{
  AmgSymMatrix(5) m;
  m.setIdentity();
  return m;
}


using PerigeeUVec_t = std::vector<std::unique_ptr<Trk::Perigee> >;

using NeutralUVec_t = std::vector<std::unique_ptr<Trk::NeutralPerigee> >;
NeutralUVec_t makeNeutrals1()
{
  Amg::Vector3D pos0 { 0, 0, 0 };

  Amg::Vector3D pos1a { 3*mm, 0.5*mm, -7*mm };
  Amg::Vector3D mom1a { 1000*MeV, 900*MeV, 2000*MeV };
  Amg::Vector3D pos1b { -1*mm, 2.5*mm, 1*mm };
  Amg::Vector3D mom1b { 800*MeV, 1000*MeV, 300*MeV };
  Amg::Vector3D pos1c { -1.5*mm, 1*mm, -3*mm };
  Amg::Vector3D mom1c { 500*MeV, 4000*MeV, 800*MeV };

  NeutralUVec_t ret;

  ret.emplace_back (std::make_unique<Trk::NeutralPerigee>(pos1a, mom1a,  1, pos1a, cov5()));
  ret.emplace_back (std::make_unique<Trk::NeutralPerigee>(pos1b, mom1b,  1, pos1b, cov5()));
  ret.emplace_back (std::make_unique<Trk::NeutralPerigee>(pos1c, mom1c,  1, pos1c, cov5()));

  return ret;
}




// Copied from TrackParticleCreatorTool.
void setDefiningParameters( xAOD::TrackParticle& tp,
                            const Trk::Perigee& perigee )
{
  tp.setDefiningParameters(perigee.parameters()[Trk::d0],
    perigee.parameters()[Trk::z0],
    perigee.parameters()[Trk::phi0],
    perigee.parameters()[Trk::theta],
    perigee.parameters()[Trk::qOverP]);
  const AmgSymMatrix(5)* covMatrix = perigee.covariance();
  // see https://its.cern.ch/jira/browse/ATLASRECTS-645 for justification to comment out the following line
  // assert(covMatrix && covMatrix->rows()==5&& covMatrix->cols()==5);
  std::vector<float> covMatrixVec;
  if( !covMatrix ) ;//ATH_MSG_WARNING("Setting Defining parameters without error matrix");
  else Amg::compress(*covMatrix,covMatrixVec);
  tp.setDefiningParametersCovMatrixVec(covMatrixVec);
  const Amg::Vector3D& surfaceCenter = perigee.associatedSurface().center();
  tp.setParametersOrigin(surfaceCenter.x(), surfaceCenter.y(), surfaceCenter.z() );
}
void setDefiningParameters( xAOD::NeutralParticle& tp,
                            const Trk::NeutralPerigee& perigee )
{
  tp.setDefiningParameters(perigee.parameters()[Trk::d0],
    perigee.parameters()[Trk::z0],
    perigee.parameters()[Trk::phi0],
    perigee.parameters()[Trk::theta],
    perigee.parameters()[Trk::qOverP]);
  const AmgSymMatrix(5)* covMatrix = perigee.covariance();
  // see https://its.cern.ch/jira/browse/ATLASRECTS-645 for justification to comment out the following line
  // assert(covMatrix && covMatrix->rows()==5&& covMatrix->cols()==5);
  std::vector<float> covMatrixVec;
  if( !covMatrix ) ;//ATH_MSG_WARNING("Setting Defining parameters without error matrix");
  else Amg::compress(*covMatrix,covMatrixVec);
  tp.setDefiningParametersCovMatrixVec(covMatrixVec);
  const Amg::Vector3D& surfaceCenter = perigee.associatedSurface().center();
  tp.setParametersOrigin(surfaceCenter.x(), surfaceCenter.y(), surfaceCenter.z() );
}


using xAODTPUVec_t = std::vector<std::unique_ptr<xAOD::TrackParticle> >;
xAODTPUVec_t makexAODTP (PerigeeUVec_t&& perigees)
{
  xAODTPUVec_t tracks;

  for (std::unique_ptr<Trk::Perigee>& p : perigees) {
    auto tp = std::make_unique<xAOD::TrackParticle>();
    tp->makePrivateStore();
    setDefiningParameters (*tp, *p);
    tracks.push_back (std::move (tp));
  }

  return tracks;
}


using xAODNPUVec_t = std::vector<std::unique_ptr<xAOD::NeutralParticle> >;
xAODNPUVec_t makexAODNP (NeutralUVec_t&& perigees)
{
  xAODNPUVec_t tracks;

  for (std::unique_ptr<Trk::NeutralPerigee>& p : perigees) {
    auto tp = std::make_unique<xAOD::NeutralParticle>();
    tp->makePrivateStore();
    setDefiningParameters (*tp, *p);
    tracks.push_back (std::move (tp));
  }

  return tracks;
}


void dumpCovariance (const AmgSymMatrix(5)& m)
{
  for (int i=0; i < 5; i++) {
    for (int j=0; j < 5; j++) {
      std::cout << m(i,j) << ", ";
    }
  }
}


[[maybe_unused]]
void dumpVertex (const xAOD::Vertex& v)
{
  std::cout << "vvv\n";
  std::cout << v.x() << " " << v.y() << " " << v.z() << "\n";
  static const SG::ConstAccessor<short> vertexTypeAcc ("vertexType");
  if (vertexTypeAcc.isAvailable(v)) {
    std::cout << "vertexType " << v.vertexType() << "\n";
  }
  std::cout << "chi2/ndof " << v.chiSquared() << " " << v.numberDoF() << "\n";

  std::cout << "cov ";
  for (float f : v.covariance()) {
    std::cout << f << " ";
  }
  std::cout << "\n";

  static const SG::ConstAccessor<std::vector<ElementLink<xAOD::TrackParticleContainer> > > trackParticleLinksAcc ("trackParticleLinks");
  if (trackParticleLinksAcc.isAvailable(v)) {
    std::cout << "tplinks ";
    for (const ElementLink< xAOD::TrackParticleContainer >& l : v.trackParticleLinks()) {
      std::cout << l.dataID() << "/" << l.index() << " ";
    }
    std::cout << "\n";
  }

  static const SG::ConstAccessor<float> trackWeightsAcc ("trackWeights");
  if (trackWeightsAcc.isAvailable(v)) {
    std::cout << "wt ";
    for (float f : v.trackWeights()) {
      std::cout << f << " ";
    }
    std::cout << "\n";
  }

  static const SG::ConstAccessor<std::vector<ElementLink<xAOD::NeutralParticleContainer> > > neutralParticleLinksAcc ("neutralParticleLinks");
  if (neutralParticleLinksAcc.isAvailable(v)) {
    std::cout << "nplinks ";
    for (const ElementLink< xAOD::NeutralParticleContainer >& l : v.neutralParticleLinks()) {
      std::cout << l.dataID() << "/" << l.index() << " ";
    }
    std::cout << "\n";
  }

  static const SG::ConstAccessor<float> neutralWeightsAcc ("neutralWeights");
  if (neutralWeightsAcc.isAvailable(v)) {
    std::cout << "wt ";
    for (float f : v.neutralWeights()) {
      std::cout << f << " ";
    }
    std::cout << "\n";
  }

  std::cout << v.vxTrackAtVertexAvailable() << "\n";
  for (const Trk::VxTrackAtVertex& vv : v.vxTrackAtVertex()) {
    vv.dump (std::cout);
    std::cout << "cov ";
    if (vv.perigeeAtVertex()) {
      dumpCovariance (*vv.perigeeAtVertex()->covariance());
    }
    else {
      std::cout << "(null)";
    }
    std::cout << "\n";
  }
}


void assertVec3D (const char* which,
                  const Amg::Vector3D& a,
                  const Amg::Vector3D& b,
                  double thresh = 2e-5)
{
  thresh = std::max (thresh, 2e-5);
  if ( ! Athena_test::isEqual (a.x(), b.x(), thresh) ||
       ! Athena_test::isEqual (a.y(), b.y(), thresh) ||
       ! Athena_test::isEqual (a.z(), b.z(), thresh) )
  {
    std::cerr << "TrkVKalVrtFitterTestAlg::assertVec3D mismatch " << which
              << ": ["
              << a.x() << ", "
              << a.y() << ", "
              << a.z() << "] / ["
              << b.x() << ", "
              << b.y() << ", "
              << b.z() << "]\n";
    std::abort();
  }
}


void comparePerigee (const Trk::TrackParameters* a,
                     const Trk::TrackParameters* b,
                     double thresh = 2e-5)
{
  if (!a && !b) return;
  if (!a || !b) std::abort();
  assertVec3D ("perigee pos", a->position(), b->position(), thresh);
  assertVec3D ("perigee mom", a->momentum(), b->momentum(), thresh);
  assert (a->charge() == b->charge());
  assertVec3D ("perigee surf",
               a->associatedSurface().center(),
               b->associatedSurface().center(), thresh);
}


void compareVertex (const xAOD::Vertex& a, const xAOD::Vertex& b,
                    double thresh = 2e-5)
{
  assertVec3D ("vertex pos", a.position(), b.position(), thresh);
  assert (Athena_test::isEqual (a.chiSquared(), b.chiSquared(),
                                std::max (5e-5, thresh)) );
  assert (Athena_test::isEqual (a.numberDoF(), b.numberDoF(), 1e-5) );
  assert (a.covariance().size() == b.covariance().size());
  for (unsigned int i = 0; i < a.covariance().size(); i++) {
    if (std::isinf(a.covariance()[i]) && std::isinf(b.covariance()[i])) continue;
    assert (Athena_test::isEqual (a.covariance()[i], b.covariance()[i], 2e-2) );
  }

  assert (a.vxTrackAtVertexAvailable() == b.vxTrackAtVertexAvailable());
  if (a.vxTrackAtVertexAvailable()) {
    const std::vector< Trk::VxTrackAtVertex >& avec = a.vxTrackAtVertex();
    const std::vector< Trk::VxTrackAtVertex >& bvec = b.vxTrackAtVertex();
    assert (avec.size() == bvec.size());
    for (unsigned int i = 0; i < avec.size(); i++) {
      comparePerigee (avec[i].initialPerigee(), bvec[i].initialPerigee(), thresh);
      comparePerigee (avec[i].perigeeAtVertex(), bvec[i].perigeeAtVertex(), thresh);
      assert (Athena_test::isEqual (avec[i].trackQuality().chiSquared(),
                                    bvec[i].trackQuality().chiSquared(),
                                    3e-2));
      assert (avec[i].trackQuality().numberDoF() ==
              bvec[i].trackQuality().numberDoF());
    }
  }
}


void setRefittedPerigee (xAOD::Vertex& v, unsigned i,
                         float charge,
                         const Amg::Vector3D& mom,
                         const std::vector<float>& c)
{
  std::vector< Trk::VxTrackAtVertex >& vec = v.vxTrackAtVertex();
  if (vec.size() <= i) vec.resize(i+1);

  AmgSymMatrix(5) cov = cov5();
  for (int ii=0; i < 5; ii++) {
    for (int j=0; j < 5; j++) {
      unsigned ipos = ii*5 + j;
      (cov)(ii,j) = ipos < c.size() ? c[ipos] : 0;
    }
  }

  const Amg::Vector3D& pos = v.position();
  auto p = std::make_unique<Trk::Perigee> (pos, mom, charge, pos,
                                           cov);
  vec[i].setPerigeeAtVertex (p.release());
}


void setFitQuality (xAOD::Vertex& v, unsigned i, float chi2, int ndof)
{
  std::vector< Trk::VxTrackAtVertex >& vec = v.vxTrackAtVertex();
  if (vec.size() <= i) vec.resize(i+1);
  vec[i].setTrackQuality (Trk::FitQuality (chi2, ndof));
}


} // anonymous namespace


namespace Trk {


/**
 * @brief Standard Gaudi initialize method.
 */
StatusCode TrkVKalVrtFitterTestAlg::initialize()
{
  ATH_CHECK( m_fitter.retrieve() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Standard Gaudi execute method.
 */
StatusCode TrkVKalVrtFitterTestAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_VERBOSE ("execute");

  ATH_CHECK( test1(ctx) );
  ATH_CHECK( test2(ctx) );
  ATH_CHECK( test3(ctx) );

  return StatusCode::SUCCESS;
}



// Neutral, no constraint.
// (Mixed charged+neutral seems not to work.)
StatusCode TrkVKalVrtFitterTestAlg::test1(const EventContext& ctx) const
{
  xAOD::Vertex exp_v0;
  exp_v0.makePrivateStore();
  exp_v0.setPosition ({-5.58116, -8.50767, -4.76804});
  exp_v0.setFitQuality (0.541406, 3);
  exp_v0.setCovariance(std::vector<float>{
      259.128,     394.824,     764.131,    144.775,     266.345,  155.231,
      -30152,      -6184.95,    -2642.5,    4.60831e+13, 10912.8,  -4760.06,
      6318.67,     1.0494e+14,  2.4226e+14, -12079,      -23269.1, -40859.3,
      2.00894e+13, 4.51754e+13, 9.12659e+12});
  setFitQuality(exp_v0, 0, 0.311, 2);
  setFitQuality (exp_v0, 1, 0.138, 2);
  setFitQuality (exp_v0, 2, 0.082, 2);

  NeutralUVec_t neutrals = makeNeutrals1();
  std::unique_ptr<xAOD::Vertex> v1 (m_fitter->fit (ctx, std::vector<const Trk::TrackParameters*>(),
                                                   asVec (neutrals)));
  compareVertex (*v1, exp_v0);

  return StatusCode::SUCCESS;
}



// Neutral + Vector3D constraint
StatusCode TrkVKalVrtFitterTestAlg::test2(const EventContext& ctx) const
{
  xAOD::Vertex exp_v0;
  exp_v0.makePrivateStore();
  exp_v0.setPosition ({ 24.131, 38.8186, 13.2978});
  exp_v0.setFitQuality (0.564942, 3);
  exp_v0.setCovariance(std::vector<float>{
      21964.5,     32891.5,     51244.9,     13612.1,     20963.9,
      9573.41,     816688,      1.09315e+06, 393680,      1.92433e+14,
      1.10804e+06, 1.87392e+06, 616429,      3.20119e+14, 5.41674e+14,
      401502,      640572,      338398,      1.97024e+14, 3.42617e+14,
      2.25906e+14});
  setFitQuality(exp_v0, 0, 0.306, 2);
  setFitQuality(exp_v0, 1, 0.00660789, 2);
  setFitQuality(exp_v0, 2, 0.266975, 2);

  Amg::Vector3D pnt1(5, 6, -3);

  NeutralUVec_t neutrals = makeNeutrals1();
  std::unique_ptr<xAOD::Vertex> v1 (m_fitter->fit (ctx, std::vector<const Trk::TrackParameters*>(),
                                                   asVec (neutrals),
                                                   pnt1));
  compareVertex (*v1, exp_v0);

  // ??? This gives a different result due to different reference frame
  //     handling in CvtNeutralParameters vs CvtNeutralParticle
  xAOD::Vertex exp_v1;
  exp_v1.makePrivateStore();
  exp_v1.setPosition ({27.7411, 46.5526, 14.3721});
  exp_v1.setFitQuality (0.537727, 3);
  exp_v1.setCovariance(std::vector<float>{
      32598.3,     50372.7,     80773.1,    18537.3,     29396.9,
      12058.1,     850485,      1.1479e+06, 380180,      1.54295e+14,
      1.21881e+06, 2.11418e+06, 643860,     2.55981e+14, 4.31589e+14,
      417309,      683027,      356607,     1.51101e+14, 2.64837e+14,
      1.77033e+14});
  setFitQuality(exp_v1, 0, 0.281, 2);
  setFitQuality(exp_v1, 1, 0.010, 2);
  setFitQuality(exp_v1, 2, 0.248, 2);

  xAODNPUVec_t xaodnp = makexAODNP(makeNeutrals1());
  std::unique_ptr<xAOD::Vertex> v2 (m_fitter->fit (ctx, std::vector<const xAOD::TrackParticle*>(),
                                                   asVec (xaodnp),
                                                   pnt1));
  compareVertex (*v2, exp_v1);

  return StatusCode::SUCCESS;
}


// Neutral + Vertex constraint
StatusCode TrkVKalVrtFitterTestAlg::test3(const EventContext& ctx) const
{
  xAOD::Vertex exp_v0;
  exp_v0.makePrivateStore();
  exp_v0.setPosition ({5.05085, 6.19647, -2.81445});
  exp_v0.setFitQuality (2.20220, 6);
  exp_v0.setCovariance (std::vector<float>
                        { 0.955506, 0.0266418, 0.95407, -0.0110041,
                            -0.0101241, 0.953395, 530.063, -411.644,
                            -325.151, 3.03883e+14, -711.821, 1110.92,
                            -551.227, 2.09286e+14, 2.49946e+14, 123.813,
                            386.78, 892.698, -3.14264e+12, 1.04832e+14,
                          1.10053e+14 });
  setFitQuality (exp_v0, 0, 0.396, 2);
  setFitQuality (exp_v0, 1, 0.752, 2);
  setFitQuality (exp_v0, 2, 1.055, 2);

  Amg::Vector3D pnt1 (5, 6, -3);
  xAOD::Vertex pnt2;
  pnt2.makePrivateStore();
  pnt2.setPosition (pnt1);
  AmgSymMatrix(3) pnt2covar;
  pnt2covar.setIdentity();
  pnt2.setCovariancePosition (pnt2covar);

  NeutralUVec_t neutrals = makeNeutrals1();
  std::unique_ptr<xAOD::Vertex> v1 (m_fitter->fit (ctx, std::vector<const Trk::TrackParameters*>(),
                                                   asVec (neutrals),
                                                   pnt2));
  compareVertex (*v1, exp_v0);

  // ??? This gives a different result due to different reference frame
  //     handling in CvtNeutralParameters vs CvtNeutralParticle
  xAOD::Vertex exp_v1;
  exp_v1.makePrivateStore();
  exp_v1.setPosition ({5.00912, 6.23591, -2.82707});
  exp_v1.setFitQuality (2.25019, 6);
  exp_v1.setCovariance (std::vector<float>
                        { 0.968686, 0.0228402, 0.951705, -0.00910123,
                            -0.0098848, 0.950267, 435.6, -418.438,
                            -276.022, 3.17041e+14, -401.428, 987.01,
                            -336.323, 1.592e+14, 1.35435e+14, 183.46,
                            272.759, 988.474, -1.22294e+13, 5.57451e+13,
                           7.21632e+13 });
  setFitQuality (exp_v1, 0, 0.313, 2);
  setFitQuality (exp_v1, 1, 0.692, 2);
  setFitQuality (exp_v1, 2, 1.246, 2);

  xAODNPUVec_t xaodnp = makexAODNP (makeNeutrals1());
  std::unique_ptr<xAOD::Vertex> v2 (m_fitter->fit (ctx, std::vector<const xAOD::TrackParticle*>(),
                                                   asVec (xaodnp),
                                                   pnt2));
  compareVertex (*v2, exp_v1);

  return StatusCode::SUCCESS;
}


} // namespace Trk



namespace {


AmgSymMatrix(5) cov5a()
{
  AmgSymMatrix(5) m;
  m.setZero();
  for (int i=0; i < 5; i++) {
    (m)(i,i) = 1e-2;
  }
  (m)(1,1)=1;
  return m;
}


using PerigeeUVec_t = std::vector<std::unique_ptr<Trk::Perigee> >;
PerigeeUVec_t makePerigees2()
{
  Amg::Vector3D pos1a { 10*mm,   0*mm, -5*mm };
  Amg::Vector3D mom1a { 1000*MeV, 0*MeV, 0*MeV };
  Amg::Vector3D pos1b { 10.5*mm, 0.5*mm, -5.5*mm };
  Amg::Vector3D mom1b { 800*MeV, 200*MeV, 200*MeV };
  Amg::Vector3D pos1c { 9.5*mm, -0.5*mm, -4.5*mm };
  Amg::Vector3D mom1c { 700*MeV, -300*MeV, -200*MeV };

  PerigeeUVec_t ret;

  ret.emplace_back (std::make_unique<Trk::Perigee>(pos1a, mom1a,  1, pos1a, cov5a()));
  ret.emplace_back (std::make_unique<Trk::Perigee>(pos1b, mom1b, -1, pos1a, cov5a()));
  ret.emplace_back (std::make_unique<Trk::Perigee>(pos1c, mom1c, -1, pos1a, cov5a()));

  return ret;
}


PerigeeUVec_t makePerigees3()
{
  Amg::Vector3D pos1a { 5*mm,   0*mm, -2.5*mm };
  Amg::Vector3D mom1a { 600*MeV, 400*MeV, 200*MeV };
  Amg::Vector3D pos1b { 5.1*mm, 0.3*mm, -2.6*mm };
  Amg::Vector3D mom1b { 700*MeV, -300*MeV, -150*MeV };

  PerigeeUVec_t ret;

  ret.emplace_back (std::make_unique<Trk::Perigee>(pos1a, mom1a,  1, pos1a, cov5a()));
  ret.emplace_back (std::make_unique<Trk::Perigee>(pos1b, mom1b, -1, pos1a, cov5a()));

  return ret;
}


Amg::Vector3D exp_mom (const double moms[][4], int imom)
{
  return {moms[imom][0], moms[imom][1], moms[imom][2]};
}


} // anonymous namespace


namespace Trk {


// Simple cascade fitter test.
StatusCode TrkVKalVrtFitterTestAlg::test4(const EventContext& ctx) const
{
  const Trk::IVertexCascadeFitter* fitter =
    dynamic_cast<const Trk::IVertexCascadeFitter*> (m_fitter.get());
    if (!fitter)[[unlikely]]{
      ATH_MSG_ERROR("TrkVKalVrtFitterTestAlg::test4: 'fitter' is nullptr");
      return StatusCode::FAILURE;
    }
  xAODTPUVec_t tracks1 = makexAODTP (makePerigees2());

  std::unique_ptr<IVKalState> state (fitter->makeState(ctx));
  Trk::VertexID v1 = fitter->startVertex (asVec (tracks1),
                                          std::vector<double> {100*MeV, 150*MeV, 200*MeV},
                                          *state,
                                          930*MeV);

  xAODTPUVec_t tracks2 = makexAODTP (makePerigees3());

  fitter->nextVertex (asVec (tracks2),
                      std::vector<double> {130*MeV, 160*MeV},
                      std::vector<Trk::VertexID> {v1},
                      *state,
                      2000*MeV);

  std::unique_ptr<Trk::VxCascadeInfo> info1 (fitter->fitCascade(*state));
  info1->setSVOwnership (true);

#if 0
  std::cout << info1->fitChi2() << " " << info1->nDoF() << "\n";
  for (const std::vector<TLorentzVector>& vv : info1->getParticleMoms()) {
    std::cout << "===\n";
    for (const TLorentzVector& vvv : vv) {
      std::cout << vvv.X() << " " << vvv.Y() << " " << vvv.Z() << " " << vvv.E() << "\n";
    }
  }
  std::cout << "=== vertices\n";
  for (const xAOD::Vertex* v : info1->vertices()) {
    dumpVertex (*v);
  }
#endif

  // Some of the comparison thresholds here have to be quite large,
  // because the fit is unstable against small changes in the rounding
  // of, eg, trig functions.  The results we get can vary depending
  // on the libc version used as well as on the exact hardware used.

  assert (Athena_test::isEqual (info1->fitChi2(), 8.69008, 1e-5));
  assert (info1->nDoF() == 8);


  const double exp_moms0[][4] =
    {
     {65.8357, -2.03326, -1.46405, 119.752},
     {755.228,  239.515,  134.648, 817.537},
     {900.997, -348.825, -292.857, 1029.19},
     {1013.68,  681.319,  331.188, 1272.13},
     {522.571, -222.398, -113.273, 600.81},
     {1719.34, -112.854, -155.908, 1964.27},
    };
  const size_t nmoms0 = std::distance (std::begin(exp_moms0), std::end(exp_moms0));
  size_t imoms0 = 0;
  for (const std::vector<TLorentzVector>& vv : info1->getParticleMoms()) {
    for (const TLorentzVector& vvv : vv) {
      assert (imoms0 < nmoms0);
      assert( Athena_test::isEqual (vvv.X(), exp_moms0[imoms0][0], 0.1) );
      assert( Athena_test::isEqual (vvv.Y(), exp_moms0[imoms0][1], 0.1) );
      assert( Athena_test::isEqual (vvv.Z(), exp_moms0[imoms0][2], 0.1) );
      assert( Athena_test::isEqual (vvv.E(), exp_moms0[imoms0][3], 0.1) );
      ++imoms0;
    }
  }
  assert (imoms0 == nmoms0);

  assert (info1->vertices().size() == 2);

  xAOD::Vertex exp_v0;
  exp_v0.makePrivateStore();
  exp_v0.setPosition({ 7.89827, 0.0514449, -4.04121 });
  exp_v0.setFitQuality(5.34877, 8);
  exp_v0.setCovariance(std::vector<float>{
    0.218298, -0.00769266, 0.0194589, -0.0118989, 0.0107223, 0.208922 });
  setRefittedPerigee(
    exp_v0,
    0,
    1,
    exp_mom(exp_moms0, 0),
    { 0.209404,    -4.58753,  -0.00519457,  0.00377293,   0.00105397,
      -4.58753,    154483,    32.603,       -0.117209,    -35.6007,
      -0.00519457, 32.603,    0.0113951,    -0.000116443, -0.00751125,
      0.00377293,  -0.117209, -0.000116443, 0.009643,     2.37152e-05,
      0.00105397,  -35.6007,  -0.00751125,  2.37152e-05,  0.0082042 });
  setFitQuality(exp_v0, 0, 0.926, 2);
  setRefittedPerigee(
    exp_v0,
    1,
    -1,
    exp_mom(exp_moms0, 1),
    { 0.185428,     0.807536,  -0.00324979,  -0.000237823, 1.99968e-05,
      0.807536,     154490,    6.26553,      -0.238515,    0.168369,
      -0.00324979,  6.26553,   0.00856011,   -0.00036866,  -2.23097e-05,
      -0.000237823, -0.238515, -0.00036866,  0.00933191,   7.51824e-06,
      1.99968e-05,  0.168369,  -2.23097e-05, 7.51824e-06,  3.74483e-07 });
  setFitQuality(exp_v0, 1, 4.142, 2);
  setRefittedPerigee(
    exp_v0,
    2,
    -1,
    exp_mom(exp_moms0, 2),
    { 0.191446,     -9.49834,  -0.00495436,  -0.00130953,  -4.72358e-05,
      -9.49834,     154476,    5.42258,      -0.231539,    0.236084,
      -0.00495436,  5.42258,   0.00860893,   -0.000425575, 3.61158e-05,
      -0.00130953,  -0.231539, -0.000425575, 0.00920703,   -5.93187e-06,
      -4.72358e-05, 0.236084,  3.61158e-05,  -5.93187e-06, 4.8944e-07 });
  setFitQuality(exp_v0, 2, 0.281, 2);

  compareVertex(*info1->vertices()[0], exp_v0, 0.1);

  xAOD::Vertex exp_v1;
  exp_v1.makePrivateStore();
  exp_v1.setPosition({ 5.31046, 0.22012, -3.80093 });
  exp_v1.setFitQuality(3.34131, 8);
  exp_v1.setCovariance(std::vector<float>{
    0.0239352, 0.000977903, 0.00708136, 4.16975e-05, 0.00295894, 0.2431 });
  setRefittedPerigee(
    exp_v1,
    0,
    1,
    exp_mom(exp_moms0, 3),
    { 0.166917,     -16.9685,    -0.000309914, -0.000819924, 1.31935e-05,
      -16.9685,     6.35682e+08, -139.016,     9.70413,      -495.474,
      -0.000309914, -139.016,    0.0100214,    -7.71225e-06, 0.000103046,
      -0.000819924, 9.70413,     -7.71225e-06, 0.00998731,   -5.42066e-06,
      1.31935e-05,  -495.474,    0.000103046,  -5.42066e-06, 0.000386195 });
  setFitQuality(exp_v1, 0, 1.986, 2);
  setRefittedPerigee(
    exp_v1,
    1,
    -1,
    exp_mom(exp_moms0, 4),
    { 0.20904,      -15.3143,    0.00086181,  -0.000463146, -2.83922e-05,
      -15.3143,     6.35682e+08, 306.365,     -3.09595,     -2470.81,
      0.00086181,   306.365,     0.0101467,   -2.3178e-06,  -0.00116433,
      -0.000463146, -3.09595,    -2.3178e-06, 0.00999678,   8.90989e-07,
      -2.83922e-05, -2470.81,    -0.00116433, 8.90989e-07,  0.00960596 });
  setFitQuality(exp_v1, 1, 1.356, 2);

  compareVertex (*info1->vertices()[1], exp_v1, 0.1);

  return StatusCode::SUCCESS;
}


} // namespace Trk
