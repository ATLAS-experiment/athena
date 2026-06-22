///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// GenEvent_p7.h
// Header file for class GenEvent_p7
// Author: S.Binet<binet@cern.ch>
// Date:   March 2007
///////////////////////////////////////////////////////////////////
#ifndef GENERATOROBJECTSTPCNV_GENEVENT_P7_H
#define GENERATOROBJECTSTPCNV_GENEVENT_P7_H

// STL includes
#include <vector>
#include <utility> //> for std::pair

// forward declarations
class McEventCollectionCnv_p7;

class GenEvent_p7
{
  ///////////////////////////////////////////////////////////////////
  // Friend classes
  ///////////////////////////////////////////////////////////////////

  // Make the AthenaPoolCnv class our friend
  friend class McEventCollectionCnv_p7;

  ///////////////////////////////////////////////////////////////////
  // Public methods
  ///////////////////////////////////////////////////////////////////
public:

  /** Default constructor
   */
  GenEvent_p7();

  /** Constructor with parameters
   */
  GenEvent_p7( int signalProcessId,
               int eventNbr,
               int mpi,
               double eventScale,
               double alphaQCD,
               double alphaQED,
               double filterWeight,
               double filterHT,
               double filterMET,
               int signalProcessVtx,
               int beamParticle1,
               int beamParticle2,
               const std::vector<double>& weights,
               const std::vector<long int>& randomStates,
               const std::vector<double>& crossSection,
               const std::vector<float>& heavyIon,
               const std::vector<double>& pdfinfo,
               int momentumUnit,
               int lengthUnit,
               unsigned int verticesBegin,
               unsigned int verticesEnd,
               unsigned int particlesBegin,
               unsigned int particlesEnd
               ,const std::vector<int>&         e_attribute_id =  std::vector<int>()
               ,const std::vector<std::string>& e_attribute_name = std::vector<std::string>()
               ,const std::vector<std::string>& e_attribute_string = std::vector<std::string>()
               ,const std::vector<std::string>& r_attribute_name = std::vector<std::string>()
               ,const std::vector<std::string>& r_attribute_string = std::vector<std::string>()
               ,const std::vector<std::string>& r_tool_name = std::vector<std::string>()
               ,const std::vector<std::string>& r_tool_version = std::vector<std::string>()
               ,const std::vector<std::string>& r_tool_description = std::vector<std::string>()
               ,const std::vector<std::string>& r_weight_names = std::vector<std::string>()
               );

  ///////////////////////////////////////////////////////////////////
  // Protected data:
  ///////////////////////////////////////////////////////////////////
protected:

  /** Id of the processus being generated
   */
  int m_signalProcessId;

  /** Event number
   */
  int m_eventNbr;

  /** Number of multi particle interactions
   */
  int m_mpi;

  /** Energy scale. see hep-ph/0109068
   */
  double m_eventScale;

  /** value of the QCD coupling. see hep-ph/0109068
   */
  double m_alphaQCD;

  /** value of the QED coupling. see hep-ph/0109068
   */
  double m_alphaQED;

  /** value of the extra weight introduced during reweighting events in filter and value of some variables we filter on
   */
  double m_filterWeight;
  double m_filterHT;
  double m_filterMET;
  /** Barcode of the GenVertex holding the signal process.
   *  0 means that no signal process vertex has been written out.
   *  This may come from upstream limitations, e.g. an absence of this information in the input file.
   */
  int m_signalProcessVtx;

  /** Barcode of the beam particle 1
   */
  int m_beamParticle1;

  /** Barcode of the beam particle 2
   */
  int m_beamParticle2;

  /** Weights for this event.
   *  First weight is used by default for hit and miss.
   */
  std::vector<double> m_weights;

  /** Container of random numbers for the generator states
   */
  std::vector<long int> m_randomStates;

  /** Container of HepMC::GenCrossSection object translated to vector<double>
   */
  std::vector<double> m_crossSection;

  /** Container of HepMC::HeavyIon object translated to vector<double>
   */
  std::vector<float> m_heavyIon;

  /** Container of HepMC::PdfInfo object translated to
   * vector<double> for simplicity
   */
  std::vector<double> m_pdfinfo;

  /** HepMC::Units::MomentumUnit casted to int
   */
  int m_momentumUnit;

  /** HepMC::Units::LengthUnit casted to int
   */
  int m_lengthUnit;

  /** Begin position in the vector of vertices composing this event.
   */
  unsigned int m_verticesBegin;

  /** End position in the vector of vertices composing this event.
   */
  unsigned int m_verticesEnd;

  /** Begin position in the vector of particles composing this event.
   */
  unsigned int m_particlesBegin;

  /** End position in the vector of particles composing this event.
   */
  unsigned int m_particlesEnd;

  /** We define those exactly as in the HepMC3::GenEvent */
  std::vector<int>         m_e_attribute_id;     ///< Attribute owner id for event
  std::vector<std::string> m_e_attribute_name;   ///< Attribute name for event
  std::vector<std::string> m_e_attribute_string; ///< Attribute serialized as string for event
  std::vector<std::string> m_r_attribute_name;   ///< Attribute name for run info
  std::vector<std::string> m_r_attribute_string; ///< Attribute serialized as string for run info
  std::vector<std::string> m_r_tool_name; ///< Name of the used tool
  std::vector<std::string> m_r_tool_version; ///< Version of the used tool
  std::vector<std::string> m_r_tool_description; ///< Description of the used tool
  std::vector<std::string> m_r_weight_names; ///< The weight names
};

///////////////////////////////////////////////////////////////////
/// Inline methods:
///////////////////////////////////////////////////////////////////
inline GenEvent_p7::GenEvent_p7():
  m_signalProcessId  ( -1 ),
  m_eventNbr         ( -1 ),
  m_mpi              ( -1 ),
  m_eventScale       ( -1 ),
  m_alphaQCD         ( -1 ),
  m_alphaQED         ( -1 ),
  m_filterWeight     (  1 ),
  m_filterHT         ( -13 ),
  m_filterMET        ( -13 ),
  m_signalProcessVtx (  0 ),
  m_beamParticle1    (  0 ),
  m_beamParticle2    (  0 ),
  m_weights          (    ),
  m_randomStates     (    ),
  m_crossSection     (    ),
  m_heavyIon         (    ),
  m_pdfinfo          (    ),
  m_momentumUnit     (  0 ),
  m_lengthUnit       (  0 ),
  m_verticesBegin    (  0 ),
  m_verticesEnd      (  0 ),
  m_particlesBegin   (  0 ),
  m_particlesEnd     (  0 )
  ,m_e_attribute_id    (    )
  ,m_e_attribute_name  (    )
  ,m_e_attribute_string(    )
  ,m_r_attribute_name  (    )
  ,m_r_attribute_string(    )
  ,m_r_tool_name(    )
  ,m_r_tool_version(    )
  ,m_r_tool_description(    )
  ,m_r_weight_names(    )
{}

inline GenEvent_p7::GenEvent_p7( int signalProcessId,
                                 int eventNbr,
                                 int mpi,
                                 double eventScale,
                                 double alphaQCD,
                                 double alphaQED,
                                 double filterWeight,
                                 double filterHT,
                                 double filterMET,
                                 int signalProcessVtx,
                                 int beamParticle1,
                                 int beamParticle2,
                                 const std::vector<double>& weights,
                                 const std::vector<long int>& randomStates,
                                 const std::vector<double>& crossSection,
                                 const std::vector<float>& heavyIon,
                                 const std::vector<double>& pdfinfo,
                                 int momentumUnit,
                                 int lengthUnit,
                                 unsigned int verticesBegin,
                                 unsigned int verticesEnd,
                                 unsigned int particlesBegin,
                                 unsigned int particlesEnd
                                 ,const std::vector<int>&         e_attribute_id
                                 ,const std::vector<std::string>& e_attribute_name
                                 ,const std::vector<std::string>& e_attribute_string
                                 ,const std::vector<std::string>& r_attribute_name
                                 ,const std::vector<std::string>& r_attribute_string
                                 ,const std::vector<std::string>& r_tool_name
                                 ,const std::vector<std::string>& r_tool_version
                                 ,const std::vector<std::string>& r_tool_description
                                 ,const std::vector<std::string>& r_weight_names
                                 ) :
  m_signalProcessId  ( signalProcessId ),
  m_eventNbr         ( eventNbr ),
  m_mpi              ( mpi ),
  m_eventScale       ( eventScale ),
  m_alphaQCD         ( alphaQCD ),
  m_alphaQED         ( alphaQED ),
  m_filterWeight     ( filterWeight ),
  m_filterHT         ( filterHT ),
  m_filterMET        ( filterMET ),
  m_signalProcessVtx ( signalProcessVtx ),
  m_beamParticle1    ( beamParticle1 ),
  m_beamParticle2    ( beamParticle2 ),
  m_weights          ( weights ),
  m_randomStates     ( randomStates ),
  m_crossSection     ( crossSection ),
  m_heavyIon         ( heavyIon ),
  m_pdfinfo          ( pdfinfo ),
  m_momentumUnit     ( momentumUnit ),
  m_lengthUnit       ( lengthUnit ),
  m_verticesBegin    ( verticesBegin ),
  m_verticesEnd      ( verticesEnd ),
  m_particlesBegin   ( particlesBegin ),
  m_particlesEnd     ( particlesEnd )
  ,m_e_attribute_id    ( e_attribute_id )
  ,m_e_attribute_name  ( e_attribute_name )
  ,m_e_attribute_string( e_attribute_string )
  ,m_r_attribute_name  ( r_attribute_name )
  ,m_r_attribute_string( r_attribute_string )
  ,m_r_tool_name( r_tool_name )
  ,m_r_tool_version( r_tool_version )
  ,m_r_tool_description( r_tool_description )
  ,m_r_weight_names( r_weight_names )
{}

#endif //> GENERATOROBJECTSTPCNV_GENEVENT_p7_H
