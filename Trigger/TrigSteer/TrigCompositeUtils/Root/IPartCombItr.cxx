/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCompositeUtils/IPartCombItr.h"

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include <set>
#include <ostream>

namespace {
  using namespace TrigCompositeUtils;
  template <typename T>
  std::ostream &operator<<(std::ostream &os, const LinkInfo<T> &info)
  {
      return os << info.source << ", (" << info.link.persKey() << ", " << info.link.persIndex() << ")";
  }

  template <typename T>
  std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
  {
    os << "[";
    if (v.size() > 0)
    {
      for (auto itr = v.begin(); itr != v.end() - 1; ++itr)
        os << *itr << ", ";
      os << v.back();
    }
    return os << "]";
  }
}

namespace TrigCompositeUtils
{
  bool uniqueObjects(const std::vector<LinkInfo<xAOD::IParticleContainer>> &links)
  {
    std::set<const xAOD::IParticle *> seen;
    for (const auto &info : links)
      if (!seen.insert(*info.link).second)
        return false;
    return true;
  }

  bool uniqueInitialRoIs(const std::vector<LinkInfo<xAOD::IParticleContainer>> &links)
  {
    std::set<std::pair<uint32_t, uint32_t>> seen;
    for (const auto &info : links)
    {
      LinkInfo<TrigRoiDescriptorCollection> roi = findLink<TrigRoiDescriptorCollection>(info.source, initialRoIString());
      if (!seen.insert(std::make_pair(roi.link.persKey(), roi.link.persIndex())).second)
        // Insert returns false if that item already exists in it
        return false;
    }
    return true;
  }

  bool uniqueRoIs(const std::vector<LinkInfo<xAOD::IParticleContainer>> &links)
  {
    std::set<std::pair<uint32_t, uint32_t>> seen;
    for (const auto &info : links)
    {
      LinkInfo<TrigRoiDescriptorCollection> roi = findLink<TrigRoiDescriptorCollection>(info.source, "roi");
      if (!seen.insert(std::make_pair(roi.link.persKey(), roi.link.persIndex())).second)
        // Insert returns false if that item already exists in it
        return false;
    }
    return true;
  }

  std::function<bool(const std::vector<LinkInfo<xAOD::IParticleContainer>> &)> getFilter(FilterType filter)
  {
    switch (filter){
    case FilterType::All:
      return [](const std::vector<LinkInfo<xAOD::IParticleContainer>> &) { return true; };
    case FilterType::UniqueObjects:
      return uniqueObjects;
    case FilterType::UniqueRoIs:
      return uniqueRoIs;
    default:
      throw std::runtime_error("Unhandled FilterType enum value!");
    }
  }

  IPartCombItr::IPartCombItr() {}

  IPartCombItr::IPartCombItr(
      const std::vector<std::tuple<std::size_t, LInfoItr_t, LInfoItr_t>> &pieces,
      std::function<bool(const VecLInfo_t &)> filter)
      : m_filter(std::move(filter))
  {
    std::vector<KFromNItr> idxItrs;
    idxItrs.reserve(pieces.size());
    m_linkInfoItrs.reserve(pieces.size());
    std::size_t size = 0;
    for (const auto &tup : pieces)
    {
      std::size_t multiplicity = std::get<0>(tup);
      LInfoItr_t begin = std::get<1>(tup);
      LInfoItr_t end = std::get<2>(tup);
      idxItrs.emplace_back(multiplicity, std::distance(begin, end));
      m_linkInfoItrs.push_back(begin);
      size += multiplicity;
    }
    m_idxItr = ProductItr<KFromNItr>(idxItrs, std::vector<KFromNItr>(idxItrs.size()));
    m_current.assign(size, {});
    readCurrent();
  }

  IPartCombItr::IPartCombItr(
      const std::vector<std::tuple<std::size_t, LInfoItr_t, LInfoItr_t>> &pieces,
      FilterType filter)
      : IPartCombItr(pieces, getFilter(filter))
  {
  }

  void IPartCombItr::reset()
  {
    // Reset each individual iterator and set our current value accordingly
    m_idxItr.reset();
    readCurrent();
  }

  bool IPartCombItr::exhausted() const
  {
    return m_idxItr.exhausted();
  }

  IPartCombItr::reference IPartCombItr::operator*() const
  {
    if (exhausted())
      throw std::runtime_error("Dereferencing past-the-end iterator");
    return m_current;
  }

  IPartCombItr::pointer IPartCombItr::operator->() const
  {
    if (exhausted())
      throw std::runtime_error("Dereferencing past-the-end iterator");
    return &m_current;
  }

  IPartCombItr &IPartCombItr::operator++()
  {
    if (exhausted())
      // Don't iterate an iterator that is already past the end
      return *this;
    ++m_idxItr;
    readCurrent();
    return *this;
  }

  IPartCombItr IPartCombItr::operator++(int)
  {
    IPartCombItr ret(*this);
    this->operator++();
    return ret;
  }

  bool IPartCombItr::operator==(const IPartCombItr &other) const
  {
    // All past-the-end iterators compare equal
    if (exhausted() && other.exhausted())
      return true;
    return m_idxItr == other.m_idxItr && m_linkInfoItrs == other.m_linkInfoItrs;
  }

  bool IPartCombItr::operator!=(const IPartCombItr &other) const
  {
    return !(*this == other);
  }

  void IPartCombItr::readCurrent()
  {
    if (exhausted())
      m_current.assign(m_current.size(), {});
    else
    {
      auto currentItr = m_current.begin();
      for (std::size_t iLeg = 0; iLeg < nLegs(); ++iLeg)
      {
        std::vector<std::size_t> indices = *(*m_idxItr)[iLeg];
        for (std::size_t idx : indices)
            *(currentItr++) = *(m_linkInfoItrs[iLeg] + idx);
      }
      if (!m_filter(m_current))
        // If we fail the filter condition, advance the iterator again
        this->operator++();
    }
  }

} // namespace TrigCompositeUtils
