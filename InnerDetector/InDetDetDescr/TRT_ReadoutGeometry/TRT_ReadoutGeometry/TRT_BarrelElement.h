/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TRT_BarrelElement.h
 */

#ifndef TRT_READOUTGEOMETRY_TRT_BARRELELEMENT_H
#define TRT_READOUTGEOMETRY_TRT_BARRELELEMENT_H

#include "TRT_ReadoutGeometry/TRT_BarrelCode.h"
#include "TRT_ReadoutGeometry/TRT_BarrelDescriptor.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include <atomic>

class TRT_ID;

namespace Trk{
  class Surface;
  class SurfaceBounds;
}

class SurfaceCache;

namespace InDetDD {

  class TRT_BarrelConditions;

  /** @class TRT_BarrelElement

      Extended TRT_BaseElement to describe a TRT readout element,
      this is a planar layer with n ( order of 20 ) straws, in one
      of the 32 sectors of the TRT barrel.

      The number of straws described by this readout element can be
      retrieved by nStraws()

      @author: Grant Gorfine
      - modified & maintained: Nick Styles, Andreas Salzburger
      */

  class TRT_BarrelElement final: public TRT_BaseElement  {

  public:

    /** Constructor: */
    TRT_BarrelElement(const GeoVFullPhysVol* volume,
                      const TRT_BarrelDescriptor* descriptor,
                      bool isPositive,
                      unsigned int modIndex,
                      unsigned int phiIndex,
                      unsigned int strawLayIndex,
                      const TRT_ID* idHelper,
                      const TRT_Conditions* conditions);

    TRT_BarrelElement(const TRT_BarrelElement& right);

    /** Destructor: */
    virtual ~TRT_BarrelElement() = default;

    /** Type information */
    virtual TRT_BaseElement::Type type() const override final;

    /**	Returns a pointer to conditions data.  This includes
    	information on dead and noisy wires, as well as wire
    	sags. */
    static const TRT_BarrelConditions * getConditionsData() ;

    /**	Returns a pointer to a descriptor, giving common
    	information on module construction: */
    const TRT_BarrelDescriptor * getDescriptor() const;

    /** Get X Position:
        DEPRECATED. Please use starwCenter(i).x() */
    inline double strawXPos (unsigned int i) const;

    /** Get Y Position:
        DEPRECATED. Please use starwCenter(i).y() */
    inline double strawYPos (unsigned int i) const;

    /** Get Z Position:  (active center)
       DEPRECATED. Please use starwCenter(i).z() */
    inline double strawZPos (unsigned int i) const;

    /** Get the length of the straws (active length): */
    virtual const double & strawLength() const override final;

    /**  StrawDirection. +1 if axis is in same direction as local coordinate, -1 otherwise. */
    virtual int strawDirection() const override final;

    /**	Returns the next-in-phi detector element, or zero if none (forseeing gaps, in other words). */
    const TRT_BarrelElement * nextInPhi() const;

    /**	Returns the previous-in-phi detector element, or zero if none (forseeing gaps, in other words). */
    const TRT_BarrelElement * previousInPhi() const;

    /**	Returns the next-in-R detector element, or zero if none. */
    const TRT_BarrelElement * nextInR() const;

    /** Returns the next-in-R detector element, or zero if none. */
    const TRT_BarrelElement * previousInR() const;

    /**	Sets the next-in-phi detector. */
    void  setNextInPhi(const TRT_BarrelElement *element);

    /**	Sets the previous-in-phi detector. */
    void  setPreviousInPhi(const TRT_BarrelElement *element);

    /**	Sets the next-in-r detector. */
    void  setNextInR(const TRT_BarrelElement *element);

    /**	Sets the previous-in-r detector. */
    void  setPreviousInR(const TRT_BarrelElement *element);

    /**	Doomed (?): */
    const TRT_BarrelCode & getCode() const;

    /** Surface bounds */
    virtual const Trk::SurfaceBounds & strawBounds() const override final;

  private:
    /** Illegal Operations: */
    const TRT_BarrelElement& operator=(const TRT_BarrelElement& right) = delete;
    /** These transforms are effectively to the local coord
      system of a straw derived from GeoModel -> hence CLHEP */
    virtual HepGeom::Transform3D calculateStrawTransform(int straw, GeoAlignmentStore* alignStore) const override;
    HepGeom::Transform3D calculateLocalStrawTransform(int straw) const;
    /** return the surface of the element */
    virtual const Trk::Surface & elementSurface() const override;
    /** create the cache for the element */
    virtual void createSurfaceCache(GeoAlignmentStore* alignStore) const override;
    SurfaceCache createSurfaceCacheHelper() const;

    // Private Member data:
    TRT_BarrelCode m_code;
    const TRT_BarrelDescriptor* m_descriptor;
    const TRT_BarrelElement* m_nextInPhi;
    const TRT_BarrelElement* m_previousInPhi;
    const TRT_BarrelElement* m_nextInR;
    const TRT_BarrelElement* m_previousInR;
    };
}

#include "TRT_BarrelElement.icc"
#endif


