/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDENTIFIER_IDCONTEXT_H
#define IDENTIFIER_IDCONTEXT_H

#include "Identifier/ExpandedIdentifier.h"

/**
 *  @class IdContext
 *
 *  @brief This class saves the "context" of an expanded identifier
 *  (ExpandedIdentifier) for compact or hash versions (Identifier32 or
 *  IdentifierHash). This context is composed of 
 *
 *    1) begin and end indices of fields that are stored in the
 *       compact/hash id
 *    2) a possible "prefix" identifier for cases where the begin
 *       index is not 0 or the top level of the expaneded identifier. 
 *
 *  The IdContext is needed when only some of the identifier levels
 *   are to encoded in the compact/hash ids. 
 */

class IdContext{
public:
  //
  typedef ExpandedIdentifier::size_type 	size_type;
  //
  IdContext() = default;
  /// Construct with no prefix
  IdContext(size_type begin_index, size_type end_index);
  /// Constructors with full initialization of members
  IdContext(const ExpandedIdentifier& prefix, 
      size_type begin_index, 
      size_type end_index);
  //    
  IdContext(ExpandedIdentifier&& prefix, 
      size_type begin_index, 
      size_type end_index);
      
  ///@{ Accessors
  inline const ExpandedIdentifier&	prefix_id() const{ return m_prefix;}
  inline size_type begin_index() const{ return m_begin_index;}
  inline size_type end_index() const{return m_end_index;};
  ///@}
  
  /// Set all the private members
  void set(const ExpandedIdentifier& prefix,size_type begin_index,size_type end_index);
    
private:
  ExpandedIdentifier	m_prefix{};
  size_type		m_begin_index{};
  size_type		m_end_index{};
};


inline 
IdContext::IdContext(size_type begin_index, size_type end_index):
  m_begin_index(begin_index),
  m_end_index(end_index){/* */}


inline void			
IdContext::set(const ExpandedIdentifier& prefix, size_type begin_index, size_type end_index){
    m_prefix = prefix;
    m_begin_index = begin_index;
    m_end_index = end_index;
}

#endif // IDENTIFIER_IDCONTEXT_H
