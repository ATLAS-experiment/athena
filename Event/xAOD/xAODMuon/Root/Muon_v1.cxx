/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Misc includes
#include <vector>

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackSummaryAccessors_v1.h"
#include "xAODPrimitives/tools/getIsolationAccessor.h"
#include "xAODPrimitives/tools/getIsolationCorrectionAccessor.h"

// Local include(s):
#include "xAODMuon/versions/Muon_v1.h"
#include "MuonAccessors_v1.h"
#include "xAODMuon/versions/MuonTrackSummaryAccessors_v1.h"
// Athena-only includes

#include "TruthUtils/ParticleConstants.h"

#define IMPLEMENT_LINK_GETTER(ContType, DECORATOR_NAME)                        \
    {                                                                          \
       static const SG::Accessor<ElementLink<ContType>> acc{DECORATOR_NAME};   \
       if (!acc.isAvailable(*this)) {                                          \
          return nullptr;                                                      \
       }                                                                       \
       const auto& link = acc(*this);                                          \
       if (!link.isValid()){                                                   \
          return nullptr;                                                      \
       }                                                                       \
       return *link;                                                           \
    }                                                                          \

namespace xAOD {

  Muon_v1::Muon_v1(const Muon_v1& rhs)
    : IParticle(rhs) //IParticle does not have a copy constructor. AuxElement has one with same behavior as default ctor
  {
    this->makePrivateStore(rhs);
  }
  
  Muon_v1& Muon_v1::operator=(const Muon_v1& rhs ){
    if(this == &rhs) return *this;
 
    if( ( ! hasStore() ) && ( ! container() ) ) {
       makePrivateStore();
    }
    this->IParticle::operator=( rhs );
    
    return *this;
  }
  
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( Muon_v1, float, double, pt)
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( Muon_v1, float, double, eta)
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( Muon_v1, float, double, phi)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( Muon_v1, float, charge, setCharge )

  // AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( Muon_v1, float, double, e)
  
  double Muon_v1::e() const {
    // FIXME - optimise?
    return genvecP4().E();
  }
  
  double Muon_v1::m() const {
    return ParticleConstants::muonMassInMeV;
  }

  void Muon_v1::setP4(double pt, double eta, double phi)  {
    static const Accessor< float > acc1( "pt" );
    static const Accessor< float > acc2( "eta" );
    static const Accessor< float > acc3( "phi" );
    acc1( *this )=pt;
    acc2( *this )=eta;
    acc3( *this )=phi;
  }

  double Muon_v1::rapidity() const {
    return genvecP4().Rapidity();
  }

  Muon_v1::FourMom_t Muon_v1::p4() const {
    Muon_v1::FourMom_t p4;
    p4.SetPtEtaPhiM( pt(), eta(), phi(), m() ); 
    return p4;
  }

  // depend on return value optimization
  Muon_v1::GenVecFourMom_t Muon_v1::genvecP4() const {
    return GenVecFourMom_t(pt(), eta(), phi(), m());
  }

  Type::ObjectType Muon_v1::type() const {
    return Type::Muon;
  }  

  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( Muon_v1, uint16_t, Muon_v1::Author, author) 
  AUXSTORE_PRIMITIVE_SETTER_WITH_CAST( Muon_v1, uint16_t, Muon_v1::Author, author, setAuthor) 

  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( Muon_v1, uint16_t, allAuthors, setAllAuthors)


  void Muon_v1::addAllAuthor ( const Author author ){
    static const Accessor< uint16_t > acc( "allAuthors" );
    acc(*this) |= 1<<static_cast<unsigned int>(author);
  }

  bool Muon_v1::isAuthor ( const Author author ) const{
    static const Accessor< uint16_t > acc( "allAuthors" );
    return (acc(*this)& (1<<static_cast<unsigned int>(author)));
  }

  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( Muon_v1, uint16_t, Muon_v1::MuonType, muonType)
  AUXSTORE_PRIMITIVE_SETTER_WITH_CAST( Muon_v1, uint16_t, Muon_v1::MuonType, muonType, setMuonType)

  bool Muon_v1::summaryValue(uint8_t& value, const SummaryType information)  const {
    const auto* acc = trackSummaryAccessorV1<uint8_t>( information );
    if (acc->isAvailable(*this))  {
        value = (*acc)( *this );
        return true;
    }
    // Okay - fallback: try to get from TrackParticle.
    const TrackParticle* primTrk = trackParticle(TrackParticleType::Primary);
    return primTrk->summaryValue(value, information);
  }  

  void Muon_v1::setSummaryValue( uint8_t  value, const SummaryType 	information ) {
    const Muon_v1::Accessor< uint8_t >* acc = trackSummaryAccessorV1<uint8_t>( information ); ///FIXME!
    // Set the value:
    ( *acc )( *this ) = value;
  }

  // No set method for 'float' values as not expected to be needed
   
  bool Muon_v1::summaryValue(float& value, const SummaryType information)  const {
    return trackParticle(TrackParticleType::Primary)->summaryValue(value,information);
  }  
  
  float Muon_v1::floatSummaryValue(const SummaryType information) const {
    const Muon_v1::Accessor< float >* acc = trackSummaryAccessorV1< float >( information );
  	return ( *acc )( *this );
  }

  uint8_t Muon_v1::uint8SummaryValue(const SummaryType information) const{
    const Muon_v1::Accessor< uint8_t >* acc = trackSummaryAccessorV1< uint8_t >( information );
    return ( *acc )( *this );  	
  }
  
  bool Muon_v1::summaryValue(uint8_t& value, const MuonSummaryType information)  const {
    const auto& acc = muonTrackSummaryAccessorV1( information );
    if( !acc.isAvailable( *this ) ) {
      value = 0;
      return false;
    }
    // Retrieve the value:
    value = acc( *this );
    return true;
  }
  
  float Muon_v1::uint8MuonSummaryValue(const MuonSummaryType information) const{
    uint8_t sumVal{0};
    summaryValue(sumVal, information);
    return sumVal;
  }
  

  void Muon_v1::setSummaryValue(uint8_t value, const MuonSummaryType information) {
    const auto& acc = muonTrackSummaryAccessorV1( information );
    // Set the value:
    acc(*this) =  value;
  }
  
  bool Muon_v1::parameter(float& value, const ParamDef information)  const {
    const Muon_v1::Accessor< float >* acc = parameterAccessorV1<float>( information );
    if( ! acc || ! acc->isAvailable( *this ) ) {
      value = 0.;
      return false;
    }
    // Retrieve the value:
    value = ( *acc )( *this );
    return true;
  }

  float Muon_v1::floatParameter(const ParamDef information) const{
    float sumVal{0.f};
    parameter(sumVal, information);
    return sumVal;
  }

  void Muon_v1::setParameter(float value, const ParamDef information){
    const Muon_v1::Accessor< float >* acc = parameterAccessorV1<float>( information );
    if( ! acc ) {
      throw std::runtime_error("Muon_v1::setParameter - no float accessor for paramdef number: "
                              +std::to_string(information));
    }
    // Set the value:
    ( *acc )( *this ) = value;
  }
  
  bool Muon_v1::parameter(int& value, const ParamDef information)  const {
    const Muon_v1::Accessor< int >* acc = parameterAccessorV1<int>( information );
    if( ! acc || ! acc->isAvailable( *this ) ) {
      value = 0; 
      return false;
    }
    // Retrieve the value:
    value = ( *acc )( *this );
    return true;
  }
	
  int Muon_v1::intParameter(const ParamDef information) const{
      int sumValue{0};
      parameter(sumValue, information);
      return sumValue;
  }

  void Muon_v1::setParameter(int value, const ParamDef information) {
    const Muon_v1::Accessor< int >* acc = parameterAccessorV1<int>( information );
    if( ! acc ) {
      throw std::runtime_error("Muon_v1::setParameter - no int accessor for paramdef number: "+std::to_string(information));
    }
    // Set the value:
    ( *acc )( *this ) = value;
  }

  Muon_v1::Quality Muon_v1::quality() const {
    static const Accessor< uint8_t > acc( "quality" );
    uint8_t temp =  acc( *this );
    return static_cast<Quality>(temp&3);     
  }
  
  void Muon_v1::setQuality(const Quality value) {
    static const Accessor< uint8_t > acc( "quality" );
    uint8_t temp = static_cast< uint8_t >(value);
    acc( *this ) = acc( *this ) & ~(0x7); // Reset the first 3 bits.
    acc( *this ) |= temp;
    return;      
  }
  
  bool Muon_v1::passesIDCuts() const {
    static const Accessor< uint8_t > acc( "quality" );
    uint8_t temp =  acc( *this );
    // We use 4th bit for 'passesIDCuts'
    return temp&8;     
  }
  
  void Muon_v1::setPassesIDCuts(bool value) {
    static const Accessor< uint8_t > acc( "quality" );
    // We use 4th bit for 'passesIDCuts'
    if (value) acc( *this ) |= 8;
    else       acc( *this ) &= 247;
    return;      
  }
  
  bool Muon_v1::isolation(float& value, const Iso::IsolationType information)  const {
    const SG::AuxElement::Accessor< float >* acc = getIsolationAccessor( information );
    
    if( ! acc || !acc->isAvailable( *this) ){
       value =0.;
       return  false;
    }
    // Retrieve the value:
    value = ( *acc )( *this );
    return true;
  }
  
  float Muon_v1::isolation( const Iso::IsolationType information)  const {
    float isoVal{0.f};
    isolation(isoVal, information);
    return isoVal;
  }
  
  void Muon_v1::setIsolation(float value, const Iso::IsolationType information){
    const SG::AuxElement::Accessor< float >* acc = getIsolationAccessor( information );
    if( !acc ) {
      throw std::runtime_error( "Unknown/Unavailable Isolation type requested" );
    }
    // Set the value:
    ( *acc )( *this ) = value;
  }
  
bool Muon_v1::isolationCaloCorrection(  float& value, const Iso::IsolationFlavour flavour, 
                                        const Iso::IsolationCaloCorrection type,
                                        const Iso::IsolationCorrectionParameter param) const{
    const SG::AuxElement::Accessor< float > acc = getIsolationCorrectionAccessor(flavour,type,param);
    if( !acc.isAvailable( *this) ) return false;
    // Retrieve the value:
    value = acc( *this );
    return true;
  }

  float Muon_v1::isolationCaloCorrection(const Iso::IsolationFlavour flavour, const Iso::IsolationCaloCorrection type,
  const Iso::IsolationCorrectionParameter param) const{

    const SG::AuxElement::Accessor< float > acc = getIsolationCorrectionAccessor(flavour,type,param);
    if( !acc.isAvailable( *this) ) throw std::runtime_error( "Unknown/Unavailable Isolation correction requested" );
    return  acc( *this );
  }

  bool Muon_v1::setIsolationCaloCorrection(float value, const Iso::IsolationFlavour flavour, const Iso::IsolationCaloCorrection type,
  const Iso::IsolationCorrectionParameter param){
    const SG::AuxElement::Accessor< float > acc = getIsolationCorrectionAccessor(flavour,type,param); 
    // Set the value:
    acc( *this ) = value;
    return true;
  }

  bool Muon_v1::isolationTrackCorrection(float& value, const Iso::IsolationFlavour flavour, const Iso::IsolationTrackCorrection type) const{
    const SG::AuxElement::Accessor< float > acc = getIsolationCorrectionAccessor(flavour,type);
    if( !acc.isAvailable( *this) ) return  false;
    // Retrieve the value:
    value = acc( *this );
    return true;
  }

  float Muon_v1::isolationTrackCorrection(const Iso::IsolationFlavour flavour, const Iso::IsolationTrackCorrection type) const{

    const SG::AuxElement::Accessor< float > acc = getIsolationCorrectionAccessor(flavour,type);
    if( !acc.isAvailable( *this) ) throw std::runtime_error( "Unknown/Unavailable Isolation correction requested" );
    return  acc( *this );
  }

  bool Muon_v1::setIsolationTrackCorrection(float value, const Iso::IsolationFlavour flavour, const Iso::IsolationTrackCorrection type){
    const SG::AuxElement::Accessor< float > acc = getIsolationCorrectionAccessor(flavour,type);
    // Set the value:
    acc( *this ) = value;
    return true;
  }

  bool Muon_v1::isolationCorrectionBitset(std::bitset<32>& value, const Iso::IsolationFlavour flavour ) const{
    const SG::AuxElement::Accessor< uint32_t > acc = getIsolationCorrectionBitsetAccessor( flavour );
    if( !acc.isAvailable( *this) ) return false;
    // Retrieve the value:
    value = std::bitset<32>(acc( *this ));
    return true;
  }

  std::bitset<32> Muon_v1::isolationCorrectionBitset(const Iso::IsolationFlavour flavour ) const{
    const SG::AuxElement::Accessor< uint32_t > acc = getIsolationCorrectionBitsetAccessor( flavour );
    if( !acc.isAvailable( *this) ) throw std::runtime_error( "Unknown/Unavailable Isolation BitSet requested" );
    return  std::bitset<32>( acc( *this ) );
  }

  bool Muon_v1::setIsolationCorrectionBitset(uint32_t value, const Iso::IsolationFlavour flavour ) {
    const SG::AuxElement::Accessor< uint32_t > acc = getIsolationCorrectionBitsetAccessor( flavour );
    // Set the value:
    acc( *this ) = value;
    return true;
  }

  AUXSTORE_OBJECT_GETTER( Muon_v1, ElementLink< TrackParticleContainer >, inDetTrackParticleLink)
  AUXSTORE_OBJECT_GETTER( Muon_v1, ElementLink< TrackParticleContainer >, muonSpectrometerTrackParticleLink)
  AUXSTORE_OBJECT_GETTER( Muon_v1, ElementLink< TrackParticleContainer >, extrapolatedMuonSpectrometerTrackParticleLink)
  AUXSTORE_OBJECT_GETTER( Muon_v1, ElementLink< TrackParticleContainer >, msOnlyExtrapolatedMuonSpectrometerTrackParticleLink)
  AUXSTORE_OBJECT_GETTER( Muon_v1, ElementLink< TrackParticleContainer >, combinedTrackParticleLink)

  const ElementLink< TrackParticleContainer >& Muon_v1::primaryTrackParticleLink() const{
    switch ( muonType() ) {
      case Combined :
      case SiliconAssociatedForwardMuon : {
        return combinedTrackParticleLink();
      } case SegmentTagged :
        case CaloTagged :
        return inDetTrackParticleLink();
      case MuonStandAlone :
        {          
          // Not checking if links are valid here - this is the job of the client (as per the cases above).
          // But we DO check that the link is available, so we can check for both types of links.
          
          static const Accessor< ElementLink< TrackParticleContainer > > acc1( "extrapolatedMuonSpectrometerTrackParticleLink" );
          if ( acc1.isAvailable( *this ) && acc1( *this ).isValid() ) {
            return acc1( *this );
          }

          static const Accessor< ElementLink< TrackParticleContainer > > acc2( "msOnlyExtrapolatedMuonSpectrometerTrackParticleLink" );
          if ( acc2.isAvailable( *this ) && acc2( *this ).isValid() ) {
            return acc2( *this );
          }
          
          static const Accessor< ElementLink< TrackParticleContainer > > acc3( "muonSpectrometerTrackParticleLink" );
          if ( acc3.isAvailable( *this ) && acc3( *this ).isValid()) {            
            return acc3( *this );
          }
          // We could also just return a dummy EL here, but the link is part of the aux store, and so it might be that something bad has happened...?
          throw std::runtime_error("Type is MuonStandAlone but no available link to return!");
        }
      default:
        throw std::runtime_error("Unknown primary type - not sure which track particle to return!");
    }
    // static ElementLink< TrackParticleContainer > dummy;
    // return dummy;
  }
  
  const TrackParticle* Muon_v1::primaryTrackParticle() const{
      return trackParticle(TrackParticleType::Primary);
  }
  
  const ElementLink< TrackParticleContainer >& Muon_v1::trackParticleLink( TrackParticleType type) const{
    using LinkAcc_t = SG::Accessor<ElementLink< TrackParticleContainer >>;
    switch ( type ) {
      case Primary :
        return primaryTrackParticleLink();
      case CombinedTrackParticle : {
          return combinedTrackParticleLink();
      } case InnerDetectorTrackParticle :{
          return inDetTrackParticleLink();
      } case MuonSpectrometerTrackParticle :{
          return muonSpectrometerTrackParticleLink();
      } case ExtrapolatedMuonSpectrometerTrackParticle :{
          return extrapolatedMuonSpectrometerTrackParticleLink();
      } case MSOnlyExtrapolatedMuonSpectrometerTrackParticle : {
          return msOnlyExtrapolatedMuonSpectrometerTrackParticleLink();
      } default:
        throw std::runtime_error("Unknown TrackParticleType - not sure which track particle to return!");
    }
    // static ElementLink< TrackParticleContainer > dummy;
    // return dummy;
    
  }
  
  const TrackParticle* Muon_v1::trackParticle(const TrackParticleType type) const{
    switch ( type ) {
      using enum TrackParticleType;
      case Primary : {
          switch(muonType()) {
              using enum MuonType;
              case Combined:
              case SiliconAssociatedForwardMuon : {
                return trackParticle(TrackParticleType::CombinedTrackParticle);
              } case SegmentTagged:
                case CaloTagged : {
                  return trackParticle(TrackParticleType::InnerDetectorTrackParticle);
              } case MuonStandAlone : {
                  for (const auto MsType : {ExtrapolatedMuonSpectrometerTrackParticle,
                                            MSOnlyExtrapolatedMuonSpectrometerTrackParticle,
                                            MuonSpectrometerTrackParticle}) {
                    if (const TrackParticle* msTrk = trackParticle(MsType); msTrk != nullptr) {
                       return msTrk;
                    }
                  }
                  return nullptr;
              } default: return nullptr;
          }
      } case CombinedTrackParticle : 
        IMPLEMENT_LINK_GETTER(TrackParticleContainer, "combinedTrackParticleLink");
      case InnerDetectorTrackParticle :
        IMPLEMENT_LINK_GETTER(TrackParticleContainer, "inDetTrackParticleLink");
      case MuonSpectrometerTrackParticle :
        IMPLEMENT_LINK_GETTER(TrackParticleContainer, "muonSpectrometerTrackParticleLink");
      case ExtrapolatedMuonSpectrometerTrackParticle :
        IMPLEMENT_LINK_GETTER(TrackParticleContainer, "extrapolatedMuonSpectrometerTrackParticleLink");
      case MSOnlyExtrapolatedMuonSpectrometerTrackParticle :
        IMPLEMENT_LINK_GETTER(TrackParticleContainer, "msOnlyExtrapolatedMuonSpectrometerTrackParticleLink");
    }
    return nullptr;
  }

  void Muon_v1::setTrackParticleLink(const TrackParticleType type, const ElementLink< TrackParticleContainer >& link){
    switch ( type ) {
      case InnerDetectorTrackParticle :
        static const Accessor< ElementLink< TrackParticleContainer > > acc1( "inDetTrackParticleLink" );
        acc1(*this)=link;
        break;
      case MuonSpectrometerTrackParticle :
        static const Accessor< ElementLink< TrackParticleContainer > > acc2( "muonSpectrometerTrackParticleLink" );
        acc2(*this)=link;
        break;
      case CombinedTrackParticle :
        static const Accessor< ElementLink< TrackParticleContainer > > acc3( "combinedTrackParticleLink" );
        acc3(*this)=link;          
        break;
      case ExtrapolatedMuonSpectrometerTrackParticle :
        static const Accessor< ElementLink< TrackParticleContainer > > acc4( "extrapolatedMuonSpectrometerTrackParticleLink" );
        acc4(*this)=link;
        break;
      case MSOnlyExtrapolatedMuonSpectrometerTrackParticle :
        static const Accessor< ElementLink< TrackParticleContainer > > acc5( "msOnlyExtrapolatedMuonSpectrometerTrackParticleLink" );
	acc5(*this)=link;
	break;
      case Primary :
      default:
        throw std::runtime_error("Unknown or Primary TrackParticleType - not sure which track particle to set!");
    }
  }
  
  AUXSTORE_OBJECT_SETTER_AND_GETTER( Muon_v1, ElementLink<CaloClusterContainer>, clusterLink, setClusterLink)
  const CaloCluster* Muon_v1::cluster() const { 
    
    if (const TrackParticle* idTrack = trackParticle(TrackParticleType::InnerDetectorTrackParticle); 
        idTrack == nullptr) {
        return nullptr;
    }
    IMPLEMENT_LINK_GETTER(CaloClusterContainer, "clusterLink");
  } 

  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( Muon_v1, uint8_t, Muon_v1::EnergyLossType,
                                       energyLossType )
  AUXSTORE_PRIMITIVE_SETTER_WITH_CAST( Muon_v1, uint8_t, Muon_v1::EnergyLossType,
                                       energyLossType, setEnergyLossType )

  AUXSTORE_OBJECT_SETTER_AND_GETTER( Muon_v1, std::vector< ElementLink< MuonSegmentContainer > >, muonSegmentLinks, setMuonSegmentLinks)

  static const SG::AuxElement::Accessor< std::vector< ElementLink< MuonSegmentContainer > > > muonSegmentsAcc( "muonSegmentLinks" ); 
  size_t Muon_v1::nMuonSegments() const {
        // If a link was not set (yet), return zero.
    if( ! muonSegmentsAcc.isAvailable( *this ) ) {
      return 0;
    }
    return muonSegmentsAcc(*this).size();
  }

  const ElementLink< MuonSegmentContainer >& Muon_v1::muonSegmentLink( size_t i ) const {
        // If a Trk::Track link was not set (yet), return a dummy object:
      // FIXME - maybe 
    if( ! muonSegmentsAcc.isAvailable( *this ) ) {
      static const ElementLink< MuonSegmentContainer > dummy;
      return dummy;
    }
    return muonSegmentsAcc(*this).at(i);
  }

  const MuonSegment* Muon_v1::muonSegment( size_t i ) const{	
    if (i >= nMuonSegments()) {
        return nullptr;
    }
    const ElementLink< MuonSegmentContainer >& el = muonSegmentsAcc(*this).at(i);
    if (!el.isValid()) {
      return nullptr;
    }
    return *el;
  }
} // namespace xAOD

