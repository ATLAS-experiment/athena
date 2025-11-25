/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "IdDictParser/IdDictParser.h"  
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "IdDict/IdDictRange.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/Range.h" 
#include "Identifier/Identifier.h" 
 
#include <iostream> 
 
static void tab (size_t level) 
{ 
  for (size_t i = 0; i < level; ++i) std::cout << " "; 
} 
 
int main (int argc, char* argv[])  
{  
  if (argc < 2) return (1);  
  
  IdDictParser parser;  
  
  const IdDictMgr& idd = parser.parse (argv[1]);  
 
  int n = 0; 
 
  for (const IdDictDictionary* dictionary : idd.get_dictionaries())
    { 
      std::cout << "---- " << n << " ----------------------------" << std::endl; 
      std::cout << "Dictionary " << dictionary->name() << std::endl;

      size_t nregions = dictionary->n_regions();
      for (size_t i = 0; i < nregions; ++i)
        {
          const IdDictRegion& region = dictionary->region(i);
          std::cout << "region #" << region.index() << std::endl;
 
          size_t width = 0; 

          size_t nimpl = region.n_implementation();
          for (size_t i = 0; i < nimpl; ++i) {
              const IdDictFieldImplementation& impl = region.implementation(i);
              size_t w = impl.range()->field()->name().size ();
 
              if (w > width) width = w; 
            } 
 
          int bits = 0; 
 
         for (size_t i = 0; i < nimpl; ++i) {
              const IdDictFieldImplementation& impl = region.implementation(i);

              size_t w = impl.range()->field()->name().size ();

              std::cout << "  implement field #" << impl.range()->field()->index() <<
                  " " << impl.range()->field()->name();
 
              tab (width - w); 
 
              std::cout << " -> " << (std::string) impl.field() <<  
                  "/" << (std::string) impl.ored_field() <<  
                  " (" << impl.bits() << " bits)" <<  
                  std::endl; 
 
              bits += impl.bits(); 
            } 
 
          Range range = region.build_range ();
 
          std::cout << " -> " << (std::string) range <<  
              " (cardinality=" << range.cardinality () << ")" << 
              " (" << bits << " bits)" << std::endl; 
        } 
    } 
 
  { 
    const IdDictDictionary* dictionary = idd.find_dictionary ("InnerDetector"); 
 
    if (dictionary != 0) 
      { 
        ExpandedIdentifier id ("1/0/0/21/6/319/191"); 
 
//  	    IdDictDictionary::bits32 b = dictionary->pack32 (id, 0, 6); 
	Identifier packedB((Identifier::value_type)0);
	if (dictionary->pack32 (id, 0, 6, packedB) != 0) {
          std::cout << "error from pack32\n";
          return 1;
        }
 
        std::cout << "b=[" << packedB << "]" << std::endl; 
 
	ExpandedIdentifier id2;
	dictionary->unpack ("pixel", packedB, ExpandedIdentifier (), 6, id2);
 
        std::cout << "unpack->[" << (std::string) id2 << "]" << std::endl; 
      } 
  } 
 
  return 0;  
}  
  
 
 
 
 
 
