/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author quarrie  
 * @date 2008-12-09 
**/
 
#include "IdDictParser/IdDictParser.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictDictionaryRef.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "IdDict/IdDictLabel.h"
#include "IdDict/IdDictRange.h"
#include "IdDict/IdDictReference.h"
#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictSubRegion.h"
#include "IdDict/IdDictAltRegions.h"
 
#include <cstdlib>
#include <iostream> 

namespace IdDictParserNS{
  class Debugger { 
  public: 
    static bool get_debug_state(){
      return ::getenv ("XMLDEBUG") != 0;
    }
    //
    static bool debug (){
      static const bool debug_state = get_debug_state();
      return debug_state;
    }
    //
    static void tab (int n) {
	    std::cout << std::string(n, ' '); 
    } 
  }; 
  
  class IdDictBaseFactory : public XMLCoreFactory  { 
  public:  
    void do_start (XMLCoreParser& parser, const XMLCoreNode& node);  
    void do_end (XMLCoreParser& parser, const XMLCoreNode& node);  
    virtual void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
    virtual void idd_end (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class IdDictMgrFactory : public IdDictBaseFactory  {  
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
    void idd_end (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class DictionaryFactory : public IdDictBaseFactory  {   
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
    void idd_end (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class FieldFactory : public IdDictBaseFactory  {   
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
    void idd_end (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class LabelFactory : public IdDictBaseFactory  {   
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
    void idd_end (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class AltRegionsFactory : public IdDictBaseFactory  {   
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
    void idd_end   (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class RegionFactory : public IdDictBaseFactory  {   
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
    void idd_end (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class SubRegionFactory : public IdDictBaseFactory  {   
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
    void idd_end (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class RegionEntryFactory : public IdDictBaseFactory  {   
  public:  
    virtual void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
    virtual void idd_end (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class RangeFactory : public RegionEntryFactory  {   
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class ReferenceFactory : public RegionEntryFactory  {   
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
  };  
  
  class DictionaryRefFactory : public RegionEntryFactory  {   
  public:  
    void idd_start (IdDictParser& parser, const XMLCoreNode& node);  
  };  
}

using namespace IdDictParserNS;
 
IdDictParser::IdDictParser () : XMLCoreParser () {  
  register_factory ("IdDict",            std::make_unique<IdDictMgrFactory>());  
  register_factory ("IdDictionary",      std::make_unique<DictionaryFactory>());  
  register_factory ("field",             std::make_unique<FieldFactory>());  
  register_factory ("label",             std::make_unique<LabelFactory>());  
  register_factory ("alternate_regions", std::make_unique<AltRegionsFactory>());  
  register_factory ("region",            std::make_unique<RegionFactory>());  
  register_factory ("subregion",         std::make_unique<SubRegionFactory>());  
  register_factory ("range",             std::make_unique<RangeFactory>());  
  register_factory ("reference",         std::make_unique<ReferenceFactory>());  
  register_factory ("dictionary",        std::make_unique<DictionaryRefFactory>());
 
  m_dictionary = 0; 
  m_field      = 0; 
  m_region     = 0; 
  m_subregion  = 0; 
  m_altregions = 0;
  m_regionentry = 0;
} 
  
IdDictMgr& 
IdDictParser::parse (const std::string& file_name, const std::string& tag)  { 
  m_idd.clear (); 
  XMLCoreParser::visit (file_name);  
  if (Debugger::debug ()) { 
    std::cout << "IdDictParser::parse1>" << std::endl; 
  } 
  m_idd.resolve_references (); 
  if (Debugger::debug ()) { 
    std::cout << "IdDictParser::parse2>" << std::endl; 
  } 
  m_idd.generate_implementation (tag); 
  if (Debugger::debug ()) { 
    std::cout << "IdDictParser::parse3>" << std::endl; 
  } 
  return (m_idd); 
}  
  
void 
IdDictBaseFactory::do_start (XMLCoreParser& parser,   
                             const XMLCoreNode& node)  {
  parser.up (); 
  idd_start (static_cast<IdDictParser&>(parser), node);
}  
  
void 
IdDictBaseFactory::do_end (XMLCoreParser& parser, const XMLCoreNode& node)  {  
  idd_end (static_cast<IdDictParser&>(parser), node);
  parser.down (); 
}  
  
void IdDictBaseFactory::idd_start (IdDictParser& /*parser*/,   
				   const XMLCoreNode& /*node*/)  {  
}  
  
void IdDictBaseFactory::idd_end (IdDictParser& /*parser*/,   
				 const XMLCoreNode& /*node*/)  {  
}  
 
  
  
void 
IdDictMgrFactory::idd_start (IdDictParser& parser, const XMLCoreNode& node)  {  
  if (Debugger::debug ()) { 
    Debugger::tab (parser.level());
    std::cout << "IdDictMgrFactoryFactory::idd_start>" << std::endl; 
  } 
  parser.m_idd.set_DTD_version(get_value (node, "DTD_version"));
}  
  
void 
IdDictMgrFactory::idd_end (IdDictParser& parser,   
				const XMLCoreNode& /*node*/)  { 
  if (Debugger::debug ()) { 
    Debugger::tab (parser.level());
    std::cout << "IdDictMgrFactoryFactory::idd_end>" << std::endl; 
  } 
} 
  
void 
DictionaryFactory::idd_start (IdDictParser& parser, const XMLCoreNode& node)  {  
  if (Debugger::debug ()) { 
    Debugger::tab (parser.level());
    std::cout << "DictionaryFactory::idd_start>" << std::endl; 
  }
  parser.m_dictionary = new IdDictDictionary (get_value (node, "name"),
                                              get_value (node, "version"),
                                              get_value (node, "date"),
                                              get_value (node, "author"));
}  
  
void 
DictionaryFactory::idd_end (IdDictParser& parser, const XMLCoreNode& /*node*/)  { 
  if (Debugger::debug ()) { 
    Debugger::tab (parser.level());
    std::cout << "DictionaryFactory::idd_end> d=" << parser.m_dictionary << std::endl; 
  } 
 
  if (parser.m_dictionary != 0){ 
    parser.m_idd.add_dictionary (parser.m_dictionary); 
    parser.m_dictionary = 0; 
  } 
}  
  
void 
FieldFactory::idd_start (IdDictParser& parser, const XMLCoreNode& node)  {  
  if (Debugger::debug ()) { 
    Debugger::tab (parser.level());
    std::cout << "FieldFactory::idd_start>" << std::endl; 
  } 
  parser.m_field = new IdDictField(get_value (node, "name"));
}  
  
void 
FieldFactory::idd_end (IdDictParser& parser, const XMLCoreNode& /*node*/)  {  
  if (Debugger::debug ()) { 
    Debugger::tab (parser.level());
    std::cout << "FieldFactory::idd_end>" << std::endl; 
  } 

  if (parser.m_field != 0) { 
      if (parser.m_dictionary != 0) parser.m_dictionary->add_field (parser.m_field);  
      else delete parser.m_field; 
      parser.m_field = 0; 
  } 
}  
  
void 
LabelFactory::idd_start (IdDictParser& parser, const XMLCoreNode& node)  {  
  if (Debugger::debug ()) { 
      Debugger::tab (parser.level());
      std::cout << "LabelFactory::idd_start>" << std::endl; 
  } 
 
  IdDictLabel* label;
  if (has_attribute (node, "value")) {
      label = new IdDictLabel (get_value (node, "name"), get_int (node, "value"));
  } else { 
      label = new IdDictLabel (get_value (node, "name"));
  } 
  if (parser.m_field != 0) parser.m_field->add_label (label);  
  else delete label; 
}  
  
void 
LabelFactory::idd_end (IdDictParser& parser, const XMLCoreNode& /*node*/)  {  
  if (Debugger::debug ()) { 
      Debugger::tab (parser.level());
      std::cout << "LabelFactory::idd_end>" << std::endl; 
    } 
}  
  
void 
AltRegionsFactory::idd_start (IdDictParser& parser, const XMLCoreNode& /*node*/)  {  
  if (Debugger::debug ()) { 
      Debugger::tab (parser.level());
      std::cout << "AltRegionsFactory::idd_start>" << std::endl; 
  } 
  IdDictAltRegions* altregions = new IdDictAltRegions;
  if (Debugger::debug ()) { 
      Debugger::tab (parser.level());
      std::cout << "AltRegionsFactory::idd_start> previous=" << parser.m_altregions
                << " new=" << altregions
                << std::endl; 
  } 
  parser.m_altregions = altregions;
}
  
void 
AltRegionsFactory::idd_end (IdDictParser& parser, const XMLCoreNode& /*node*/)  {  
  if (Debugger::debug ()) { 
      Debugger::tab (parser.level());
      std::cout << "AltRegionsFactory::idd_end>" << std::endl; 
  } 
  if (parser.m_altregions != 0) { 
    // Set altregions to point to the default region
    parser.m_altregions->select_region ("");

    // add to dict
    if (parser.m_dictionary != 0) parser.m_dictionary->add_dictentry (parser.m_altregions);  
    else delete parser.m_altregions; 
    // reset pointer to altregions
    parser.m_altregions = 0; 
  }
}  
  
void 
RegionFactory::idd_start (IdDictParser& parser, const XMLCoreNode& node)  {
  if (Debugger::debug ()) { 
      Debugger::tab (parser.level());
      std::cout << "RegionFactory::idd_start>" << std::endl; 
  } 
 
  parser.m_region = new IdDictRegion (get_value (node, "name"),
                                      get_value (node, "group"),
                                      get_value (node, "tag"));

  // check for next region in absolute eta
  if (has_attribute (node, "next_abs_eta")) { 
      parser.m_region->set_next_abs_eta_name (get_value (node, "next_abs_eta"));
  } 

  // Look for regions in the previous sampling
  if (has_attribute (node, "prev_samp")) { 
    std::string names = get_value (node, "prev_samp"); 
    std::string::size_type pos = 0; 
    for (;;) { 
      std::string::size_type next; 
      next = names.find_first_not_of (" ", pos); 
      if (next == std::string::npos) break; 
      pos = next; 
      next = names.find_first_of (" ", pos); 
      std::string name; 
      name = names.substr (pos, next - pos); 
      parser.m_region->add_prev_samp_name (name);
      if (next == std::string::npos) { 
        break; 
      } else {
        pos = next; 
      } 
    } 
  }
  // Look for regions in the next sampling
  if (has_attribute (node, "next_samp")) { 
    std::string names = get_value (node, "next_samp"); 
    std::string::size_type pos = 0; 
    for (;;) { 
        std::string::size_type next; 

        next = names.find_first_not_of (" ", pos); 
        if (next == std::string::npos) break; 
        pos = next; 
        next = names.find_first_of (" ", pos); 

        std::string name; 
        name = names.substr (pos, next - pos); 
        parser.m_region->add_next_samp_name (name);

        if (next == std::string::npos) { 
          break;
        } else {
          pos = next; 
      } 
    } 
  }
  // Look for regions in the prev subdet
  if (has_attribute (node, "prev_subdet")) { 
    std::string names = get_value (node, "prev_subdet"); 
    std::string::size_type pos = 0; 
    for (;;) { 
      std::string::size_type prev; 
      prev = names.find_first_not_of (" ", pos); 
      if (prev == std::string::npos) break; 
      pos = prev; 
      prev = names.find_first_of (" ", pos);
      std::string name; 
      name = names.substr (pos, prev - pos); 
      parser.m_region->add_prev_subdet_name (name);
      if (prev == std::string::npos) { 
        break; 
      } else {
        pos = prev; 
      } 
    } 
  }
  // Look for regions in the next subdet
  if (has_attribute (node, "next_subdet")) { 
    std::string names = get_value (node, "next_subdet"); 
    std::string::size_type pos = 0; 
    for (;;) { 
      std::string::size_type next; 
      next = names.find_first_not_of (" ", pos); 
      if (next == std::string::npos) break; 
      pos = next; 
      next = names.find_first_of (" ", pos); 
      std::string name; 
      name = names.substr (pos, next - pos); 
      parser.m_region->add_next_subdet_name (name);
      if (next == std::string::npos) { 
        break; 
      } else {
        pos = next; 
      } 
    } 
  }

  parser.m_region->set_etaphi (get_double (node, "eta0"),
                               get_double (node, "deta"),
                               get_double (node, "phi0"),
                               get_double (node, "dphi"));

  if (Debugger::debug ()) { 
    Debugger::tab (parser.level());
    std::cout << "RegionFactory::idd_start> name, group, tag, next eta, prev/next samp, prev/next subdet "
      << parser.m_region->name() << " "
      << parser.m_region->group_name() << " "
      << parser.m_region->tag() << " "
      << parser.m_region->next_abs_eta() << " ";
    for (const std::string& s : parser.m_region->prev_samp_names()) {
      std::cout << s << " ";
    }
    for (const std::string& s : parser.m_region->next_samp_names()) {
      std::cout << s << " ";
    }
    for (const std::string& s : parser.m_region->prev_subdet_names()) {
      std::cout << s << " ";
    }
    for (const std::string& s : parser.m_region->next_subdet_names()) {
      std::cout << s << " ";
    }
    std::cout << parser.m_region->eta0() << " "
      << parser.m_region->deta() << " "
      << parser.m_region->phi0() << " "
      << parser.m_region->dphi() << " "
      << std::endl; 
  } 
}
  
void 
RegionFactory::idd_end (IdDictParser& parser, const XMLCoreNode& /*node*/)  {  
  if (Debugger::debug ()){ 
    Debugger::tab (parser.level());
    std::cout << "RegionFactory::idd_end>" << std::endl; 
  } 
  if (parser.m_region != 0){ 
    if (parser.m_altregions != 0) {
      parser.m_altregions->add_region (parser.m_region);
      if (parser.m_dictionary != 0) parser.m_dictionary->add_region (parser.m_region);
      // Check whether region is empty, i.e. no region entries have
      // been found and added
      if (parser.m_region->entries().size() == 0) {
        parser.m_region->set_is_empty();
      }
    } else if (parser.m_dictionary != 0) {
      parser.m_dictionary->add_dictentry (parser.m_region);  
      parser.m_dictionary->add_region (parser.m_region);
      // Check whether region is empty, i.e. no region entries have
      // been found and added
      if (parser.m_region->entries().size() == 0) {
          parser.m_region->set_is_empty();
      }
    } else {
      delete parser.m_region; 
    }
    parser.m_region = 0; 
  } 
}  
  
void 
SubRegionFactory::idd_start (IdDictParser& parser, const XMLCoreNode& node)  {  
  if (Debugger::debug ()){ 
    Debugger::tab (parser.level());
    std::cout << "SubRegionFactory::idd_start>" << std::endl; 
  } 
  parser.m_subregion = new IdDictSubRegion (get_value (node, "name"), "", "");
} 
  
void 
SubRegionFactory::idd_end (IdDictParser& parser, const XMLCoreNode& /*node*/)  {  
  if (Debugger::debug ()) { 
    Debugger::tab (parser.level());
    std::cout << "SubRegionFactory::idd_end>" << std::endl; 
  } 
  if (parser.m_subregion != 0) { 
    if (parser.m_dictionary != 0) parser.m_dictionary->add_subregion (parser.m_subregion);  
    else delete parser.m_subregion; 
 
    parser.m_subregion = 0; 
  } 
}  
  
void 
RegionEntryFactory::idd_start (IdDictParser& parser, const XMLCoreNode& /*node*/)  {  
  if (Debugger::debug ()){ 
    Debugger::tab (parser.level());
    std::cout << "RegionEntryFactory::idd_start>" << std::endl; 
  } 
} 
 
void 
RegionEntryFactory::idd_end (IdDictParser& parser, const XMLCoreNode& /*node*/){  
  if (Debugger::debug ()) { 
    Debugger::tab (parser.level());
    std::cout << "RegionEntryFactory::idd_end>" << std::endl; 
  } 
  if (parser.m_regionentry != 0){ 
    if (parser.m_region != 0) parser.m_region->add_entry (parser.m_regionentry);  
    else if (parser.m_subregion != 0) parser.m_subregion->add_entry (parser.m_regionentry);  
    else delete parser.m_regionentry; 
    parser.m_regionentry = 0; 
  } 
}  
  
void 
RangeFactory::idd_start (IdDictParser& parser, const XMLCoreNode& node)  {  
  if (Debugger::debug ())  { 
    Debugger::tab (parser.level());
    std::cout << "RangeFactory::idd_start>" << std::endl; 
  } 
  IdDictRange* range = new IdDictRange (get_value (node, "field"));
  parser.m_regionentry = range; 
  if (has_attribute (node, "value")){
      range->set_range (get_value (node, "value"));
    } else if (has_attribute (node, "values")) { 
      std::string labels = get_value (node, "values"); 
      std::string::size_type pos = 0;
      std::vector<std::string> label_vec;
      for (;;){ 
        std::string::size_type next; 
        next = labels.find_first_not_of (" ", pos); 
        if (next == std::string::npos) break; 
        pos = next; 
        next = labels.find_first_of (" ", pos); 
        label_vec.push_back (labels.substr (pos, next - pos));
        if (next == std::string::npos) { 
          break; 
        } else  { 
            pos = next; 
        } 
      }
      range->set_range (label_vec);
    } else  { 
      const bool hasMin = has_attribute (node, "minvalue");
      const bool hasMax = has_attribute (node, "maxvalue");
      if (hasMin and hasMax) {
          range->set_range (get_int (node, "minvalue"),
                            get_int (node, "maxvalue"));
      }
      //falls through to a case where there is *no* attribute value, values, minvalue, maxvalue
      //https://its.cern.ch/jira/browse/ATLASSIM-7295
    } 
  if (has_attribute (node, "wraparound")){
    bool wraparound = get_boolean (node, "wraparound");
    if (wraparound) range->set_wrap_around();
  }
  if (has_attribute (node, "prev_value")){
    range->set_prev (get_int (node, "prev_value"));
  }
  if (has_attribute (node, "next_value")) {
    range->set_next (get_int (node, "next_value"));
  }
}  
  
void 
ReferenceFactory::idd_start (IdDictParser& parser, const XMLCoreNode& node)  {  
  if (Debugger::debug ()){ 
    Debugger::tab (parser.level());
    std::cout << "ReferenceFactory::idd_start>" << std::endl; 
  } 
  parser.m_regionentry = new IdDictReference(get_value (node, "subregion"));
}  
  
void 
DictionaryRefFactory::idd_start (IdDictParser& parser, const XMLCoreNode& node) {  
  if (Debugger::debug ())  { 
    Debugger::tab (parser.level());
    std::cout << "DictionaryRefFactory::idd_start>" << std::endl; 
  } 
  IdDictDictionaryRef* dictionaryref = new IdDictDictionaryRef (get_value (node, "name"));
  parser.m_regionentry = dictionaryref; 
  // Add dictionary name to subdictionaries
  if (dictionaryref->dictionary_name() != "") {
    parser.m_idd.add_subdictionary_name (dictionaryref->dictionary_name());
    if (parser.m_dictionary != 0) parser.m_dictionary->add_subdictionary_name (dictionaryref->dictionary_name());
  } 
}  
  
