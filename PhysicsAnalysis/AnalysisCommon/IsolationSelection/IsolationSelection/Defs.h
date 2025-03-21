/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_DEFS_H
#define ISOLATIONSELECTION_DEFS_H

// EDM include(s):
#include <AthContainers/AuxElement.h>
#include <xAODCaloEvent/CaloCluster.h>
#include <xAODPFlow/FlowElement.h>
#include <xAODPrimitives/IsolationType.h>
#include <xAODTracking/TrackParticle.h>

#include <set>
#include <unordered_set>
namespace CP {

    using CharAccessor = SG::AuxElement::ConstAccessor<char>;
    using CharDecorator = SG::AuxElement::Decorator<char>;

    using FloatAccessor = SG::AuxElement::ConstAccessor<float>;
    using FloatDecorator = SG::AuxElement::Decorator<float>;

    using SelectionAccessor = std::unique_ptr<CharAccessor>;
    using SelectionDecorator = std::unique_ptr<CharDecorator>;

    using BoolDecorator = SG::AuxElement::Decorator<bool>;
    using BoolAccessor = SG::AuxElement::ConstAccessor<bool>;

    using IntDecorator = SG::AuxElement::Decorator<int>;
    using IntAccessor = SG::AuxElement::ConstAccessor<int>;

    using ShortDecorator = SG::AuxElement::Decorator<short>;
    using ShortAccessor = SG::AuxElement::ConstAccessor<short>;

    using IsoType = xAOD::Iso::IsolationType;
    using IsoVector = std::vector<IsoType>;

    /// Small helper struct to have sets of particle pointers sorted by pt
    template <class Obj> struct SortedObjPtr {
        SortedObjPtr() = default;
        SortedObjPtr(const Obj* _ptr) : m_ptr{_ptr} {}

        const Obj* get() const { return m_ptr; }
        const Obj* operator->() const { return m_ptr; }
        const Obj& operator*() const { return *m_ptr; }
        bool operator!() const { return !m_ptr; }
        operator bool() const { return m_ptr; }
        operator const Obj*() const { return m_ptr; }
        bool operator<(const SortedObjPtr<Obj>& other) const {
            return other.get() != get() && get() &&
                   (!other || other->pt() < get()->pt() || (other->pt() == get()->pt() && other.get() < get()));
        }

    private:
        const Obj* m_ptr{nullptr};
    };
    /// For the flow elements we need a special derivate which also contains the weights
    struct FlowElementPtr : public SortedObjPtr<xAOD::FlowElement> {
        FlowElementPtr(const xAOD::FlowElement* ele, float _weight) : SortedObjPtr<xAOD::FlowElement>{ele}, weight{_weight} {}
        FlowElementPtr() = default;
        bool operator<(const FlowElementPtr& other) const {
            if (other.weight != weight) return weight < other.weight;
            return SortedObjPtr<xAOD::FlowElement>::operator<(other);
        }
        float weight{1.f};
    };

    using CaloClusterPtr = SortedObjPtr<xAOD::CaloCluster>;
    using TrackPtr = SortedObjPtr<xAOD::TrackParticle>;

    using TrackSet = std::set<TrackPtr>;
    using ClusterSet = std::set<CaloClusterPtr>;
    using PflowSet = std::set<FlowElementPtr>;

    using UnorderedClusterSet = std::unordered_set<const xAOD::CaloCluster*>;
    using UnorderedContainerSet = std::unordered_set<const SG::AuxVectorData*>;
}  // namespace CP

#endif
