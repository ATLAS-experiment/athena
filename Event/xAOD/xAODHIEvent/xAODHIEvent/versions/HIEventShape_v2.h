// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: HIEventShape_v2.h 693494 2015-09-07 14:59:45Z krasznaa $
#ifndef XAODHIEVENT_VERSIONS_HIEVENTSHAPE_V2_H
#define XAODHIEVENT_VERSIONS_HIEVENTSHAPE_V2_H

// System include(s):
#include <vector>

// EDM include(s):
#include "AthContainers/AuxElement.h"

/// Namespace holding all the xAOD classes/functions
namespace xAOD {

   /// Interface class for the HI reconstruction EDM
   ///
   /// This class describes the underlying event shape in calorimeter slices
   /// The description can be of varying granularity.
   /// The most granular form is when energy and azimuthal modulations
   ///   are calculated for slices of eta for each calorimeter layer.
   /// The tool responsible for filling this information is HIEventShapeFillerTool.
   ///
   /// @author Aaron Angerami <angerami@cern.ch>
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   ///
   class HIEventShape_v2 : public SG::AuxElement {

   public:
      /// Default constructor
      HIEventShape_v2();

      /// @name Energy density information
      /// @{

      /// Transverse energy reconstructed on the slice
      float et() const;
      /// Set the reconstructed transverse energy
      void setEt( float value );

      /// obtain the area of the eta slice 
      float area() const;
      /// set the area of the eta slice
      void setArea( float value );

      /// energy density (et/area)
      float rho() const;
      /// set the rho value
      void setRho( float value );

      /// number of cells that were summed in slice
      int nCells() const;
      /// set the number of cells summed in slice
      void setNCells( int value );

      /// @}

      /// @name Harmonic modulation
      /// @{

      /// cosine (y) part of the harmonic modulation strength
      /// Following convention is used: index 0 is first harmonic, 1 is the second one etc.
      /// The other of harmonics is arbitrary and depends on the code filling it.
      /// The modulation is et weighted.
      const std::vector< float >& etCos() const;
      /// read write accessor for cosine modulation
      std::vector< float >& etCos();
      /// set the cosine harmonic modulation
      void setEtCos( const std::vector< float >& value );

      /// sine (x) part of the harmonic modulation strength
      /// @see etCos
      const std::vector<float>& etSin() const;
      /// read-write accessor for since modulation
      std::vector<float>& etSin();
      /// set the sine harmonic modulation
      void setEtSin( const std::vector< float >& value );

      /// @}

      /// @name Information about the slice
      /// @{

      /// eta slice "left" edge
      float etaMin() const;
      /// set eta slice "left" edge
      void setEtaMin( float value );

      /// eta slice "right" edge
      float etaMax() const;
      /// set eta slice "right" edge
      void setEtaMax( float value );

      /// calorimeter layer for which the quantities were calculated
      /// 0 - all calorimeter depth
      /// others numbers are calorimeter layers ID 
      int layer() const;
      /// set layer information
      void setLayer( int value );

      /// @}

   }; // class HIEventShape_v2

} // namespace xAOD

// Declare SG::AuxElement as a base of the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::HIEventShape_v2, SG::AuxElement );

#endif // XAODHIEVENT_VERSIONS_HIEVENTSHAPE_V2_H
