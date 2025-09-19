/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictDictionary_H
#define IDDICT_IdDictDictionary_H


#include "Identifier/Identifier.h" //typedef
#include "Identifier/Range.h" //member
#include "Identifier/ExpandedIdentifier.h"

#include <vector>
#include <string>


class IdDictField;
struct IdDictLabel;
class IdDictSubRegion;
class IdDictRegion;
class IdDictGroup;
class IdDictMgr;

class MultiRange;
class ExpandedIdentifier;
class IdDictDictEntry;



class IdDictDictionary  {  
public:  
    using value_type = Identifier::value_type;
    using size_type  = Identifier::size_type;

    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictDictionary ();  
    ~IdDictDictionary ();  


    //@}
    // ==================================
    //** @name Simple accessors.
    // @{

    /// Dictionary name
    const std::string& name() const;

    /// Dictionary version
    const std::string& version() const;

    /// Number of contained regions.
    size_t n_regions() const;

    /// Region at index i.
    const IdDictRegion& region(size_t i) const;

    /// Access to file name
    const std::string&  file_name() const;

    /// Access to the dictionary tag
    const std::string&  dict_tag() const;


    //@}
    // ==================================
    //** @name Search for field/label/region by name.
    // @{

    IdDictField* find_field (const std::string& name) const;  
    IdDictLabel* find_label (const std::string& field, const std::string& label) const;
    int get_label_value (const std::string& field, const std::string& label, int& value) const;  // > 0 == error
    IdDictSubRegion* find_subregion (const std::string& subregion_name) const;  
    IdDictRegion* find_region (const std::string& region_name) const;  
    IdDictRegion* find_region (const std::string& region_name, const std::string& group_name) const;  
    IdDictGroup* find_group (const std::string& group_name) const;


    //@}
    // ==================================
    //** @name Find first region that matches id
    // @{

    int find_region(const ExpandedIdentifier& id, size_type& index) const;
    IdDictRegion* find_region(const ExpandedIdentifier& id) const;
    IdDictRegion* find_region(const ExpandedIdentifier& id,const std::string& group_name) const;


    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    void add_field (IdDictField* field);  
    void add_subregion (IdDictSubRegion* subregion);  
    void add_dictentry (IdDictDictEntry* entry);
    void add_subdictionary_name (const std::string& name);  

    /// Set file name
    void                set_file_name    (const std::string& name);

    /// Set the dictionary tag
    void                set_dict_tag     (const std::string& tag);

    void resolve_references (const IdDictMgr& idd);  
    void generate_implementation (const IdDictMgr& idd, const std::string& tag = "");  
    void reset_implementation ();  
    bool verify   () const;
    void sort     ();
    void clear    ();

    ///  Set up integral of bits for efficient unpacking
    void integrate_bits ();


    //@}
    // ==================================
    //** @name Range building
    // @{

    ///  Get MultiRange for full dictionary
    MultiRange build_multirange () const; 

    /**
     *   Get MultiRange for a specific region_id up to and including
     *   'last_field' If last_field == "", all fields are
     * taken. Prepend prefix if provided.  */
    MultiRange build_multirange (const ExpandedIdentifier& region_id,
                                 const Range& prefix = Range(),
                                 const std::string& last_field = "") const; 

    /**
     *   Get MultiRange for a specific region_id, constrained to be
     *   within a specific named group. The depth of the multirange is
     *   up to and including 'last_field' If last_field == "", all
     *   fields are taken. Prepend prefix if provided.  */

    MultiRange build_multirange (const ExpandedIdentifier& region_id,
                                 const std::string& group_name,
                                 const Range& prefix = Range(),
                                 const std::string& last_field = "") const; 


    //@}
    // ==================================
    //** @name Packing and unpacking
    // @{

    /** 
     *   Pack to 32bits the subset of id between (inclusive) index1
     *   and index2 - this is generic, i.e. not the most efficient
     *
     *   Assumptions: 
     *     - packedId is initialized to 0
     *     - the expanded id begins at index = 0
     *     - index1,2 correspond to the level/field number, this is
     *       thus the same as the index into the expanded id due to the
     *       previous assumption.
     *
     */
    int pack32 (const ExpandedIdentifier& id, 
                size_t index1, 
                size_t index2,
                Identifier& packedId) const; 
 
    /** 
     *   Pack to 32bits the subset of id between (inclusive) index1
     *   and index2. Here one must provide the region index, which can
     *   be found with find_region. This is more efficient than the
     *   above pack32.
     *
     *   Assumptions: (different than the previous pack32)
     *     - packedId is NOT initialized to 0 - allows to just
     *       overwrite the specified fields
     *     - the field array begins at index1, i.e. NOT the first field
     *     - the field array field array has a length of index2-index1+1 
     *     - index1,2 correspond to the level/field number
     *     - first_field_index defines the offset for
     *       packing. Normally, this is this is 0. But this may be a
     *       field > 0, but of course <= index1
     *
     * */
    int pack32 (const int* fields, 
                size_t index1, 
                size_t index2,
                size_t region_index,
                Identifier& packedId,
                size_t first_field_index=0) const; 
 
    /** 
     *   Reset fields from index1 to index2.
     * */
    int reset (size_t index1, 
               size_t index2,
               size_t region_index,
               Identifier& packedId) const; 
 


    /** 
     *  Unpack the value_type id to an expanded Identifier for a given group,
     *  considering the provided prefix (result will include the prefix)
     *  and up to index2 - (index1 is assumed to be 0, i.e. part of prefix).
     *
     *  Returns 0 on success, nonzero on error.
     */
    int unpack (const std::string& group,
                const Identifier& id,
                const ExpandedIdentifier& prefix,
                size_t index2,
                ExpandedIdentifier& unpackedId) const;
   
    /** 
     *  Unpack the value_type id to a string for a given group,
     *  considering the provided prefix (result will include the prefix)
     *  and up to index2 - (index1 is assumed to be 0, i.e. part of prefix).
     *
     *  Returns 0 on success, nonzero on error.
     */
    int unpack (const std::string& group,
                const Identifier& id,
                const ExpandedIdentifier& prefix,
                size_t index2,
                const std::string& sep,
                std::string& unpackedId) const; 
   
    /** 
     **  Unpack a single field of the value_type id. One specifies the
     **  field index desired and passes in the region index to be used
     **  for decoding. The region index is obtained from
     **  find_region. The first_field_index is normally 0. It is
     **  non-zero when fields 0 to first_field_index -1 are missing
     **  from the compact 
     */
    int unpack (const Identifier& id, 
                size_t first_field_index, 
                size_t field_index,
                size_t region_index,
                int& field) const; 
 
   
    /** 
     **  Copy a number of fields of the value_type id into another value_type
     **  id. One specifies the begin and end (inclusive) field indexes
     **  desired and passes in the region index to be used for
     **  decoding. The region index is obtained from find_region.  The
     **  first_field_index is normally 0. It is non-zero when fields 0
     **  to first_field_index -1 are missing from the compact 
     */
    int copy (const Identifier& idin, 
              size_t first_field_index, 
              size_t begin_field_index,
              size_t end_field_index,
              size_t region_index,
              Identifier& idout) const; 
 

    //@}
    // ==================================
    //** @name Configuration and diagnostics
    // @{

    /// Checks are performed by default in debug compilation and NOT
    /// in optimized compilation. One can switch or query this mode for
    /// any idHelper with the following methods:
    bool                do_checks       () const;
    void                set_do_checks   (bool do_checks);

    /// Neighbour initialization is performed by default
    /// One can switch or query this mode for
    /// any idHelper with the following methods:
    bool                do_neighbours           () const;
    void                set_do_neighbours       (bool do_neighbours);

    /// Dump regions and trees for each group.
    void dump() const;


    //@}

    std::string m_name{};
    std::string m_version{};  
    std::string m_date{};  
    std::string m_author{};  
  
    typedef std::vector<IdDictDictEntry*> entries_type;
    typedef entries_type::iterator        entries_it;
    typedef entries_type::const_iterator  entries_const_it;

    typedef std::vector<IdDictRegion*>    regions_type;
    typedef regions_type::iterator        regions_it;
    typedef regions_type::const_iterator  regions_const_it;

    typedef std::vector<IdDictGroup*>     groups_type;
    typedef groups_type::iterator         groups_it;
    typedef groups_type::const_iterator   groups_const_it;

    std::map<std::string, IdDictField*>   m_fields;  
    std::map<std::string, IdDictSubRegion*> m_subregions;  
    std::vector<IdDictRegion*>            m_regions;  // corresponding regions for REs
    std::vector<IdDictRegion*>            m_all_regions;  // all regions
    std::vector<IdDictGroup*>             m_groups;    
    std::vector<std::string>              m_subdictionary_names; 
    IdDictDictionary*                     m_parent_dict{nullptr};

private:
    std::string m_file_name{};  
    std::string m_dict_tag{};  
    bool m_generated_implementation{false};
    bool  m_do_checks{false};
    bool  m_do_neighbours{true};
};

//-------------------
// inline definitions
//-------------------

// Dictionary name.
inline
const std::string&
IdDictDictionary::name() const
{
    return m_name;
}


// Dictionary version.
inline
const std::string&
IdDictDictionary::version() const
{
    return m_version;
}


// Access region by index.
inline
const IdDictRegion&
IdDictDictionary::region(size_t i) const
{
    return *m_regions.at(i);
}


// Number of regions
inline
size_t
IdDictDictionary::n_regions() const
{
    return m_regions.size();
}


// Access to file name
inline const std::string&
IdDictDictionary::file_name() const{
    return m_file_name;
}

    
/// Access to the dictionary tag
inline const std::string&
IdDictDictionary::dict_tag( ) const{
    return m_dict_tag;
}


/// Set file name
inline void                
IdDictDictionary::set_file_name(const std::string& name){
    m_file_name = name;
}

   
/// Set the dictionary tag
inline void                
IdDictDictionary::set_dict_tag(const std::string& tag){
    m_dict_tag = tag;
}


#endif
