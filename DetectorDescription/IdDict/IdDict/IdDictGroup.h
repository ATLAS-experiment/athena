/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictGroup_H
#define IDDICT_IdDictGroup_H

#include <string>
#include <vector>
#include <variant>
#include <utility>
#include <memory>

class IdDictDictEntry;
class IdDictRegion;
class IdDictMgr;
class IdDictDictionary;
class IdDictFieldImplementation;
class Identifier;
class ExpandedIdentifier;
class MultiRange;


 
class IdDictGroup{ 
public: 
    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictGroup (); 
    IdDictGroup (const std::string& name); 
    ~IdDictGroup (); 


    //@}
    // ==================================
    //** @name Simple accessors.
    // @{

    const std::string&  name();
    const std::vector<IdDictDictEntry*>& entries();
    const std::vector<IdDictRegion*>&    regions();
    size_t n_regions() const;
    const IdDictRegion&  region(size_t index) const;


    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{
    ///  Get MultiRange for this group

    MultiRange build_multirange () const; 
    void add_dictentry (IdDictDictEntry* entry);

    void resolve_references (const IdDictMgr& idd,  
                             IdDictDictionary& dictionary,
                             size_t& index);  
    void generate_implementation (const IdDictMgr& idd,  
                                  IdDictDictionary& dictionary, 
                                  const std::string& tag = "");
    void reset_implementation ();  
    bool verify () const;
    void sort   ();  
    void clear  (); 


    /**
     *  Unpack the value_type id to an expanded Identifier, considering
     *  the provided prefix (result will include the prefix) and up to
     *  index2 - (index1 is assumed to be 0, i.e. part of prefix).
     *  If impls is provided, then fill it with pointers to the implementations
     *  for each unpacked field.
     *
     *  Returns 0 on success, nonzero on error.
     */
    int unpack (const Identifier& id,
                const ExpandedIdentifier& prefix,
                size_t index2,
                ExpandedIdentifier& unpackedId,
                std::vector<const IdDictFieldImplementation*>* impls = nullptr) const;

    /**
     * Take the list of regions and build a tree structure for fast unpacking.
     */
    void build_region_tree();


    /**
     * Dump regions and tree for this group.
     */
    void dump() const;


    /**
     * Dump the list of regions for this group.
     */
    void dump_regions() const;


    /**
     * Dump the tree structure built from the regions for fast unpacking.
     */
    void dump_tree() const;


private:
    /**
     * @brief Recursively add new nodes to the tree structure.
     * @param re The region we're processing.
     * @param ifield The index of the next field to add.
     * @param inode The index of the node to which to add.
     */
    void add_tree_field (const IdDictRegion& re, unsigned ifield, unsigned inode);

    std::string                   m_name;  
    std::vector<IdDictDictEntry*> m_entries;  // just the RegionEntries
    std::vector<IdDictRegion*>    m_regions;  // regions derived from entries
    bool m_generated_implementation;


    /**
     * Tree structure for fast unpacking.
     *
     * A group is defined by a coherent list of regions; each region is
     * a set of constrained fields.  To unpack an Identifier into
     * an ExpandedIdentifier, we notionally loop through the regions, try to
     * match each in turn with the Identifier, filling
     * in the ExpandedIdentifier, and then return it once we've matched
     * all the fields.  However, that entails a significant amount of
     * redundant matching, and scales as O(nregions).
     *
     * We can instead organize the regions into a tree structure, where
     * we unpack each field exactly once, and use it's value to constrain
     * the possibilities for the remaining fields.  This removes
     * the redundant unpacking, and scales as O(log(nregions)).
     *
     * The tree is structured as vector of nodes.
     * For each node in the tree, we save a reference to the
     * IdDictFieldImplementation showing how to unpack that field.
     * Then each node has a vector indexed by the index unpacked from
     * the field giving the number of the next node to examine,
     * or the special values END or 0.  (To save a little memory, if all
     * elements in this vector point at the same node, we can replace
     * it with with a pair giving this value and the length of the vector.)
     *
     * So, for example, suppose we have these regions:
     *
     *   3/1,3,5,6  -1,1/-4:4   0:31/0:31   0:63/0:63
     *   3/1,3,5,6  -2,2/-4:4   0:7 /0:31   0:31/0:63
     *   3/1,3,5,6  -2,2/-4:4   8:15/0:31   0:15/0:63
     *
     * Notation:
     *   3/1,3,5,6 - an enumerated field with possible values 1,3,5,6 that
     *               here must take value 3 (index 1).
     *   -2,2/-4:4 - a field that can store values from -4 to 4, that here
     *               must take values -2 or 2.
     *   0:7 /0:31 - a field that can store values from 0 to 31, that here
     *               must take a value from 0 to 7.
     *
     * We can represent these regions as a tree like this:
     *
     *   0  /1,3,5, 6   [0, 1, 0, 0]
     *   1: /-4:4       [0, 0, 4, 2, 2, 4, 0 0]
     *   2: /0:31       32*[3]
     *   3: /0:63       64*[END]
     *   4  /0:31       8*[5] + 8*[6]
     *   5: /0:63       32*[END]
     *   6: /0:63       16*[END]
     *
     * The structure below defines the type of a node, and the nodes
     * are stored in m_region_tree.
     *
     * In addition, for a given identifier field, there may be multiple
     * Range::field structures that it can contain.  Normally this doesn't
     * matter, but if we're unpacking to a string, then we need to find
     * the proper field in order to get labels represented correctly.
     * In most cases, though, there is only one field.
     * So the m_impl field below references the first implementation
     * that we see.  If we encounter additional implementations
     * with not-equivalent fields, we save these in m_other_impls.
     */
    struct IdDictRegionTreeNode {
      /// Constructor, taking a reference to the implementation
      IdDictRegionTreeNode (const IdDictFieldImplementation& impl);
      /// Compress the vector of node indices, if they are all the same.
      void optimize();

      /// Reference to the the implementation field.
      const IdDictFieldImplementation& m_impl;

      /// Implementations for other field descriptions that this
      /// ored field of the identifier may contain.
      /// This is usually empty, which is why we separate it from m_impl
      /// above and hold it via a unique_ptr.
      std::unique_ptr<std::vector<const IdDictFieldImplementation*> > m_other_impls;

      /// References to child nodes.  The special value 0 means that there
      /// there is no valid identifier on that path, and END means that
      /// we have some to the end of an identifier.
      /// This can be stored in two ways.
      /// First, simply as a vector of child node numbers, indexed by
      /// the unpacked field index.  Or if all the node numbers are the same
      /// up to some point (and all 0's after that), we can store it as a pair
      /// of the number of non-zero values plus the value itself.
      using ChildPair = std::pair<size_t, unsigned>;
      std::variant<std::vector<unsigned>, ChildPair> m_children;

      /// Special value used to indicate that we've reached the end
      // of an identifier.
      static constexpr unsigned END = static_cast<unsigned> (-1);
    };
    /// The list of region nodes.
    std::vector<IdDictRegionTreeNode> m_region_tree;
}; 


inline
size_t IdDictGroup::n_regions() const
{
    return m_regions.size();
}


inline
const IdDictRegion& IdDictGroup::region(size_t index) const
{
    return *m_regions.at(index);
}


#endif
