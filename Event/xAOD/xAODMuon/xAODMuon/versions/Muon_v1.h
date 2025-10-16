// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// $Id: Muon_v1.h 745098 2016-05-05 15:47:04Z wleight $
#ifndef XAODMUON_VERSIONS_MUON_V1_H
#define XAODMUON_VERSIONS_MUON_V1_H

// Core include(s):
#include "AthContainers/AuxElement.h"
#include "AthLinks/ElementLink.h"

//xAOD Primitives
#include "xAODPrimitives/IsolationCorrection.h"
#include "xAODPrimitives/IsolationType.h"
#include "xAODPrimitives/IsolationFlavour.h"

// Misc xAOD include(s):
#include "xAODBase/IParticle.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include <bitset>
#include <stdint.h>

// ROOT include(s):
#include "Math/Vector4D.h"

namespace xAOD {
  /// Class describing a Muon.
  ///
  /// @author Edward Moyse <Edward.Moyse@cern.ch>
  /// @nosubgrouping
  class Muon_v1 : public IParticle {

  public:
    /// inject the enums
    #include "xAODMuon/versions/MuonEnums.def"
    /// Default constructor
    Muon_v1();

    /// Copy constructor
    Muon_v1(const Muon_v1& rhs);

    /// Destructor
    virtual ~Muon_v1();

    /// Assignment operator
    Muon_v1& operator=(const Muon_v1& rhs);

    /// @name IParticle functions
    /// @{
    /// The transverse momentum (\f$p_T\f$) of the particle.
    virtual double           pt() const;
    /// The pseudorapidity (\f$\eta\f$) of the particle.
    virtual double           eta() const;
    /// The azimuthal angle (\f$\phi\f$) of the particle.
    virtual double           phi() const;
    /// The invariant mass of the particle..
    virtual double           m() const;
    /// The total energy of the particle.
    virtual double           e() const;
    /// The true rapidity (y) of the particle.
    virtual double           rapidity() const;

    /// Set method for IParticle values
    void setP4(double pt, double eta, double phi);

    /// Definition of the 4-momentum type.
    typedef IParticle::FourMom_t FourMom_t;

    /// The full 4-momentum of the particle.
    virtual FourMom_t p4() const;

    /// Base 4 Momentum type for Muon
    typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > GenVecFourMom_t;

    ///  The full 4-momentum of the particle : GenVector
    GenVecFourMom_t genvecP4() const;

    /// The type of the object as a simple enumeration
    virtual Type::ObjectType type() const;
    /// @}

    /// @name Charge
    /// Returns the charge.
    float charge() const;

    /// Set the charge (*must* be the same as primaryTrackParticle() )
    void setCharge(float charge);

    /// @name Author
    /// Methods to query the author(s) of this Muon
    /// @{
    /// @todo - update and add documentation.
    /// Returns the primary author of this Muon.
    Author author() const;
    /// Returns 'true' if 'author' is the an author of this muon.
    bool isAuthor ( const Author author ) const;
    /// set author
    void setAuthor(Author auth);
    /// Get all the authors of this Muon.
    /// For example during overlap checking, the same Muon may have been reconstructed by many different algorithms. This method returns a 16bit
    /// number, where each bit represents a muon algorithm, defined as follows (the lowest bit is indicates that something has gone wrong):
    /// unknown | MuidCo | STACO | MuTag | MuTagIMO | MuidSA | MuGirl | MuGirlLowBeta | CaloTag | CaloLikelihood | CaloScore | ExtrapolateMuonToIP | MuonCombinedRefit | ExtrapolateMuonToIP | Commissioning
    /// @returns  16-bit word, 1-bit reserved for each muon Algorithm:
    uint16_t allAuthors() const;
    void setAllAuthors(uint16_t authors);
    /// add author to all authors
    void addAllAuthor( const Author author );

    /// @}

    /// @name Summary information
    /// Return summary information about the muon, such as its MuonType, and the TrackSumary values of the primary TrackParticle.
    /// @{
    ///@todo Add documentation.
    /// Generic accessor to type information.
    MuonType muonType() const;
    /// @todo - do we actually need this? Deduce it from other information?
    void setMuonType(MuonType type);

    /// Accessor for TrackSummary values (in most cases, retrieved from the 'primary' TrackParticle - though it could be stored on the Muon, depending on the
    /// job configuration)
    /// If 'information' is stored in the primary TrackParticle/Muon and is of the correct templated type T,
    /// then the function fills 'value' and returns 'true', otherwise it returns 'false', and does not touch 'value'.
    /// See below for an example of how this is intended to be used.
    /// @code
    /// uint8_t numberOfInnermostPixelLayerHits=0;
    /// if( myParticle.summaryValue<uint8_t>(numberOfInnermostPixelLayerHits,numberOfInnermostPixelLayerHits) ){
    ///   ATH_MSG_INFO("Successfully retrieved the integer value, numberOfInnermostPixelLayerHits");
    /// }
    /// float numberOfCscPhiHits=0.0; //Wrong! This is actually an int too.
    /// if( !myParticle.summaryValue<float>(numberOfCscPhiHits,numberOfCscPhiHits) ){
    ///   ATH_MSG_INFO("Types must match!");
    /// }
    /// @endcode
    /// @param[in] information The information being requested. This is not guaranteed to be stored in all Muons (or primary TrackParticle).
    /// @param[out] value  Only filled if this Muon (or its primary TrackParticle) contains 'information', and the types match.
    /// @return Returns 'true' if the Muon contains 'information', and its concrete type matches 'value' (templated type T).
    bool summaryValue(uint8_t& value, const SummaryType information) const;
    /// Set method for storing TrackSummary SummaryType information on the Muon (see Aux to see which is already defined as static)
    void setSummaryValue(uint8_t value, const SummaryType information);
    /// @copydoc bool summaryValue(uint8_t& value, const SummaryType information) const;
    bool summaryValue(float& value, const SummaryType information) const;
    /// Accessor for MuonSummaryType.
    bool summaryValue(uint8_t& value, const MuonSummaryType information) const;
    /// Set method for MuonSummaryType.
    void setSummaryValue(uint8_t value, const MuonSummaryType information);

    /// Same as bool summaryValue(float& value, const SummaryType &information) const , but without check (will throw exception if value isn't there)
    /// Primarily for use in Python.
    float floatSummaryValue(const SummaryType information) const;
    /// Same as bool summaryValue(uint8_t& value, const SummaryType &information) const, but without check (will throw exception if value isn't there)
    /// Primarily for use in Python.
    uint8_t uint8SummaryValue(const SummaryType information) const;
    /// Same as bool summaryValue(uint8_t& value, const MuonSummaryType &information) const, but without check (will throw exception if value isn't there)
    /// Primarily for use in Python.
    float uint8MuonSummaryValue(const MuonSummaryType information) const;

    /// Get a parameter for this Muon - momentumBalanceSignificance for example
    /// @todo Finish documentation
    /// include matchChi2, muonentrancechi2 (instead of outerMatchChi2). Store chi2/dof instead of two?
    /// fitChi2 comes from TrackParticle.
    bool parameter(float& value, const ParamDef parameter) const;

    /// Set method for parameter values.
    void setParameter(float value, const ParamDef parameter);

    /// Same as bool parameter(float& value, const ParamDef &parameter) const, but without check (will throw exception if value isn't there).
    /// Primarily for use in Python.
    float floatParameter(const ParamDef parameter) const;

    /// Get an integer parameter for this Muon - msInnerMatchDOF for example
    bool parameter(int& value, const ParamDef parameter) const;

    /// Set method for parameter values.
    void setParameter(int value, const ParamDef parameter);

    /// Same as bool parameter(float& value, const ParamDef &parameter) const, but without check (will throw exception if value isn't there).
    /// Primarily for use in Python.
    int intParameter(const ParamDef parameter) const;

    /// Muon CP quality accessors.
    /// These methods get/set the muon's "quality" decoration, which is a bitfield:
    /// - Bits 0-2 hold the xAOD::Muon::Quality enum (Tight -> VeryLoose).
    /// - Higher bits may store auxiliary flags (e.g. ID hit-cuts).
    /// This decoration is currently populated by MuonSelectionTool (see MCP documentation).

    /// get/set the Quality enum in bits 0-2, preserving higher-bit flags
    Quality quality() const;
    void setQuality(Quality);

    /// MCP ID hit cuts - get/set the corresponding status bit in the quality decoration.
    bool passesIDCuts() const;
    void setPassesIDCuts(bool);

    /// @}

    /// @name Isolation information.
    ///
    /// @{

    /// @brief Accessor for Isolation values.
    /// If 'information' is stored in this xAOD::Muon and is of the correct type,
    /// then the function fills 'value' and returns 'true', otherwise it returns 'false', and does not touch 'value'.
    bool isolation(float& value,   const Iso::IsolationType information) const;

    /// Accessor to Isolation values , this just returns the value without internaly checking if it exists.
    /// Will lead to an exception if the information is not available
    float isolation(const Iso::IsolationType information) const;

    /// Set method for Isolation values.
    void setIsolation(float value, const Iso::IsolationType information);
    /// @}

    /// @brief Accessor for Isolation Calo correction.
    /// If 'information' is stored in this xAOD::Muon and is of the correct type,
    /// then the function fills 'value' and returns 'true', otherwise it returns 'false', and does not touch 'value'.
    bool isolationCaloCorrection(float& value, const Iso::IsolationFlavour flavour, const Iso::IsolationCaloCorrection type,
                                 const Iso::IsolationCorrectionParameter param) const;

    /// Accessor to Isolation Calo corrections , this just returns the correction without internaly checking if it exists.
    /// Will lead to an exception if the information is not available
    float isolationCaloCorrection(const Iso::IsolationFlavour flavour, const Iso::IsolationCaloCorrection type,
                                  const Iso::IsolationCorrectionParameter param) const;

    /// set method for Isolation Calo Corrections.
    bool setIsolationCaloCorrection(float value, const Iso::IsolationFlavour flavour, const Iso::IsolationCaloCorrection type,
                                    const Iso::IsolationCorrectionParameter param);


    /// @brief Accessor for Isolation Track correction.
    bool isolationTrackCorrection(float& value, const Iso::IsolationFlavour flavour , const Iso::IsolationTrackCorrection type ) const;

    /// Accessor to Isolation Track corrections , this just returns the correction without internaly checking if it exists.
    /// Will lead to an exception if the information is not available
    float isolationTrackCorrection(const Iso::IsolationFlavour flavour , const Iso::IsolationTrackCorrection type) const;

    /// Set method for Isolation Track Corrections.
    bool setIsolationTrackCorrection(float value, const Iso::IsolationFlavour flavour , const Iso::IsolationTrackCorrection type);


    /// @brief Accessor for Isolation corection Bitset
    bool isolationCorrectionBitset(std::bitset<32>& value, const Iso::IsolationFlavour flavour ) const;

    /// Accessor to Isolation corection Bitset , this just returns the bitset without internaly checking if it exists.
    /// Will lead to an exception if the information is not available
    std::bitset<32> isolationCorrectionBitset(const Iso::IsolationFlavour flavour ) const;

    /// Set method for Isolation corection Bitset.
    bool setIsolationCorrectionBitset(uint32_t value, const Iso::IsolationFlavour flavour );

    /// @}

    /// @name Links
    /// With the following methods you can retrieve links to the objects used to identify this muon - depending on how the muon was built the link may
    /// or may not be valid (i.e. a muon built from a standalone MS track won't have an ID TrackParticle associated to it).
    /// @todo finish documentation
    /// @code
    /// Add some code here, showing how to properly use the element links
    /// @endcode
    /// @note Some links were removed from the "Run-1" AOD::muon, in particular
    /// @{
    /// @brief Returns an ElementLink  to the primary TrackParticle corresponding to the MuonType of this muon. This is determined in the following order:
    ///  1. CombinedTrackParticle
    ///  2. InnerDetectorTrackParticle
    ///  3. ExtrapolatedMuonSpectrometerTrackParticle
    ///  4. MSOnlyExtrapolatedMuonSpectrometerTrackParticle
    ///  5. MuonSpectrometerTrackParticle
    /// This method can throw a std::runtime_error exception if either the 'muontype' is unknown, or if the type is MuonStandAlone,
    /// but there is no available extrapolatedMuonSpectrometerTrackParticleLink or muonSpectrometerTrackParticleLink to return.
    const ElementLink< TrackParticleContainer >& primaryTrackParticleLink() const;

    /// @brief Returns a pointer (which should not usually be NULL, but might be if the muon has been stripped of information) to the
    /// primary TrackParticle corresponding to the MuonType of this muon.
    ///This is determined in the following order:
    ///  1. CombinedTrackParticle
    ///  2. InnerDetectorTrackParticle
    ///  3. ExtrapolatedMuonSpectrometerTrackParticle
    ///  4. MSOnlyExtrapolatedMuonSpectrometerTrackParticle
    ///  5. MuonSpectrometerTrackParticle
    const TrackParticle* primaryTrackParticle() const;

    /// @brief Returns an ElementLink to the InnerDetector TrackParticle used in identification of this muon.
    const ElementLink< TrackParticleContainer >& inDetTrackParticleLink() const;
    /// @brief Returns an ElementLink to the InnerDetector TrackParticle used in identification of this muon.
    const ElementLink< TrackParticleContainer >& muonSpectrometerTrackParticleLink() const;
    /// @brief Returns an ElementLink to the InnerDetector TrackParticle used in identification of this muon.
    const ElementLink< TrackParticleContainer >& combinedTrackParticleLink() const;
    /// @brief Returns an ElementLink to the Extrapolated Muon Spectrometer TrackParticle used in identification of this muon.
    const ElementLink< TrackParticleContainer >& extrapolatedMuonSpectrometerTrackParticleLink() const;
    /// @brief Returns an ElementLink to the MS-only Extrapolated Muon Spectrometer TrackParticle used in identification of this muon.
    const ElementLink< TrackParticleContainer >& msOnlyExtrapolatedMuonSpectrometerTrackParticleLink() const;

    /// @brief Returns an ElementLink to the  TrackParticle used in identification of this muon.
    const ElementLink< TrackParticleContainer >& trackParticleLink( TrackParticleType type) const;
    /// @brief Set method for TrackParticle links.
    void setTrackParticleLink(TrackParticleType type, const ElementLink< TrackParticleContainer >& link);
    /// @brief Returns a pointer (which can be NULL) to the  TrackParticle used in identification of this muon.
    const TrackParticle* trackParticle( TrackParticleType type) const;

    /// @brief Returns an ElementLinkto the cluster associated to this muon.
    ///@todo Why just one?
    const ElementLink<CaloClusterContainer>& clusterLink() const;
    /// @brief Set method for cluster links.
    void setClusterLink(const ElementLink<CaloClusterContainer>& link);
    /// Retrieve the associated cluster with a bare pointer
    const CaloCluster* cluster() const;

    /** Energy determined from parametrization or not (measured). The actual energy loss is returned via
    @code
    float etCore;
    bool hasEnergyLoss = parameter(float& value, const ParamDef parameter)
    @endcode
    */
    EnergyLossType energyLossType (void) const;
    /// Set method for the type
    void setEnergyLossType (EnergyLossType type) ;

    ///@todo complete the various calo energy additions (i.e. depositInCalo etc)

    /// @brief Returns a vector of ElementLinks to the MuonSegments used to create this Muon.
    const std::vector< ElementLink< MuonSegmentContainer > > & muonSegmentLinks() const;
    /// @brief Set the vector of ElementLinks to the MuonSegments used to create this Muon.
    void setMuonSegmentLinks(const std::vector< ElementLink< MuonSegmentContainer > >& segments) ;
    /// @brief Number of MuonSegments linked to by this Muon.
    size_t nMuonSegments() const;
    /// @brief Returns a pointer to the specified MuonSegment.
    /// @param i Index of the MuonSegment requested. If i is not in range (0<i<nMuonSegments()) an exception will be thrown.
    const MuonSegment* muonSegment( size_t i ) const;
    /// @brief Returns a link to the specified MuonSegment.
    /// @param i Index of the MuonSegment requested. If i is not in range (0<i<nMuonSegments()) an exception will be thrown.
    const ElementLink< MuonSegmentContainer >& muonSegmentLink( size_t i ) const;

    /// @}
  }; // class xAOD::Muon

} // namespace xAOD

// Declare IParticle as a base class of Muon_v1:
#include "AthContainers/DataVector.h"
  DATAVECTOR_BASE( xAOD::Muon_v1, xAOD::IParticle );

#endif // XAODMUON_VERSIONS_Muon_v1_H
