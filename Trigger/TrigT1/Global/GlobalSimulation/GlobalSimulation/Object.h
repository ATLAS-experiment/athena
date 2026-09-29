/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_OBJECT_H
#define GLOBALSIM_OBJECT_H

#include "AthContainers/AuxElement.h"
#include "xAODCore/tools/PrintHelpers.h"

#include "BitSpec.h" // for NoBase

namespace GlobalSim {

    // Object template class exists so that can have "whole object" methods like bits(), which need to know about
    // the "Spec" as a whole. If tried to put these into the ObjectAcc subclass, that subclass doesn't know about
    // the Spec it lives in, so in the DECLARE_FIELDS macro we would also have to pass the SpecName
    // in order to define (in the macro) the bits() method that uses SpecName::packFields etc

    template<typename Spec> class Object; // forward declare primary template

    // Terminal case: nothing more to compose. Also conveniently can create and own an AuxElement in one place
    template<>
    class Object<NoBase> {
    public:
        explicit Object(const SG::AuxElement& obj) : m_ref(obj) {}

        Object() : m_owned(std::make_shared<SG::AuxElement>()), m_ref(*m_owned) {
            m_owned->makePrivateStore();
        }
    protected:
        const SG::AuxElement &auxRef() const { return m_ref; }
    private:
        std::shared_ptr<SG::AuxElement> m_owned; // null when wrapping external object;
        const SG::AuxElement& m_ref;
    };

    template<typename Spec>
    class Object : public Object<typename Spec::BaseSpec>, public Spec::ObjectAcc {
    public:
        using BaseObject = Object<typename Spec::BaseSpec>;

        static constexpr Spec spec{}; // allows access to the BitSpec with obj.spec. syntax


        explicit Object(const SG::AuxElement &obj) : BaseObject(obj), Spec::ObjectAcc(obj) {

        }

        // convenience constructor that will create and use a private AuxElement
        explicit Object() : BaseObject(), Spec::ObjectAcc(BaseObject::auxRef()) { }

        // ------------------------------------------------------------------------
        // Complete packed representation.
        // ------------------------------------------------------------------------

        Spec::bitset_type bits() const {
            return Spec::packFields(Spec::allFields(), Spec::ObjectAcc::m_obj);
        }

        void operator=(const Spec::bitset_type &bits) {
            std::apply([&](const auto *... field) {
                           (field->decodeAndAssignFrom(bits, const_cast<SG::AuxElement &>(Spec::ObjectAcc::m_obj)), ...);
                       },
                       Spec::allFields());
        }

    public:
        const SG::AuxElement *objectPtr() const {
            return &this->Spec::ObjectAcc::m_obj;
        }

        void dump() const {
            xAOD::dump(*objectPtr());
        }


    };

}

#endif