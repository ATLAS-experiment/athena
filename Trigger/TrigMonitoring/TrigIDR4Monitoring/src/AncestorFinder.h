/** emacs: this is -*- c++ -*- 
 **
 **   @file         AncestorFinder.h  
 **
 **   @author       sutt  
 **   @date         Sun 16 Aug 2026 14:27:59 BST  
 ** 
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration **
 **
 **/


#ifndef   IDTPM2_ANCESTORFINDER_H
#define   IDTPM2_ANCESTORFINDER_H

#include "ConfiguredValue.h"

#include <cmath>


/// Search the ancestor history, looking for an ancestor that
/// satifies the predicate ...

namespace IDTPM2 { 

template<typename T> 
class IFinder  { 

public:
  
  virtual ~IFinder() = default; 

  const T* findAncestor(const T* t) const {

    // reached the end ...
    if (!t) return 0;

    /// successful stopping condition ... 
    if ( predicate(t) ) return t;

    for ( size_t p=0 ; p<t->nParents() ; p++ ) {
        const T* parent = t->parent(p);

        if ( parent==t ) continue;

	/// isn't recursion great boys and girls ? 
        if ( const T* found = findAncestor(parent) ) return found;
    }

    return 0;
  }

  virtual bool predicate( const T* t ) const = 0;
    
};



/// allow generic predicate types ...

template<typename T, typename Predicate>
class GenericAncestorFinder : public IFinder<T> {
  
public:
  
  GenericAncestorFinder( Predicate p ) : m_predicate(p) { }   

  bool predicate( const T* t ) const override { return m_predicate(t); } 
  
private:

  Predicate m_predicate;
  
};



/// predicate base class, so we can take any predicates and store
/// them on a vector or in a unique_ptr<IPreciate<T>>  

template<typename T>
class IPredicate {
public:
  virtual ~IPredicate() {}
  virtual bool operator()(const T* t) const = 0;
};
  


/// and a specific PDG ID helper, so we can select particles of
/// a particular type
/// NB: this is a class, since we will want to decode with pdg id
///     this is and store it dureing the initialise in the class
///     itself

template<typename T>
class PdgIdHelper : public IPredicate<T> {

public:

  PdgIdHelper(int pdgid) : m_pdgid(std::abs(pdgid)) {}

  /// this is quite clever - if we want to pass in an actual class
  /// the if we defined operator() then for an instance of object called,
  /// say "aclass" then we can call aclass() is if aclass was actually
  /// a function - totally wild !
  bool operator()(const T* t) const { return std::abs(t->pdgId()) == m_pdgid; }

private:

  int m_pdgid;

};



template<typename T>
class PtHelper : public IPredicate<T> {

public:

  PtHelper( ConfiguredValue<float> pt={} ) : m_pt(pt) {}

  /// this is quite clever - can pass the class instance and use it like a function
  bool operator()(const T* t) const {
    if ( std::abs(t->pt()) < m_pt ) return false;
    return true;
  }

private:

  ConfiguredValue<float> m_pt;

};

  
template<typename T, typename U=T>
class PtPdgIdHelper : public IPredicate<T> {

public:

  PtPdgIdHelper( ConfiguredValue<float> pt={}, const ConfiguredValue<int> pdgid={}) :
    m_pt(pt), m_pdgid(pdgid) {}

  /// this is quite clever - can pass the class instance and use it like a function
  bool operator()(const T* t) const {
    if ( std::abs(t->pt()) < m_pt ) return false;
    if ( std::abs(t->template as<U>()->pdgId()) != m_pdgid ) return false;
    return true;
  }

private:

  ConfiguredValue<float> m_pt;
  ConfiguredValue<int>   m_pdgid;

};

  

 
/// now a specific selector using the above PDG ID helper
/// so we can select whatever sort we want - even exotics
/// type with non standard pdg ids

template<typename T>
class PdgAncestorFinder : public GenericAncestorFinder<T, PdgIdHelper<T>> {
public:
  PdgAncestorFinder( int i ) : GenericAncestorFinder<T, PdgIdHelper<T> >( PdgIdHelper<T>(i)) { }
};



 
/// a type using a generic bool function pointer since - for some reason,
/// I can not fathom, people like to use eg MC::isTau(p) rather than say,
/// just t->pdfId() == MC::Tau - I mean why just use a value you can change
/// at runtime, when you could hard code an actual function that you
/// can't change at all, even if you need to

template<typename T, typename P=bool (*)(const T*)>
class PredicateAncestorFinder : public GenericAncestorFinder<T, P> {
public:
  PredicateAncestorFinder( P p ) : GenericAncestorFinder<T, P>(p) { }
};

 


/// allows more sophisticated selection predicates that use class
/// functions, such as isBottomHadron() etc, rather than
/// MC::isBottomHadron( t ) which woudl seem to be justified since
/// people unacccountably like to use MC::isTau( ... ) etc

template<typename T>
class HadronAncestorFinder : public IFinder<T> {

public:
  
  using Predicate = bool (T::*)() const;
  
  HadronAncestorFinder( Predicate p ) : m_predicate(p) { }   
  
  bool predicate( const T* t ) const override { return (t->*m_predicate)(); } 
  
private:

  Predicate m_predicate;
  
};


} // namespace IDTPM2


#endif  // IDTPM2_ANCESTORFINDER_H 










