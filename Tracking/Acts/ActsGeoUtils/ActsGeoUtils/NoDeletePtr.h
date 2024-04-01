
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsGeoUtils_NODELETEPTR_H
#define ActsGeoUtils_NODELETEPTR_H

#include <memory>

/** 
  * @brief: The NoDeletePtr is a shared_ptr with disabled deleter. 
  * 
*/
namespace ActsTrk {

    template<class Obj>
    class NoDeletePtr: public std::shared_ptr<Obj> {
        public:
            struct Deleter {
                void operator()(Obj*) const{}
            };
            NoDeletePtr(Obj* ptr):
                std::shared_ptr<Obj>(ptr, Deleter{}){
            }
    };
}
#endif