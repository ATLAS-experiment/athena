/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictDictionary.h"
#include "Identifier/RangeIterator.h"
#include "Identifier/MultiRange.h"
#include "src/Debugger.h"
#include <fstream>
#include <iostream>

#include <stdio.h>
#include <cstdlib>
#include <atomic>
#include <stdexcept>
#include <format>


class TypeIdBuilder
{
public:
  static int build_id();
};

template <class T>
class TypeId
{
public:
  operator int () {
    static const int i = TypeIdBuilder::build_id();

    return(i);
  }
};

int TypeIdBuilder::build_id() {
  static std::atomic<int> i = 0;

  i++;
  return(i);
}

IdDictMgr::IdDictMgr()
  :
  m_resolved_references(false),
  m_generated_implementation(false),
  m_do_checks(false),
  m_do_neighbours(true) {
}

IdDictMgr::~IdDictMgr() = default;

const std::string& IdDictMgr::tag() const {
  return m_tag;
}

const std::string& IdDictMgr::DTD_version() const {
  return m_DTD_version;
}

bool IdDictMgr::do_checks() const {
  return m_do_checks;
}

void
IdDictMgr::set_do_checks(bool do_checks) {
  m_do_checks = do_checks;
  for (const auto& p : m_dictionaries) {
    p.second->set_do_checks(do_checks);
  }
}

bool IdDictMgr::do_neighbours() const {
  return m_do_neighbours;
}

void
IdDictMgr::set_do_neighbours(bool do_neighbours) {
  m_do_neighbours = do_neighbours;
  for (const auto& p : m_dictionaries) {
    p.second->set_do_neighbours(do_neighbours);
  }
}

const std::string&
IdDictMgr::find_metadata(const std::string& name) const {
  metadata_map::const_iterator it = m_metadata.find(name);
  static const std::string empty;
  if (it != m_metadata.end()) return(it->second);
  else return empty;
}

void
IdDictMgr::add_metadata(const std::string& name, const std::string& value) {
  if (!m_metadata.insert(metadata_map::value_type(name, value)).second) {
    std::cout << "IdDictMgr::add_metadata> unable to add name/value " << name << "/" << value << std::endl;
  }
}

void
IdDictMgr::set_DTD_version(const std::string& DTD_version) {
  m_DTD_version = DTD_version;
}

std::vector<const IdDictDictionary*> IdDictMgr::get_dictionaries () const
{
  std::vector<const IdDictDictionary*> out;
  out.reserve (m_dictionaries.size());
  for (const auto& p : m_dictionaries) {
    out.push_back (p.second.get());
  }
  return out;
}

const IdDictDictionary* IdDictMgr::find_dictionary(const std::string& name) const {
  auto it = m_dictionaries.find(name);
  if (it == m_dictionaries.end()) return nullptr;
  return it->second.get();
}

IdDictDictionary* IdDictMgr::find_dictionary(const std::string& name) {
  auto it = m_dictionaries.find(name);
  if (it == m_dictionaries.end()) return nullptr;
  return it->second.get();
}

void IdDictMgr::add_dictionary(std::unique_ptr<IdDictDictionary> dictionary) {
  if (dictionary == 0) return;

  const std::string& name = dictionary->name();

  m_dictionaries[name] = std::move(dictionary);

  if (Debugger::debug()) {
    dictionary_map::iterator it;

    for (const auto& p : m_dictionaries) {
      std::cout << "IdDictMgr::add_dictionary> d[" << p.first << "]=" << p.second.get() << std::endl;
    }
  }
}

void IdDictMgr::add_subdictionary_name(const std::string& name) {
  m_subdictionary_names.insert(name);
}

void IdDictMgr::resolve_references() {
  for (auto& p : m_dictionaries) {
    // From mgr, only resolve refs for top-level dictionaries
    IdDictDictionary& dictionary = *p.second;
    if (m_subdictionary_names.find(dictionary.name()) != m_subdictionary_names.end()) continue;
    dictionary.resolve_references(*this);
  }
}

void IdDictMgr::generate_implementation(const std::string& tag) {
  if (Debugger::debug()) {
    std::cout << "IdDictMgr::generate_implementation>" << std::endl;
  }

  // Must reset the implementation for multiple passes, this resets
  // the generated flags
  if (m_generated_implementation && tag != m_tag) reset_implementation();

  if (!m_generated_implementation) {
    m_tag = tag;
    for (auto& p : m_dictionaries) {
      // From mgr, only generate impl for top-level dictionaries
      IdDictDictionary& dictionary = *p.second;
      if (m_subdictionary_names.find(dictionary.name()) != m_subdictionary_names.end()) continue;
      dictionary.generate_implementation(*this, tag);
    }
    m_generated_implementation = true;
  }
}

void IdDictMgr::reset_implementation() {
  std::cout << "IdDictMgr::reset_implementation" << std::endl;


  if (m_generated_implementation) {
    for (auto& p : m_dictionaries) {
      // From mgr, only generate impl for top-level dictionaries
      IdDictDictionary& dictionary = *p.second;
      if (m_subdictionary_names.find(dictionary.name()) != m_subdictionary_names.end()) continue;
      dictionary.reset_implementation();
    }
    m_generated_implementation = false;
  }
}

bool IdDictMgr::verify() const {
  for (auto& p : m_dictionaries) {
    if (!p.second->verify()) return(false);
  }

  return(true);
}

void IdDictMgr::clear() {
  m_dictionaries.clear();
  m_resolved_references = false;
  m_generated_implementation = false;
}
