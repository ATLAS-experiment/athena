// -*- C++ -*-
#ifndef Herwig_bb4lFullShowerVeto_H
#define Herwig_bb4lFullShowerVeto_H
//
// This is the declaration of the bb4lFullShowerVeto class.
// Copyright (C) 2017 Silvia Ferrario Ravasio, Tomas Jezo
// inspired by Contrib/ShowerVeto/NonBShowerVeto.h file
//

#include "Herwig/API/HerwigUI.h"

#include "Herwig/Shower/QTilde/Base/FullShowerVeto.h"
#include "herwig7_interface.h"
#include "CxxUtils/checker_macros.h"

namespace Herwig {

using namespace ThePEG;

/**
 * The bb4lFullShowerVeto class vetos parton showers where no b (anti)quarks are produced.
 *
 * @see \ref bb4lFullShowerVetoInterfaces "The interfaces"
 * defined for bb4lFullShowerVeto.
 */
class ATLAS_NOT_THREAD_SAFE bb4lFullShowerVeto: public FullShowerVeto {

public:
  /**
   * The default constructor.
   */
  bb4lFullShowerVeto () {
    if(powheginput_("#herwigveto",11) <= 0) {
      std::cout << "bb4lShowerVeto: You want me to veto, but herwigveto is not set to 1 in powheg.input! Exiting ..."<<std::endl;
      exit(-1);
    }
  }

protected:

  /**
   *  Determine whether to not to veto the shower, to be implemented in inheriting classes
   */
  virtual bool vetoShower ();


public:

  /**
   * The standard Init function used to initialize the interfaces.
   * Called exactly once for each class by the class description system
   * before the main function starts or
   * when this class is dynamically loaded.
   */
  static void Init();

protected:

  /** @name Clone Methods. */
  //@{
  /**
   * Make a simple clone of this object.
   * @return a pointer to the new object.
   */
  virtual IBPtr clone() const;

  /** Make a clone of this object, possibly modifying the cloned object
   * to make it sane.
   * @return a pointer to the new object.
   */
  virtual IBPtr fullclone() const;
  //@}

private:

  /**
   * The assignment operator is private and must never be called.
   * In fact, it should not even be implemented.
   */
  bb4lFullShowerVeto & operator=(const bb4lFullShowerVeto &);

};

}

#endif /* Herwig_bb4lFullShowerVeto_H */
