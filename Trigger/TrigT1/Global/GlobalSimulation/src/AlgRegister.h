//  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef GLOBALSIM_ALGREGISTER_H
#define GLOBALSIM_ALGREGISTER_H

#include "IGSAlg.h"
#include <memory>
#include <map>

/*
 * AlgRegister - a thread safe register for creator functions for
 * IGSAlg instances
 *
 * The Meyers Singleton approach is used.
 *
 * The approach, and others, are discussed in
 *
 * S Meyers: Effective C++
 * A Alexandrescu: Modern C++ design
 * Wikipedia: https://en.wikipedia.org/wiki/Double-checked_locking
 * Rainer Grimm reports timings for various approaches at
 * https://www.modernescpp.com/index.php/thread-safe-initialization-of-a-singleton/
 * 
 * 
 */

namespace GlobalSim {

  class AlgRegister {
  public:
       
    using CreatorFn = std::unique_ptr<IGSAlg> ();
    
    AlgRegister& operator=(const AlgRegister&) = delete;
    
    static AlgRegister& instance(){
      static AlgRegister reg;
      return reg;
    }
    
    // Retrieve an Alg instance from the Register,
    // which is seen though the IGSAlg interface. Note that
    // successive calls will produce different instances
    std::unique_ptr<IGSAlg> get(const std::string& label) const;

    // allow concrete classes to insert a creator function.
    bool insert(const std::string label, CreatorFn* f);
    
  private:
    
    AlgRegister () =default;
    AlgRegister(const AlgRegister&) = default;

    std::map<std::string, CreatorFn*> m_ledger;
  };

}
#endif
