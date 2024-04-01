/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOUTILS_SURFACEBOUNDSET_H
#define ACTSGEOUTILS_SURFACEBOUNDSET_H

#include "ActsGeoUtils/Defs.h"
#ifndef SIMULATIONBASE
#include "Acts/Surfaces/SurfaceBounds.hpp"

namespace ActsTrk {
    /*  If multiple surfaces in the geometry share the same dimensions, the SurfaceBoundSet provides a convenient way
     *  to create only one surface bound object in memory and parse it to the individual Surfaces. 
    */
   
    template <class BoundType> class SurfaceBoundSet {
        public:
            SurfaceBoundSet() = default;
            /// Factory method to create new SurfaceBounds.
            template<class... argList> std::shared_ptr<BoundType> make_bounds(argList... args) {
                return (*m_store.insert(std::make_shared<BoundType>(args...)).first);
            }
            size_t size() const { return m_store.size() ;}       
        private: 
            /** @brief: Comparison struct to construct sets of Acts::Surface bounds with unique elements. 
             *          Two elements are considered to be identical if all of their parameters match within epsilon. 
             *          Combined with the native insert mechanism of the set, one can easily propagate the same
             *          bound object to multiple surfaces.
            **/     
            struct BoundComparer {            
                    bool operator()(const std::shared_ptr<BoundType>& a, 
                                    const std::shared_ptr<BoundType>& b) const {
                        if (a->type() != b->type()) {
                            return static_cast<int>(a->type()) < static_cast<int>(b->type());
                        }
                        const std::vector<double> avalues{a->values()};
                        const std::vector<double> bvalues{b->values()};
                        std::size_t size = avalues.size();
                        for(std::size_t i=0; i<size-1; ++i) {
                            if(std::abs(avalues[i]- bvalues[i]) > std::numeric_limits<double>::epsilon()){
                                return avalues[i] < bvalues[i];
                            }
                        }
                        return avalues[size-1] < bvalues[size-1];
                    }
            };
            std::set<std::shared_ptr<BoundType>, BoundComparer> m_store{};
    };
    /// Aberivation to create a new SurfaceBoundSetPtr
    template<class BoundType> using SurfaceBoundSetPtr = std::shared_ptr<SurfaceBoundSet<BoundType>>;
}
#endif
#endif