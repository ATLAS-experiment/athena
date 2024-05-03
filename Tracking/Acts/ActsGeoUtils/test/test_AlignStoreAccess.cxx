/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

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

#include <thread>
#include <future>
#include <chrono>
#include <random>

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

template<> Amg::Transform3D 
    ActsTrk::TransformCacheDetEle<TestDetElement>::fetchTransform(const ActsTrk::DetectorAlignStore* store) const {
    return m_parent->getMaterialGeom()->getAbsoluteTransform(store->geoModelAlignment.get()) * Amg::getRotateX3D(M_PI);
}

/** Returns how many threads have not yet finished their work*/
template <class T> size_t count_active(const std::vector<std::future<T>>& threads) {
    size_t counts{0};
    for (const std::future<T>& thread : threads) {
         using namespace std::chrono_literals;
        if (thread.wait_for(0ms) != std::future_status::ready) ++counts;
    }
    return counts;
}


class WorkerTask {
    public:
        WorkerTask(const std::vector<std::shared_ptr<TestDetElement>>& detElements,
                   std::shared_ptr<GeoAlignmentStore> condAlignment):
            m_detEles{detElements} {
            // m_store->geoModelAlignment = condAlignment;
            m_store->geoModelAlignment = std::make_unique<GeoAlignmentStore>(*condAlignment);
            m_store->geoModelAlignment->clearPosCache();
        }
        bool execute() {
            ActsGeometryContext gctx{};
            gctx.setStore(m_store);
            Amg::Transform3D combinedTrf{Amg::Transform3D::Identity()};
            for (unsigned int daemon = 0 ; daemon < 666; ++daemon) {
                for (const auto& det : m_detEles) {
                    const Amg::Transform3D& trf{det->transform(gctx.context())};
                    // assert(trf.data()[0] != 624626.56);

                    combinedTrf = trf * combinedTrf;
                    Amg::Vector3D vec = trf.inverse() * Amg::Vector3D{51515,6262,7272};
                    
                    if (vec.dot(vec) < 0. || std::pow(vec.eta() * vec.theta(), 2) < 0.) {
                            std::cout<<"Confusion "<<std::endl;
                    }
                }           
            }
            m_executed = true;
            return true;
        }
        bool executed() const { return m_executed; }
    private:
        std::vector<std::shared_ptr<TestDetElement>> m_detEles{};
        std::shared_ptr<ActsTrk::DetectorAlignStore> m_store{std::make_unique<ActsTrk::DetectorAlignStore>(ActsTrk::DetectorType::Csc)};
        bool m_executed{false};

};

class WorkerNode {
    public:
        WorkerNode() = default;

        bool hasTask() const {
            std::shared_lock guard{m_mutex};
            return m_hasTask;
        }
        void assignTask(std::unique_ptr<WorkerTask> task) {        
            if (hasTask()) return;
            std::unique_lock guard {m_mutex};
            m_task = std::move(task);
            m_hasTask = true;
        }
        bool executeTask() {
            while (!m_abort) {                
                if (m_task) {
                    m_task->execute();
                    std::unique_lock guard{m_mutex};
                    m_task.reset();
                    m_hasTask = false;
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            return true;
        }
        void finalize() {
            m_abort = true; 
        }
    private:
        mutable std::shared_mutex m_mutex{};
        bool m_hasTask{false};
        std::atomic<bool> m_abort{false};
        std::unique_ptr<WorkerTask> m_task{};
};


int main() {
    
    unsigned int nThreads = std::min(8u, std::thread::hardware_concurrency());
    constexpr unsigned int numTrials = 40;

    /* Define the world and place randomly the volumes */
    PVLink world{createGeoWorld()};
    
    /** Define the GeoAlignmentStore to move the volumes coherently */
    std::shared_ptr<GeoAlignmentStore> condAlignment = std::make_shared<GeoAlignmentStore>();
   
    std::vector<std::shared_ptr<TestDetElement>> detElements{};
    
    for (unsigned int k =0 ; k < 25; ++k) {
        GeoIntrusivePtr<GeoAlignableTransform> alignTrf = make_intrusive<GeoAlignableTransform>(Amg::getTranslateX3D(k+1));
        condAlignment->setDelta(alignTrf, Amg::getTranslateY3D(k+1) * Amg::getRotateX3D(M_PI_2));
        world->add(alignTrf);
        
        GeoIntrusivePtr<GeoPhysVol> alignBox = make_intrusive<GeoPhysVol>(world->getLogVol());
        world->add(alignBox);
        for (unsigned int d = 0 ; d < 66; ++d) {
            world->add(make_intrusive<GeoTransform>(Amg::getTranslateZ3D(d+6)));
            GeoIntrusivePtr<GeoFullPhysVol> detVol{make_intrusive<GeoFullPhysVol>(world->getLogVol())};
            world->add(detVol);
            detElements.emplace_back(std::make_unique<TestDetElement>(detVol));
        }
    }
    condAlignment->lockDelta();

    std::vector<std::unique_ptr<WorkerNode>> workers{};
    std::vector<std::future<bool>> threads{};
    
    for (unsigned int node = 0 ; node < nThreads; ++node) {
        workers.emplace_back(std::make_unique<WorkerNode>());
        WorkerNode* testerPtr = workers.back().get();
        threads.emplace_back(std::async(std::launch::async,[testerPtr](){return testerPtr->executeTask();}));

    }
    
    std::random_device rd;
    std::mt19937 g(rd());

    unsigned int executedAttempts{};
    while (executedAttempts < numTrials) {
        for (auto& worker : workers) {
            if (!worker->hasTask()) {
                worker->assignTask(std::make_unique<WorkerTask>(detElements, condAlignment));
                std::cout<<"Launch new worker "<<executedAttempts<<" "<<std::endl;
                if (executedAttempts % 100 == 0) std::shuffle(detElements.begin(),detElements.end(), g);
                ++executedAttempts;
            }
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    }
    for (auto & worker : workers) {
        worker->finalize();
    }
    while (count_active(threads) > 0){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return EXIT_SUCCESS;
}

