/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


// System include(s):
#include <algorithm>
#include <stdexcept>
#include <utility>


// xAOD include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

#include "TrigNavStructure/StringSerializer.h"

// Local include(s):
#include "xAODTrigger/versions/TrigComposite_v1.h"

#ifndef XAOD_STANDALONE
#include "AthenaKernel/BaseInfo.h"
#endif


namespace xAOD {

  // The "linkColIndices" aux type underwent an in-line schema evolution in October 2024 
  // from vector<vector<uint16_t>> to vector<vector<uint32_t>>.
  //
  // We need to have a static Accessor object defined for this aux name which lives outside of class scope in libxAODTrigger.so
  // This object will be instantiated early, when ROOT reads this libraries dictionary upon opening a file.
  // It will register the updated type in the aux registry at this early time, before any dictionaries 
  // stored in the file are read - which would populate the aux registry with the outdated type if the
  // file was produced prior to the schema update.
  //
  // For consistency, the other Accessors used by the TrigComposite interface implementation are moved up here too. 
  static const SG::Accessor< std::vector< std::string > > acc_linkColNames( "linkColNames" );
  static const SG::Accessor< std::vector< TrigComposite_v1::sgkey_t > > acc_linkColKeys( "linkColKeys" );
  static const SG::Accessor< std::vector< TrigComposite_v1::index_type > > acc_linkColIndices( "linkColIndices" ); // Caution: Schema evolution, October 2024
  static const SG::Accessor< std::vector< uint32_t > > acc_linkColClids( "linkColClids" );

  // Remapped element link keys and indices are decorated temporarily onto immutable nodes via these accessors. 
  // These become solidified when a navigation graph is run through TrigNavSlimmingMTAlg.
  static const SG::Accessor< std::vector< TrigComposite_v1::sgkey_t > > acc_remap_linkColKeys( "remap_linkColKeys" );
  static const SG::Accessor< std::vector< TrigComposite_v1::index_type > > acc_remap_linkColIndices( "remap_linkColIndices" );

  ExcNotIParticleContainer::ExcNotIParticleContainer (const std::string& msg)
    : std::runtime_error (msg)
  {
  }

  const std::string TrigComposite_v1::s_collectionSuffix{"__COLL"};

  // Note: These definitions shadow those in TrigCompositeUtils.py
  const std::string TrigComposite_v1::s_initialRoIString{"initialRoI"};
  const std::string TrigComposite_v1::s_initialRecRoIString{"initialRecRoI"};
  const std::string TrigComposite_v1::s_roiString{"roi"};
  const std::string TrigComposite_v1::s_viewString{"view"};
  const std::string TrigComposite_v1::s_featureString{"feature"};
  const std::string TrigComposite_v1::s_seedString{"seed"};

  const std::string TrigComposite_v1::s_hltSeedingNodeNameString{"L1"};
  const std::string TrigComposite_v1::s_filterNodeNameString{"F"};
  const std::string TrigComposite_v1::s_inputMakerNodeNameString{"IM"};
  const std::string TrigComposite_v1::s_hypoAlgNodeNameString{"H"};
  const std::string TrigComposite_v1::s_comboHypoAlgNodeNameString{"CH"};
  const std::string TrigComposite_v1::s_summaryFilterNodeNameString{"SF"};
  const std::string TrigComposite_v1::s_summaryPassNodeNameString{"HLTPassRaw"};
  const std::string TrigComposite_v1::s_summaryPassExpressNodeNameString{"HLTPassExpress"};
  const std::string TrigComposite_v1::s_summaryPrescaledNodeNameString{"HLTPrescaled"};

  bool TrigComposite_v1::s_throwOnCopyError = false; 

  TrigComposite_v1::TrigComposite_v1() {
  }

  TrigComposite_v1::TrigComposite_v1( const TrigComposite_v1& parent ) : SG::AuxElement(parent) {
    this->makePrivateStore( parent );
  }

  TrigComposite_v1& TrigComposite_v1::operator=( const TrigComposite_v1& rhs ) {
    if(this == &rhs) return *this;
    if( ( ! hasStore() ) && ( ! container() ) ) this->makePrivateStore();

    // Copy the auxiliary variables:
    SG::AuxElement::operator=( rhs );

    // Return this object:
    return *this;
  }

   /////////////////////////////////////////////////////////////////////////////
   //
   //                   Built in accessor functions
   //

  AUXSTORE_OBJECT_SETTER_AND_GETTER( TrigComposite_v1, std::string,
                                      name, setName )

  AUXSTORE_OBJECT_SETTER_AND_GETTER( TrigComposite_v1, std::vector<TrigCompositeUtils::DecisionID>,
                                      decisions, setDecisions )

   //
   /////////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////////
   //
   //               Implementation for the link copy functions
   //

  void TrigComposite_v1::copyLinkInternal(const xAOD::TrigComposite_v1& other, const size_t index, const std::string& newName) {
    this->linkColNamesNC().push_back( newName );
    this->linkColClidsNC().push_back( other.linkColClids().at(index) );
    if (other.isRemapped()) {
      this->linkColKeysNC().push_back( other.linkColKeysRemap().at(index) );
      this->linkColIndicesNC().push_back( other.linkColIndicesRemap().at(index) );
    } else {
      this->linkColKeysNC().push_back( other.linkColKeys().at(index) );
      this->linkColIndicesNC().push_back( other.linkColIndices().at(index) );
    }
  }

  bool TrigComposite_v1::copyLinkFrom(const xAOD::TrigComposite_v1& other, const std::string& name, std::string newName) {
    if (newName.empty()) {
      newName = name;
    }
    bool didCopy = false;
    // Check for the existence of single link
    std::vector<std::string>::const_iterator locationIt;
    locationIt = std::find(other.linkColNames().begin(), other.linkColNames().end(), name);
    if (locationIt != other.linkColNames().end()) {
      size_t index = std::distance(other.linkColNames().begin(), locationIt);
      if (this->hasObjectLink(newName)) {
        if (s_throwOnCopyError) throw std::runtime_error("Already have link with name " + newName);
      } else {
        copyLinkInternal(other, index, newName);
        didCopy = true;
      }
    }
    if (!didCopy && s_throwOnCopyError) throw std::runtime_error("Could not find link with name " + name);
    return didCopy;
  }

  bool TrigComposite_v1::copyLinkFrom(const xAOD::TrigComposite_v1* other, const std::string& name, std::string newName) {
    return copyLinkFrom(*other, name, std::move(newName));
  }

  bool TrigComposite_v1::copyLinkCollectionFrom(const xAOD::TrigComposite_v1& other, const std::string& name, std::string newName) {
    bool didCopy = false;
    // Check for the existence of a collection.
    if (newName.empty()) {
      newName = name;
    }
    const std::string mangledName = name + s_collectionSuffix;
    const std::string mangledNewName = newName + s_collectionSuffix;
    if (other.hasObjectLink(mangledName)) {
      if (this->hasObjectLink(mangledNewName)) {
        if (s_throwOnCopyError) throw std::runtime_error("Already have link collection with name " + newName);
      } else {
        // Copy all links in the collection. Just iterating through the source vector
        for (size_t index = 0; index < other.linkColNames().size(); ++index) {
          if (other.linkColNames().at(index) == mangledName) {
            copyLinkInternal(other, index, mangledNewName);
          }
        }
        didCopy = true;
      }
    }
    if (!didCopy && s_throwOnCopyError) throw std::runtime_error("Could not find link with name " + name);
    return didCopy;
  }
    
  bool TrigComposite_v1::copyLinkCollectionFrom(const xAOD::TrigComposite_v1* other, const std::string& name, std::string newName) {
    return copyLinkCollectionFrom(*other, name, std::move(newName));
  }

  bool TrigComposite_v1::copyAllLinksFrom(const xAOD::TrigComposite_v1& other) {
    bool didCopy = false;
    for (const std::string& name : other.linkColNames()) {
      // Check we don't have one (or more) entries with this raw name (raw = might be mangled).
      if (this->hasObjectLink(name)) continue;
      // Check if the link is for a single object or collection of objects by looking for the mangled suffix
      if (name.ends_with(s_collectionSuffix)) {
        // The copyLinkCollectionFrom call needs the un-mangled name as it is a public fn. It will re-mangle.
        const std::string unmangledName = name.substr(0, name.size() - s_collectionSuffix.size());
        copyLinkCollectionFrom(other, unmangledName);
      } else { // not a collection
        copyLinkFrom(other, name);
      }
      didCopy = true;
    }
    return didCopy;
  }

  bool TrigComposite_v1::copyAllLinksFrom(const xAOD::TrigComposite_v1* other) {
    return copyAllLinksFrom(*other);
  }

   //
   /////////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////////
   //
   //               Implementation for the link accessor functions
   //


   bool TrigComposite_v1::removeObjectLink(const std::string& name) {
      bool removed = false;
      const std::vector< std::string >& names = linkColNames();
      for( size_t i = 0; i < names.size(); ++i ) {
         if( names.at(i) != name ) continue;
         // Remove
         linkColNamesNC().erase( linkColNamesNC().begin() + i );
         linkColKeysNC().erase( linkColKeysNC().begin() + i );
         linkColIndicesNC().erase( linkColIndicesNC().begin() + i );
         linkColClidsNC().erase( linkColClidsNC().begin() + i );
         removed = true;
         break;
      }
      return removed;
   }


   bool TrigComposite_v1::removeObjectCollectionLinks(const std::string& name) {
      bool removed = false;
      const std::vector< std::string >& names = linkColNames();
      const std::string mangledName = name + s_collectionSuffix;
      for( size_t i = 0; i < names.size(); /*noop*/ ) {
         if( names.at(i) == mangledName ) {
            // Remove
            linkColNamesNC().erase( linkColNamesNC().begin() + i );
            linkColKeysNC().erase( linkColKeysNC().begin() + i );
            linkColIndicesNC().erase( linkColIndicesNC().begin() + i );
            linkColClidsNC().erase( linkColClidsNC().begin() + i );
            removed = true;
         } else {
            ++i;
         }
      }
      return removed;
   }

   bool TrigComposite_v1::hasObjectLink( const std::string& name, const CLID clid ) const {

      // Since this function shouldn't throw exceptions too easily,
      // let's be super careful here...
      if( ! (acc_linkColNames.isAvailable( *this ) || acc_linkColClids.isAvailable( *this) ) ) {
         return false;
      }

      // The check itself is pretty simple:
      const std::vector< std::string >& names = acc_linkColNames( *this );
      const std::vector< uint32_t >&    clids = acc_linkColClids( *this );
      
      std::vector<std::string>::const_iterator vecIt = std::find( names.begin(), names.end(), name );
      if (vecIt == names.end()) {
         return false; // Could not find name
      }

      if (clid != CLID_NULL) { // Also check against clid
         const uint32_t storedCLID = clids.at( std::distance( names.begin(), vecIt ) );
         if (clid == ClassID_traits< xAOD::IParticleContainer >::ID()) {
            return derivesFromIParticle(storedCLID);
         } else if (storedCLID != clid) { // Otherwise we require the ID to match
            return false; // Type missmatch
         }
      }

      return true; // Satisfied
   }

   bool TrigComposite_v1::hasObjectCollectionLinks( const std::string& collectionName, const CLID clid ) const {
      const std::string mangledName = collectionName + s_collectionSuffix;
      return hasObjectLink( mangledName, clid );
   }


   bool TrigComposite_v1::hasObjectLinkExact(const std::string& name, const sgkey_t key, const TrigComposite_v1::index_type index, const uint32_t clid) const {
      for (size_t i = 0; i < this->linkColNames().size(); ++i) {
         if (this->linkColNames().at(i) != name) continue;
         if (!SG::sgkeyEqual (this->linkColKeys().at(i), key)) continue;
         if (this->linkColIndices().at(i) != index) continue;
         if (this->linkColClids().at(i) != clid) continue;
         return true;
      } 
      return false;
   }

   bool TrigComposite_v1::derivesFromIParticle(const CLID clid [[maybe_unused]]) const {
#ifndef XAOD_STANDALONE
     const SG::BaseInfoBase* bib = SG::BaseInfoBase::find (clid);
     if (bib) {
       return bib->is_base (ClassID_traits< xAOD::IParticleContainer >::ID());
     }
     // No base info available means we never called any of the macros declaring bases so it's
     // likely that the clid doesn't inherit from IParticle...
     return false;
#endif
     return true;
   }

   AUXSTORE_OBJECT_GETTER( TrigComposite_v1, std::vector< std::string >,
                           linkColNames )
   AUXSTORE_OBJECT_GETTER( TrigComposite_v1, std::vector< uint32_t >,
                           linkColClids )

   const std::vector< SG::sgkey_t >& TrigComposite_v1::linkColKeys() const {
      return acc_linkColKeys( *this );
   }

   const std::vector< TrigComposite_v1::index_type >& TrigComposite_v1::linkColIndices() const {
      return acc_linkColIndices( *this );
   }

   const std::vector< SG::sgkey_t >& TrigComposite_v1::linkColKeysRemap() const {
      return acc_remap_linkColKeys( *this );
   }

   const std::vector< TrigComposite_v1::index_type >& TrigComposite_v1::linkColIndicesRemap() const {
      return acc_remap_linkColIndices( *this );
   }

   ////////

   std::vector< std::string >& TrigComposite_v1::linkColNamesNC() {
      return acc_linkColNames( *this );
   }

   std::vector< SG::sgkey_t >& TrigComposite_v1::linkColKeysNC() {
      return acc_linkColKeys( *this );
   }

   std::vector< TrigComposite_v1::index_type >& TrigComposite_v1::linkColIndicesNC() {
      return acc_linkColIndices( *this );
   }

   std::vector< uint32_t >& TrigComposite_v1::linkColClidsNC() {
      return acc_linkColClids( *this );
   }

   void TrigComposite_v1::typelessSetObjectLink( const std::string& name, const sgkey_t key, const uint32_t clid, const TrigComposite_v1::index_type beginIndex, const TrigComposite_v1::index_type endIndex ) {

     // Loop over collections
     if ( int32_t(endIndex) - int32_t(beginIndex) > 1 ) { // Adding a *collection* of links, this needs to be a signed check as a difference <= 0 implies that endIndex is less than or equal to beginIndex

       // Check uniqueness
       const std::string mangledName = name + s_collectionSuffix;
       const std::vector< std::string >& names = linkColNames();
       int oldStart = -1;
       int oldEnd = -1;
       for( size_t nameIndex = 0; nameIndex < names.size(); ++nameIndex ) {

         // Look for an existing collection with the same name
         if( names[ nameIndex ] == mangledName ) {
           oldEnd = nameIndex + 1;
           if ( oldStart == -1 ) oldStart = nameIndex;
         }
         else if ( oldStart != -1 ) {
           // If the start has been found, we must now be past the ned
           break;
         }
       }

       // Erase the old collection, if there was one
       if ( oldStart != -1 ) {
         this->linkColNamesNC().erase( this->linkColNamesNC().begin() + oldStart, this->linkColNamesNC().begin() + oldEnd );
         this->linkColKeysNC().erase( this->linkColKeysNC().begin() + oldStart, this->linkColKeysNC().begin() + oldEnd );
         this->linkColIndicesNC().erase( this->linkColIndicesNC().begin() + oldStart, this->linkColIndicesNC().begin() + oldEnd );
         this->linkColClidsNC().erase( this->linkColClidsNC().begin() + oldStart, this->linkColClidsNC().begin() + oldEnd );
       }

       // Append the new collection
       for ( TrigComposite_v1::index_type index = beginIndex; index < endIndex; ++index ) {
         this->linkColNamesNC().push_back( mangledName );
         this->linkColKeysNC().push_back( key );
         this->linkColIndicesNC().push_back( index );
         this->linkColClidsNC().push_back( clid );
       }

     } else { // Adding a *single* link

       // Check uniqueness
       if ( std::find( linkColNamesNC().begin(), linkColNamesNC().end(), name ) == linkColNamesNC().end() ) {

         this->linkColNamesNC().push_back( name );
         this->linkColKeysNC().push_back( key );
         this->linkColIndicesNC().push_back( beginIndex );
         this->linkColClidsNC().push_back( clid );

       } else {

         // Over-write an existing object
         const std::vector< std::string >& names = linkColNames();
         for( size_t nameIndex = 0; nameIndex < names.size(); ++nameIndex ) {
           if( names[ nameIndex ] == name ) {
             this->linkColKeysNC()[ nameIndex ] = key;
             this->linkColIndicesNC()[ nameIndex ] = beginIndex;
             this->linkColClidsNC()[ nameIndex ] = clid;
             break; // Names are unique, so stop once found
           } // Check of names
         } // Loop over names

       } // Check of uniqueness of adding single link

     } // Check of adding single link vs link collection
   }

   bool TrigComposite_v1::typelessGetObjectLink( const std::string& name, sgkey_t& key, uint32_t& clid, TrigComposite_v1::index_type& index) const {
      std::vector<std::string>::const_iterator it = std::find(linkColNames().begin(), linkColNames().end(), name);
      if (it == linkColNames().end()) {
         return false;
      }
      const size_t location = std::distance(linkColNames().begin(), it);
      if (isRemapped()) {
         key = linkColKeysRemap().at(location);
         clid = linkColClids().at(location);
         index = linkColIndicesRemap().at(location);
      } else {
         key = linkColKeys().at(location);
         clid = linkColClids().at(location);
         index = linkColIndices().at(location);
      }
      return true;
   }


   bool TrigComposite_v1::typelessGetObjectCollectionLinks( const std::string& name, 
     std::vector<sgkey_t>& keyVec, std::vector<uint32_t>& clidVec, std::vector<TrigComposite_v1::index_type>& indexVec ) const
   {
      bool found = false;
      const std::string mangledName = name + s_collectionSuffix;
      for (size_t i = 0; i < this->linkColNames().size(); ++i) {
         if (linkColNames().at(i) != mangledName) {
            continue;
         }
         if (isRemapped()) {
            keyVec.push_back( linkColKeysRemap().at(i) );
            clidVec.push_back( linkColClids().at(i) );
            indexVec.push_back( linkColIndicesRemap().at(i) );
         } else {
            keyVec.push_back( linkColKeys().at(i) );
            clidVec.push_back( linkColClids().at(i) );
            indexVec.push_back( linkColIndices().at(i) );
         }
         found = true;
      }
      return found;
   }


   bool TrigComposite_v1::isRemapped() const {
      size_t nDecorations = 0;
      if (acc_remap_linkColKeys.isAvailable( *this )) ++nDecorations;
      if (acc_remap_linkColIndices.isAvailable( *this )) ++nDecorations;
      if (nDecorations == 1) {
        throw std::runtime_error("TrigComposite_v1::isRemapped Only one of the 'remap_linkColKeys' and 'remap_linkColIndices' "
          "decorations were found on this object. This should never happen, a remapped element link must have both of these collections.");
      }
      return static_cast<bool>(nDecorations); //0=False, 2=True
   }


   std::vector<std::string> TrigComposite_v1::getObjectNames(const CLID clid) const {

      std::vector< std::string > returnVec;
      const std::vector< std::string >& names = linkColNames();
      const std::vector< uint32_t >& clids = linkColClids();

      for( size_t i = 0; i < names.size(); ++i ) {
         if (names[i].find(s_collectionSuffix) != std::string::npos) { //Note: !=
            continue; // ARE *NOT* interested in collection links here
         }
         bool clidMatch = false;
         if (clid == ClassID_traits< xAOD::IParticleContainer >::ID()) {
            clidMatch = derivesFromIParticle(clids[i]);
         } else if (clid == clids[i]) {
            clidMatch = true;
         }
         if (clidMatch) {
            returnVec.push_back( names[i] );
         }
      }
      return returnVec;
   }

   std::vector<std::string> TrigComposite_v1::getObjectCollectionNames(const CLID clid) const {

      std::vector< std::string > returnVec;
      const std::vector< std::string >& names = linkColNames();
      const std::vector< uint32_t >& clids = linkColClids();

      for( size_t i = 0; i < names.size(); ++i ) {
         if (names[i].find(s_collectionSuffix) == std::string::npos) { // Note: ==
            continue; // ARE interested in collection links here
         }
         bool clidMatch = false;
         if (clid == ClassID_traits< xAOD::IParticleContainer >::ID()) {
            clidMatch = derivesFromIParticle(clids[i]);
         } else if (clid == clids[i]) {
            clidMatch = true;
         }
         if (clidMatch) {
            // Unlike with single links, we expect to find multiple links here. Only add the name once. 
            // Name is mangled in storage, need to un-mangle it before returning it to the user. 
            const std::string unmangledName = names[i].substr(0, names[i].size() - s_collectionSuffix.size());
            if ( std::none_of(returnVec.begin(), returnVec.end(), [&](const auto& s) {return unmangledName == s;}) ) {
               returnVec.push_back( std::move(unmangledName) );
            }
         }
      }
      return returnVec;
   }

   //
   /////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, const xAOD::TrigComposite_v1& tc) {
  os << "TrigComposite_v1 name:'" << tc.name() << "'" << std::endl;
  const bool isRemapped = tc.isRemapped();
  os << "  N Links:" << tc.linkColNames().size() << ", isRemapped:" << (isRemapped ? "YES" : "NO");
  for (size_t i=0; i<tc.linkColNames().size(); ++i){
    if (!i) os << std::endl;
    os << "    Link Name:"  << tc.linkColNames()[i];
    os << ", Key:"   << tc.linkColKeys()[i];
    if (isRemapped) os << ", RemappedKey:"   << tc.linkColKeysRemap()[i];
    os << ", Index:" << tc.linkColIndices()[i];
    if (isRemapped) os << ", RemappedIndex:" << tc.linkColIndicesRemap()[i];
    os << ", CLID:"  << tc.linkColClids()[i];
    if (i != tc.linkColNames().size() - 1) os << std::endl;
  }
  if (!tc.decisions().empty()) {
    os << std::endl << "  N Decisions:" << tc.decisions().size() << std::endl << "    ";
    for (const TrigCompositeUtils::DecisionID id : tc.decisions()) os << id << ", ";
  }
  return os;
}

} // namespace xAOD
