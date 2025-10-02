/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictGroup.h"
#include "src/Debugger.h"
#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictAltRegions.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "Identifier/ExpandedIdentifier.h"
#include "Identifier/MultiRange.h"
#include "Identifier/Range.h"
#include "Identifier/RangeIterator.h"

#include <iostream>
#include <map>
#include <numeric> // for iota


IdDictGroup::IdDictGroup ()
  :
  m_generated_implementation(false) {
}

IdDictGroup::IdDictGroup (const std::string& name)
  :
  m_name(name),
  m_generated_implementation(false) {
}

IdDictGroup::~IdDictGroup () {
}

const std::string& IdDictGroup::name() {
  return(m_name);
}

const std::vector<IdDictDictEntry*>&
IdDictGroup::entries() {
  return(m_entries);
}

const std::vector<IdDictRegion*>&
IdDictGroup::regions() {
  return(m_regions);
}

MultiRange
IdDictGroup::build_multirange() const {
  MultiRange result;

  for (const IdDictRegion* region : m_regions) {

    // skip regions created from parents
    if ("dummy" == region->name()) continue;

    // skip empty regions - may arise from alternate_regions
    // where a tag selects an empty region
    if (region->is_empty()) continue;

    Range r = region->build_range();
    result.add(std::move(r));
  }

  return(result);
}

void
IdDictGroup::add_dictentry(IdDictDictEntry* region) {
  m_entries.push_back(region);
}

void
IdDictGroup::resolve_references(const IdDictMgr& idd,
                                IdDictDictionary& dictionary,
                                size_t& index) {
  for (IdDictDictEntry* ent : m_entries) {
    ent->set_index(index);
    index++;

    ent->resolve_references(idd, dictionary);
  }
}

void
IdDictGroup::generate_implementation(const IdDictMgr& idd,
                                     IdDictDictionary& dictionary,
                                     const std::string& tag) {
  if (Debugger::debug()) {
    std::cout << "IdDictGroup::generate_implementation>" << std::endl;
  }

  if (!m_generated_implementation) {
    // Loop over entries and fill regions vec with selected region
    // (AltRegions have a selection)
    for (IdDictDictEntry* ent : m_entries) {
      ent->generate_implementation(idd, dictionary, tag);
      // Get region and save in m_regions
      IdDictRegion* region = dynamic_cast<IdDictRegion*> (ent);
      if (region) {
        m_regions.push_back(region);
      } else {
        IdDictAltRegions* altregions = dynamic_cast<IdDictAltRegions*> (ent);
        if (altregions) {
          m_regions.push_back(altregions->selected_region());
        }
      }
    }

    if (m_regions.size() != m_entries.size()) {
      std::cout << "IdDictGroup::generate_implementation - mismatch of sizes: regions/entries "
                << m_regions.size() << " " << m_entries.size()
                << std::endl;
    }

    m_generated_implementation = true;
  }
}

void
IdDictGroup::reset_implementation() {
  if (m_generated_implementation) {
    m_regions.clear();
    for (IdDictDictEntry* ent : m_entries) {
      ent->reset_implementation();
    }
    m_generated_implementation = false;
  }
}

bool
IdDictGroup::verify() const {
  // Should check that all regions have the same number of levels,
  // which is part of the definition of a group
  return(true);
}

/**
 *
 *   Sort:
 *
 *      Loop over regions and sort according to their first identifier
 *
 **/

void IdDictGroup::sort() {
  std::map< ExpandedIdentifier, IdDictDictEntry* > regions;

  for (IdDictRegion* region : m_regions) {
    Range range = region->build_range();
    RangeIterator itr(range);
    auto first = itr.begin();
    auto last = itr.end();
    if (first != last) {
      regions[*first] = region;
    } else {
      std::cout << "IdDictDictionary::sort - WARNING empty region cannot sort "
                << std::endl;
    }
  }
  if (regions.size() == m_regions.size()) {
    // Reorder the regions
    std::map< ExpandedIdentifier, IdDictDictEntry* >::iterator mapIt = regions.begin();
    std::vector<IdDictRegion*>::size_type vecIt = 0;
    for (; mapIt != regions.end(); ++mapIt, ++vecIt) {
      m_entries[vecIt] = (*mapIt).second;
    }
  } else {
    std::cout << "IdDictGroup::sort - WARNING region map size is NOT the same as the vector size. Map size "
              << regions.size() << " vector size " << m_regions.size()
              << std::endl;
  }
}

void
IdDictGroup::clear() {
  for (IdDictDictEntry* region : m_entries) {
    region->clear();
    delete region;
  }

  m_entries.clear();
}


/// Constructor, taking a reference to the implementation
IdDictGroup::IdDictRegionTreeNode::IdDictRegionTreeNode
  (const IdDictFieldImplementation& impl)
    : m_impl (impl),
      m_children  (std::in_place_index<0>,
                   impl.ored_field().get_indices(), 0)
{
}


/// Compress the vector of node indices, if they are all the same.
void IdDictGroup::IdDictRegionTreeNode::optimize()
{
  if (m_children.index() == 0) {
    uint64_t c = 0;
    auto& children = std::get<0> (m_children);
    size_t ipos = children.size();
    while (ipos > 0 && children[ipos-1] == 0) {
      --ipos;
    }
    if (ipos > 0) {
      c = children[ipos-1];
      size_t ipos2 = ipos;
      while (ipos2 > 0 && children[ipos2-1] == c) {
        --ipos2;
      }
      if (ipos2 > 0) return;
    }
    m_children.emplace<1> (ipos, c);
  }
}


/**
 *  Unpack the value_type id to an expanded Identifier, considering
 *  the provided prefix (result will include the prefix) and up to
 *  index2 - (index1 is assumed to be 0, i.e. part of prefix).
 *  If impls is provided, then fill it with pointers to the implementations
 *  for each unpacked field.
 *
 *  Returns 0 on success, nonzero on error.
 */
int IdDictGroup::unpack(const Identifier& id,
                        const ExpandedIdentifier& prefix,
                        size_t index2,
                        ExpandedIdentifier& unpackedId,
                        std::vector<const IdDictFieldImplementation*>* impls /*= nullptr*/) const
{
  using element_type = ExpandedIdentifier::element_type;
  using size_type = IdentifierField::size_type;

  // Give up if the tree representation hasn't been built.
  if (m_region_tree.empty()) std::abort();

  // Clear output.
  unpackedId.clear();
  if (impls) {
    impls->clear();
    impls->reserve (12);
  }

  // Start at the first node, and assume success.
  unsigned inode = 0;
  int ret = 0;

  // Loop over fields.
  for (size_t index = 0; index <= index2; ++index) {

    // Fetch the node.
    const IdDictRegionTreeNode& n = m_region_tree.at(inode);

    // Find the index+value for this field.  If we're looking at a prefix,
    // get it from there; otherwise, unpack from the input identifier.
    element_type val;
    size_type validx;
    if (index < prefix.fields()) {
      val = prefix[index];
      validx = n.m_impl.ored_field().get_value_index (val);
    }
    else {
      validx = n.m_impl.unpackToIndex (id);
      try {
        val = n.m_impl.ored_field().get_value_at (validx);
      } catch (const std::out_of_range&) {
        ret = 1;
        break;
      }
    }

    // Find the next node.
    if (n.m_children.index() == 0) {
      // Children stored as a vector.
      const auto& children = std::get<0> (n.m_children);
      if (validx < children.size()) {
        inode = children[validx];
      }
      else {
        inode = 0;
      }
    }
    else {
      // Children stored as a size,node-number field.
      const auto& children = std::get<1> (n.m_children);
      if (validx < children.first) {
        inode = children.second;
      }
      else {
        inode = 0;
      }
    }

    // Give up if no regions match the identifier.
    if (!inode) {
      break;
    }

    // Record this field in the output.
    unpackedId.add (val);

    // Also return the implementation, if requested.
    // But we need to be sure that we return an implementation that actually
    // matches the field value.
    if (impls) {
      if (n.m_impl.field().match (val)) {
        impls->push_back (&n.m_impl);
      }
      else if (n.m_other_impls) {
        for (const IdDictFieldImplementation* ii : *n.m_other_impls) {
          if (ii->field().match (val)) {
            impls->push_back (ii);
            break;
          }
        }
      }
      if (unpackedId.fields() != impls->size()) std::abort();
    }

    // Stop if we've reached the end of the identifier.
    if (inode == IdDictRegionTreeNode::END) break;
  }

  return ret;
}


/**
 * @brief Recursively add new nodes to the tree structure.
 * @param re The region we're processing.
 * @param ifield The index of the next field to add.
 * @param inode The index of the node to which to add.
 */
void IdDictGroup::add_tree_field (const IdDictRegion& re,
                                  unsigned ifield,
                                  unsigned inode)
{
  using element_type   = IdentifierField::element_type;
  using element_vector = IdentifierField::element_vector;
  using size_type      = IdentifierField::size_type;
  using index_vector   = IdentifierField::index_vector;

  // Helper to retrieve the indices that may actually be used by a field.
  // n is the node we're looking for.  We use the ored_field referenced there
  // to know what can actually be stored in the field.
  // impl gives the field that we're trying to match.
  // So we return the indices for the ored_field corresponding
  // to valid values for impl.
  auto get_field_indices = [] (const IdDictRegionTreeNode& n,
                               const IdDictFieldImplementation& impl)
  {
    index_vector indices;
    const Range::field& ored_field = n.m_impl.ored_field();
    if (impl.field().isEnumerated()) {
      const element_vector& vals = impl.field().get_values();
      indices.reserve (vals.size());
      for (element_type v : vals) {
        indices.push_back (ored_field.get_value_index (v));
      }
    }
    else if (impl.field().isBounded()) {
      auto [minval, maxval] = impl.field().get_minmax();
      size_type minidx = ored_field.get_value_index (minval);
      size_type maxidx = ored_field.get_value_index (maxval);
      indices.resize (maxidx - minidx + 1);
      std::iota (indices.begin(), indices.end(), minidx);
    }
    else {
      std::abort();
    }
    return indices;
  };

  // This will be the field we're matching.
  const IdDictFieldImplementation& prev_impl = re.implementation(ifield-1);

  {
    // Get the children for the current node.
    IdDictRegionTreeNode& n = m_region_tree.at(inode);
    auto& children = std::get<0> (n.m_children);

    // If we're looking at the last field, fill in the node pointersj
    // with END; then we're done.
    if (ifield == re.n_implementation()) {
      index_vector indices = get_field_indices (n, prev_impl);
      for (size_t idx : indices) {
        children.at (idx) = IdDictRegionTreeNode::END;
      }
      return;
    }

    // Verify consistency of the field with what's stored in the node.
    if (prev_impl.bits() != n.m_impl.bits() ||
        prev_impl.bits_offset() != n.m_impl.bits_offset() ||
        prev_impl.ored_field() != n.m_impl.ored_field())
    {
      dump();
      std::abort();
    }

    // If this implementation has a field that is not equivalent with what
    // we saved in the node, then save it in m_other_impls.
    if (n.m_impl.field() != prev_impl.field()) {
      if (!n.m_other_impls) {
        n.m_other_impls = std::make_unique<std::vector<const IdDictFieldImplementation*> > (1, &prev_impl);
      }
      else {
        if (std::ranges::find_if (*n.m_other_impls,
                                  [&](const IdDictFieldImplementation* a)
                                  { return a->field() == prev_impl.field(); })
            == n.m_other_impls->end())
        {
          n.m_other_impls->push_back (&prev_impl);
        }
      }
    }
  }

  // Indices we want to store.
  index_vector indices = get_field_indices (m_region_tree[inode], prev_impl);
  const IdDictFieldImplementation& impl = re.implementation(ifield);

  // Children for the node.
  auto children = [&]() -> std::vector<unsigned>& { return std::get<0> (m_region_tree[inode].m_children); };

  unsigned new_node = 0;
  std::vector<unsigned> nodes_seen;

  // Loop over indices that we want to add.
  for (size_t idx : indices) {

    // If we've already recorded this index as END, fail.
    unsigned next_node = children().at (idx);
    if (next_node == IdDictRegionTreeNode::END) {
      dump();
      std::abort();
    }

    if (next_node != 0) {
      // There is an existing node.  Process it recursively; but we only
      // need to this once for each unique node.
      if (std::ranges::find (nodes_seen, next_node) == nodes_seen.end()) {
        add_tree_field (re, ifield+1, next_node);
        nodes_seen.push_back (next_node);
      }
    }
    else {
      // No node had been recorded for this index.
      if (new_node != 0) {
        // We've already made a new node.  So just record it.
        children().at(idx) = new_node;
      }
      else {
        // Need to make a new node.
        new_node = m_region_tree.size();
        if (new_node == IdDictRegionTreeNode::END) {
          std::abort();
        }
        // Careful --- this will invalidate references to nodes.
        m_region_tree.emplace_back (impl);
        children().at(idx) = new_node;
        add_tree_field (re, ifield+1, new_node);
      }
    }
  }
}


/**
 * Take the list of regions and build a tree structure for fast unpacking.
 */
void
IdDictGroup::build_region_tree()
{
  // Loop through regions.
  [[maybe_unused]] unsigned iregion = 0; // Region index, really only for debugging.
  for (const IdDictRegion* re : m_regions) {
    // Skip dummy/empty regions.
    if (re->fieldSize() == 0 || re->name() == "dummy") {
      ++iregion;
      continue;
    }

    // Add an initial node if we haven't done so already.
    if (m_region_tree.empty()) {
      m_region_tree.emplace_back (re->implementation(0));
    }

    // Add nodes for the current region, starting at field 1.
    add_tree_field (*re, 1, 0);

    ++iregion;
  }

  // Compress child vectors if possible.
  for (IdDictRegionTreeNode& n : m_region_tree) {
    n.optimize();
  }
}


/**
 * Dump regions and tree for this group.
 */
void IdDictGroup::dump() const
{
  std::cout << "===== IdDictGroup " << m_name << "\n";
  dump_regions();
  dump_tree();
}


/**
 * Dump the list of regions for this group.
 */
void IdDictGroup::dump_regions() const
{
  std::cout << "Regions:\n";
  for (unsigned iregion = 0; const IdDictRegion* re : m_regions) {
    std::cout << "  " << iregion++ << " " << re->name() << " " << re->group_name() << " " << re->tag() << "\n";
    size_t nimpl = re->n_implementation();
    bool first = true;
    for (size_t i = 0; i < nimpl; ++i) {
      const IdDictFieldImplementation& impl = re->implementation(i);
      std::cout << (first ? "    " : "; ") << impl.field() << " " << impl.ored_field() << " " << impl.bits() << "/" << impl.bits_offset();
      first = false;
    }
    std::cout << "\n";
  }
}


/**
 * Dump the tree structure built from the regions for fast unpacking.
 */
void IdDictGroup::dump_tree() const
{
  size_t sz = 0;
  for (const auto& n : m_region_tree) {
    if (n.m_children.index() == 0) {
      sz += std::get<0>(n.m_children).size();
    }
  }
  std::cout << "Region Tree totsize " << sz << "\n";
  for (unsigned inode = 0; const auto& n : m_region_tree) {
    std::cout << "  " << inode++ << " " << n.m_impl.field()
              << " " << n.m_impl.ored_field() << " -- ";

    if (n.m_children.index() == 0) {
      const auto& children = std::get<0> (n.m_children);
      unsigned istart = 0;
      unsigned ichild = 0;
      bool first = true;
      for (size_t i = 0; uint64_t c : children) {
        if (c != ichild) {
          if (ichild != 0) {
            if (!first) std::cout << " ";
            first = false;
            if (istart != i-1) {
              std::cout << istart << "-";
            }
            std::cout << i-1 << ":";
            if (ichild == IdDictRegionTreeNode::END) {
              std::cout << "END";
            }
            else {
              std::cout << ichild;
            }
          }
          ichild = c;
          istart = i;
        }
        ++i;
      }
      if (ichild != 0) {
        if (!first) std::cout << " ";
        if (istart != children.size()-1) {
          std::cout << istart << "-";
        }
        std::cout << children.size()-1 << ":";
        if (ichild == IdDictRegionTreeNode::END) {
          std::cout << "END";
        }
        else {
          std::cout << ichild;
        }
      }
    }
    else {
      const auto& children = std::get<1> (n.m_children);
      if (children.first == 1) {
        std::cout << "0:";
      }
      else {
        std::cout << "0-" << children.first-1 << ":";
      }
      if (children.second == IdDictRegionTreeNode::END) {
        std::cout << "END";
      }
      else {
        std::cout << children.second;
      }
    }
    if (n.m_other_impls) {
      std::cout << " [";
      bool first = true;
      for (const IdDictFieldImplementation* ii : *n.m_other_impls) {
        if (!first)
          std::cout << "; ";
        else
          first = false;
        std::cout << ii->field();
      }
      std::cout << "]";
    }
    std::cout << "\n";
  }
  std::cout.flush();
}
