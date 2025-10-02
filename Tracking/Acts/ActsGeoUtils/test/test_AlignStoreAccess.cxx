/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include "ActsGeometryInterfaces/DetectorAlignStore.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/IDetectorElement.h"

#include "ActsGeoUtils/TransformCache.h"
#include <stdlib.h>

#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoVDetectorElement.h"
#include "GeoModelHelpers/defineWorld.h"
#include "GeoModelHelpers/ThreadPool.h"

#include <thread>
#include <future>
#include <chrono>
#include <random>
#include <unordered_map>


class TestDetElement : public ActsTrk::IDetectorElement, public GeoVDetectorElement{
    public:
        TestDetElement(GeoIntrusivePtr<GeoVFullPhysVol> detVol):
            GeoVDetectorElement(detVol) { }

        Identifier identify() const override final { 
            return Identifier{};
        }
        ActsTrk::DetectorType detectorType() const  override final { 
            return ActsTrk::DetectorType::Csc; 
        }        
        unsigned int storeAlignedTransforms(const ActsTrk::DetectorAlignStore& store) const override final {
            m_cache.getTransform(&store);
            return 1;
        }
        const Amg::Transform3D& transform(const Acts::GeometryContext& gctx) const override final {
            return m_cache.transform(gctx);
        }
        const Acts::Surface& surface() const override final  {
            static const std::shared_ptr<Acts::Surface> surf{};
            return *surf;
        }
        Acts::Surface& surface() override final {
            static const std::shared_ptr<Acts::Surface> surf{};
            return *surf;
        }
        double thickness() const override final { return 0.;}

    private:
        ActsTrk::TransformCacheDetEle<TestDetElement> m_cache{IdentifierHash{1}, this};
};

using ExpectationMap_t = std::unordered_map<const TestDetElement*, Amg::Transform3D>;

template<> Amg::Transform3D 
    ActsTrk::TransformCacheDetEle<TestDetElement>::fetchTransform(const ActsTrk::DetectorAlignStore* store) const {
    return m_parent->getMaterialGeom()->getAbsoluteTransform(store->geoModelAlignment.get()) * Amg::getRotateX3D(M_PI);
}

std::unique_ptr<ActsTrk::DetectorAlignStore> makeAlignedStore(const std::shared_ptr<GeoAlignmentStore>& condAlign) {
    auto store = std::make_unique<ActsTrk::DetectorAlignStore>(ActsTrk::DetectorType::Csc);
    store->geoModelAlignment = std::make_unique<GeoAlignmentStore>(*condAlign);
    store->geoModelAlignment->clearPosCache();
    return store;

}


class WorkerTask : public GeoThreading::ThreadPool::IThreadTask {
    public:
        WorkerTask(const std::vector<std::shared_ptr<TestDetElement>>& detElements,
                   const std::shared_ptr<GeoAlignmentStore>& condAlignment,
                   const ExpectationMap_t& trfMap):
            m_detEles{detElements},
            m_store{makeAlignedStore(condAlignment)},
            m_trfMap{trfMap} {}
#if defined(FLATTEN) && defined(__GNUC__)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
[[gnu::flatten]]
#endif
        void execute() override final {

            std::random_device rd;
            std::mt19937 g(rd());
            std::ranges::shuffle(m_detEles, g);

            ActsGeometryContext gctx{};
            gctx.setStore(m_store);
            
            for (const auto& det : m_detEles) {
                const auto find_itr = m_trfMap.find(det.get());
                if (find_itr == m_trfMap.end()) {
                    THROW_EXCEPTION("Detector element not in reference transform map");
                }
                const Amg::Transform3D& trf{det->transform(gctx.context())};
                if (!Amg::isIdentity(trf.inverse() * find_itr->second)){
                    THROW_EXCEPTION("Different alignment detected "<<std::endl
                        <<" ***    found: "<<Amg::toString(trf)<<std::endl
                        <<" *** expected: "<<Amg::toString(find_itr->second));
                }
            }
        }
        bool ready() const override final { return true; }
    private:
        std::vector<std::shared_ptr<TestDetElement>> m_detEles{};
        std::shared_ptr<ActsTrk::DetectorAlignStore> m_store{};
        const ExpectationMap_t& m_trfMap;
};
int main() {
    
    
    auto& pool = GeoThreading::ThreadPool::getPool(-1);
    constexpr unsigned int numTrials = 666;

    constexpr unsigned nAlign = 250;
    constexpr unsigned nDetPerAlign = 55;
    /* Define the world and place randomly the volumes */
    PVLink world{createGeoWorld()};
    
    /** Define the GeoAlignmentStore to move the volumes coherently */
    auto condAlignment = std::make_shared<GeoAlignmentStore>();
   
    std::vector<std::shared_ptr<TestDetElement>> detElements{};

    for (unsigned int k =0 ; k < nAlign; ++k) {
        GeoIntrusivePtr<GeoAlignableTransform> alignTrf = make_intrusive<GeoAlignableTransform>(Amg::getTranslateX3D(k+1));
        condAlignment->setDelta(alignTrf, Amg::getTranslateY3D(k+1) * Amg::getRotateX3D(M_PI_2));
        world->add(alignTrf);
        
        GeoIntrusivePtr<GeoPhysVol> alignBox = make_intrusive<GeoPhysVol>(world->getLogVol());
        world->add(alignBox);
        for (unsigned int d = 0 ; d < nDetPerAlign; ++d) {
            alignBox->add(make_intrusive<GeoTransform>(Amg::getTranslateZ3D(d+6)));
            GeoIntrusivePtr<GeoFullPhysVol> detVol{make_intrusive<GeoFullPhysVol>(world->getLogVol())};
            alignBox->add(detVol);
            detElements.emplace_back(std::make_unique<TestDetElement>(detVol));
        }
    }

    condAlignment->lockDelta();
    /// First check the positions of the detectors
    if (detElements.size() != nDetPerAlign * nAlign) {
        std::cerr<<"Not enough detector elements were constructed "<<detElements.size()<<" vs. "<<(nDetPerAlign * nAlign )<<std::endl;
        return EXIT_FAILURE;
    }
    
    ExpectationMap_t unalignedTrfs{}, alignedTrfs{};
    {
        ActsGeometryContext uGctx{};
        ActsGeometryContext aGctx{};
        uGctx.setStore(std::make_unique<ActsTrk::DetectorAlignStore>(ActsTrk::DetectorType::Csc));
        aGctx.setStore(makeAlignedStore(condAlignment));
        for (unsigned int k =0 ; k < nAlign ; ++k) {
            const Amg::Transform3D baseTrf{Amg::getTranslateX3D(k+1)};
            const Amg::Transform3D baseAlTrf{baseTrf * Amg::getTranslateY3D(k+1) * Amg::getRotateX3D(M_PI_2)};

            for (unsigned int d = 0; d< nDetPerAlign; ++d) {
                const Amg::Transform3D uExpTrf = baseTrf * 
                                                 Amg::getTranslateZ3D(d+6)* 
                                                 Amg::getRotateX3D(M_PI);
                const auto* detEle = detElements[k*nDetPerAlign + d].get();
                const Amg::Transform3D& uDetTrf{detEle->transform(uGctx.context())};
                if (!Amg::isIdentity(uExpTrf *  detEle->transform(uGctx.context()).inverse())){
                    std::cerr<<"Detector element is not where it's expected: "<<std::endl
                             <<" ** expect: "<<Amg::toString(uExpTrf)<<std::endl
                             <<" **  found: "<<Amg::toString(uDetTrf)<<std::endl;
                    return EXIT_FAILURE;
                }
                unalignedTrfs.insert(std::make_pair(detEle, uExpTrf));
                const Amg::Transform3D aExpTrf = baseAlTrf * 
                                                 Amg::getTranslateZ3D(d+6)* 
                                                 Amg::getRotateX3D(M_PI);
                const Amg::Transform3D& aDetTrf{detEle->transform(aGctx.context())};
                if (!Amg::isIdentity(aExpTrf * detEle->transform(aGctx.context()).inverse())){
                    std::cerr<<"Aligned detector  element is not where it's expected: "<<std::endl
                             <<" ** expect: "<<Amg::toString(aExpTrf)<<std::endl
                             <<" **  found: "<<Amg::toString(aDetTrf)<<std::endl;
                    return EXIT_FAILURE;
                }
                alignedTrfs.insert(std::make_pair(detEle, aExpTrf));

            }
        }
    }
        std::cout<<"Detector element positioning test passed. "<<std::endl;
    if (detElements.size() != alignedTrfs.size()) {
        std::cerr<<"Aligned expectation map is too small detEle: "<<detElements.size()<<" vs. map: "<<alignedTrfs.size()<<std::endl;
        return EXIT_FAILURE;
    }
    if (detElements.size() != unalignedTrfs.size()) {
        std::cerr<<"Nominal expectation map is too small detEle: "<<detElements.size()<<" vs. map: "<<unalignedTrfs.size()<<std::endl;
        return EXIT_FAILURE;
    }


    unsigned int executedAttempts{0};
    while (executedAttempts < numTrials) {
        
        if(numTrials % 3 == 0) {
            pool.appendTask(std::make_unique<WorkerTask>(detElements, std::make_shared<GeoAlignmentStore>(), unalignedTrfs));
        } else{
            pool.appendTask(std::make_unique<WorkerTask>(detElements, condAlignment, alignedTrfs));
        }
        ++executedAttempts;
    }
    pool.drainQueue();
    return EXIT_SUCCESS;
}

