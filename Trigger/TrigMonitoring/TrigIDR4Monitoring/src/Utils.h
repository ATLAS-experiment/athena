/* emacs: this is -*- c++ -*- */
/**
 **   @file    Utils.h        
 **                   
 **   @author  sutt
 **   @date    Sat  1 Aug 2026 13:12:33 BST
 **
 **   $Id: Utils.h, v0.0   Sat  1 Aug 2026 13:12:33 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  IDTPM2_UTILS_H
#define  IDTPM2_UTILS_H

#include <vector>
#include <string>
#include <memory>


namespace IDTPM2 {
  
template<typename T, typename Iterator>
void append_pointers( std::vector<const T*>& t, Iterator first, Iterator last ) {
  for ( ; first!=last ; first++ ) t.push_back( &*first );
}

template<typename T, typename Iterator>
void fill_pointers( std::vector<const T*>& t, Iterator first, Iterator last ) {
  t.clear();
  for ( ; first!=last ; first++ ) t.push_back( &*first );
}

template<typename T, typename Iterator>
void fill_upointers( std::vector<const T*>& t, Iterator first, Iterator last ) {
  t.clear();
  for ( ; first!=last ; first++ ) t.push_back( first->get() );
}

  
/// don't actually really need this, just call insert directly !!
template<typename T, typename Iterator>
void append( std::vector<const T*>& t, Iterator first, Iterator last ) {
  t.insert( t.end(), first, last );
}
  
template<typename T, typename Iterator>
void fillv( std::vector<T>& t, Iterator first, Iterator last ) {
  t = std::vector<T>(first, last ); 
}

template<typename T, typename V=T, typename Iterator>
void fillv( std::vector<std::unique_ptr<T>>& t, Iterator first, Iterator last ) {
  t.clear(); t.reserve(last-first);
  for ( ; first!=last ; first++ ) t.push_back(std::make_unique<V>(*first)); 
}
  

// get the current system time   
std::string stime();
  
}

#endif // IDTPM2_UTILS_H
