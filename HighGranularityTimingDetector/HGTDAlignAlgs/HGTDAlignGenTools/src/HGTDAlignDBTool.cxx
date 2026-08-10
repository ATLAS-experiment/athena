/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// HGTDAlignDBTool.cxx
// AlgTool for creating and managing HGTD alignment payloads.
// This is the first implementation of the HGTD alignment database tool.
// The alignment constants are stored at detector-module level.
// Each HGTD module owns one AlignableTransform entry identified by
// its Identifier.
// Tool responsibilities:
//   * create an empty HGTD alignment payload
//   * update module transforms
//   * retrieve module transforms
//   * print payload contents
//   * stream payload objects
//   * register payload in the conditions database
// Higher-level alignment strategies may still be performed externally,
// but the final payload stored in SQLite is represented at module level.
// Fatima Bendebba, started 2026

#include "HGTDAlignDBTool.h"

#include "DetDescrConditions/AlignableTransform.h"
#include "DetDescrConditions/AlignableTransformContainer.h"
#include "RegistrationServices/IIOVRegistrationSvc.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "HGTD_Identifier/HGTD_ID.h"

///////////////////////////////////////////////////////////////////

HGTDAlignDBTool::
HGTDAlignDBTool(const std::string& type,
                const std::string& name,
                const IInterface* parent)
  :
  AthAlgTool(type,name,parent)
{
    declareInterface<IHGTDAlignDBTool>(this);
}

///////////////////////////////////////////////////////////////////

StatusCode HGTDAlignDBTool::initialize()
{
    ATH_MSG_DEBUG("========================================");
    ATH_MSG_DEBUG("LOCAL HGTDAlignDBTool initialize()");
    ATH_MSG_DEBUG("========================================");

    ATH_CHECK(detStore().retrieve());
    ATH_CHECK(m_condStream.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detManager));
    ATH_CHECK(detStore()->retrieve(m_hgtdId));

    ATH_MSG_INFO("Alignment folder = " << m_dbRoot);
    ATH_MSG_DEBUG("HGTD modules = " << m_hgtdId->wafer_hash_max());

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////

std::string
HGTDAlignDBTool::moduleTag(const Identifier& id) const
{
    IdentifierHash hash;
    IdContext waferContext = m_hgtdId->wafer_context();

    if (m_hgtdId->get_hash(id,
                           hash,
                           &waferContext))
    {
        ATH_MSG_WARNING("Failed to obtain wafer hash for " << m_hgtdId->show_to_string(id));
        return {};
    }

    return "HGTDModule_" + std::to_string(hash.value());
}

///////////////////////////////////////////////////////////////////

StatusCode HGTDAlignDBTool::createDB()
{
    ATH_MSG_INFO("Creating HGTD alignment payload");

    if(detStore()->contains<AlignableTransformContainer>(m_dbRoot.value()))
    {
        ATH_MSG_ERROR("Alignment container already exists.");
        return StatusCode::FAILURE;
    }

    auto* container = new AlignableTransformContainer;
    
    unsigned int nModules = 0;
    for(const auto* element : *m_detManager->getDetectorElementCollection())
    {
        if(!element)
            continue;

        Identifier id = element->identify();
        IdentifierHash hash;
        IdContext waferContext = m_hgtdId->wafer_context();

        if(m_hgtdId->get_hash(id,
                              hash,
                              &waferContext))
        {
            ATH_MSG_WARNING("Cannot obtain wafer hash for " << m_hgtdId->show_to_string(id));
            continue;
        }

        auto* transform = new AlignableTransform(moduleTag(id));

        transform->add(
            id,
            Amg::EigenTransformToCLHEP(Amg::Transform3D::Identity()));

        container->push_back(transform);
        container->add(hash);
        ATH_MSG_DEBUG("hash = " << hash.value()
                    << " tag = " << transform->tag());

        ++nModules;
    }

    SG::DataProxy* proxy = detStore()->proxy(
                            ClassID_traits<AlignableTransformContainer>::ID(),
                            m_dbRoot.value());

    ATH_MSG_DEBUG("Proxy before record = " << proxy);

    ATH_CHECK(detStore()->record(container, m_dbRoot.value()));

    ATH_MSG_DEBUG("After record:");
     
    const AlignableTransformContainer* test = nullptr;
    StatusCode sc = detStore()->retrieve(test, m_dbRoot.value());

    ATH_MSG_DEBUG("retrieve after record = " << sc);

    if (test) {
        ATH_MSG_DEBUG("retrieve pointer = " << test);
    }
    //sortTrans();

    ATH_MSG_INFO("Created alignment payload for "
                 << nModules
                 << " HGTD modules.");

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////

AlignableTransform*
HGTDAlignDBTool::getTransPtr(const Identifier& id) const
{
    const AlignableTransformContainer* container = nullptr;

    if (detStore()->retrieve(container, m_dbRoot.value()).isFailure())
        return nullptr;

    const std::string tag = moduleTag(id);

    for (const auto* transform : *container) {
        if (transform->tag() == tag) {
            return const_cast<AlignableTransform*>(transform);
        }
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////

const AlignableTransform*
HGTDAlignDBTool::cgetTransPtr(const Identifier& id) const
{
    const AlignableTransformContainer* container = nullptr;

    if (detStore()->retrieve(container, m_dbRoot.value()).isFailure())
        return nullptr;

    const std::string tag = moduleTag(id);

    for (const auto* transform : *container) {
        if (transform->tag() == tag) {
            return transform;
        }
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////

bool
HGTDAlignDBTool::setTrans(const Identifier& id,
                          unsigned int level,
                          const Amg::Transform3D& trans) const
{
    /// Currently ignored.
    /// Reserved for possible future hierarchical alignment levels.
    (void)level;

    AlignableTransform* transform = getTransPtr(id);

    if(!transform)
    {
        ATH_MSG_ERROR("Cannot retrieve AlignableTransform for " << m_hgtdId->show_to_string(id));
        return false;
    }

    return transform->update(
        id,
        Amg::EigenTransformToCLHEP(trans));
}

///////////////////////////////////////////////////////////////////

bool
HGTDAlignDBTool::setTrans(
    const Identifier& id,
    unsigned int level,
    const Amg::Vector3D& translation,
    double alpha,
    double beta,
    double gamma) const
{
    Amg::Translation3D t(translation);

    Amg::Transform3D tr =
        t * Amg::RotationMatrix3D::Identity();

    tr *= Amg::AngleAxis3D(
        gamma,
        Amg::Vector3D(0.,0.,1.));

    tr *= Amg::AngleAxis3D(
        beta,
        Amg::Vector3D(0.,1.,0.));

    tr *= Amg::AngleAxis3D(
        alpha,
        Amg::Vector3D(1.,0.,0.));

    return setTrans(id, level, tr);
}

///////////////////////////////////////////////////////////////////

bool
HGTDAlignDBTool::tweakTrans(const Identifier& id,
                            unsigned int level,
                            const Amg::Transform3D& trans) const
{
    (void)level;

    AlignableTransform* transform = getTransPtr(id);

    if(!transform)
    {
        ATH_MSG_ERROR("Cannot retrieve AlignableTransform for " << m_hgtdId->show_to_string(id));
        return false;
    }

    return transform->tweak(
        id,
        Amg::EigenTransformToCLHEP(trans));
}

///////////////////////////////////////////////////////////////////

bool
HGTDAlignDBTool::tweakTrans(
    const Identifier& id,
    unsigned int level,
    const Amg::Vector3D& translation,
    double alpha,
    double beta,
    double gamma) const
{
    Amg::Translation3D t(translation);

    Amg::Transform3D tr =
        t * Amg::RotationMatrix3D::Identity();

    tr *= Amg::AngleAxis3D(
        gamma,
        Amg::Vector3D(0.,0.,1.));

    tr *= Amg::AngleAxis3D(
        beta,
        Amg::Vector3D(0.,1.,0.));

    tr *= Amg::AngleAxis3D(
        alpha,
        Amg::Vector3D(1.,0.,0.));

    return tweakTrans(id, level, tr);
}

///////////////////////////////////////////////////////////////////

Amg::Transform3D
HGTDAlignDBTool::getTrans(const Identifier& id,
                          unsigned int level) const
{
    (void)level;
    const AlignableTransform* transform = cgetTransPtr(id);

    if(!transform)
        return Amg::Transform3D::Identity();

    AlignableTransform::AlignTransMem_citr itr = transform->findIdent(id);

    if (itr != transform->end()) {
        return Amg::CLHEPTransformToEigen(
            itr->transform());
    }

    return Amg::Transform3D::Identity();
}

///////////////////////////////////////////////////////////////////

StatusCode HGTDAlignDBTool::outputObjs()
{
    ATH_MSG_INFO("Writing HGTD alignment payload");

    if (m_condStream->connectOutput().isFailure()) {
        ATH_MSG_ERROR("Cannot connect output stream");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Output stream connected");

    if (!detStore()->contains<AlignableTransformContainer>(m_dbRoot.value())) {
        ATH_MSG_ERROR("Cannot find AlignableTransformContainer "
                      << m_dbRoot.value());
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Container exists in DetectorStore");

    IAthenaOutputStreamTool::TypeKeyPairs objects;

    objects.emplace_back("AlignableTransformContainer", m_dbRoot.value());

    ATH_MSG_DEBUG("Streaming "
                 << objects.size()
                 << " object(s)");

    if (m_condStream->streamObjects(objects).isFailure()) {

        const AlignableTransformContainer* c = nullptr;
        ATH_CHECK(detStore()->retrieve(c, m_dbRoot.value()));
        ATH_MSG_INFO("Container key = " << m_dbRoot.value());
        ATH_MSG_INFO("Container size = " << c->size());
        ATH_MSG_INFO("Container pointer = " << c);

        ATH_MSG_ERROR("streamObjects failed");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Objects streamed");

    if (m_condStream->commitOutput().isFailure()) {
        ATH_MSG_ERROR("commitOutput failed");
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("POOL payload committed");
    
    const AlignableTransformContainer* c = nullptr;

    ATH_CHECK(detStore()->retrieve(c, m_dbRoot.value()));

    if (c) {
        ATH_MSG_DEBUG("container ptr = " << c);
        ATH_MSG_DEBUG("container size = " << c->size());
    } else {
        ATH_MSG_DEBUG("container is nullptr");
    }

    SG::DataProxy* proxy =
    detStore()->proxy(ClassID_traits<AlignableTransformContainer>::ID(),
                      m_dbRoot.value());

    if (!proxy) {
        ATH_MSG_WARNING("NO PROXY FOUND");
    }
    else {
        ATH_MSG_DEBUG("Proxy = " << proxy);
        ATH_MSG_DEBUG("Proxy name = " << proxy->name());
        ATH_MSG_DEBUG("Proxy CLID = " << proxy->clID());
        ATH_MSG_DEBUG("Proxy address = " << proxy->address());

        if (proxy->address()) {
            ATH_MSG_DEBUG("Storage type = " << proxy->address()->svcType());
            ATH_MSG_DEBUG("Address class = " << typeid(*proxy->address()).name());
        }
    }

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////

StatusCode
HGTDAlignDBTool::fillDB(const std::string& tag,
                        unsigned int run1,
                        unsigned int event1,
                        unsigned int run2,
                        unsigned int event2) const
{
    SmartIF<IIOVRegistrationSvc> regSvc{
        Gaudi::svcLocator()->service("IOVRegistrationSvc")
    };

    if(!regSvc.isValid())
    {
        ATH_MSG_FATAL("Cannot retrieve IOVRegistrationSvc");
        return StatusCode::FAILURE;
    }
    
    ATH_MSG_INFO("Registering folder " << m_dbRoot);
    ATH_MSG_INFO("Tag = " << tag);

    ATH_CHECK(
        regSvc->registerIOV(
            "AlignableTransformContainer",
            m_dbRoot.value(),
            tag,
            run1,
            run2,
            event1,
            event2));

    ATH_MSG_INFO("IOV registration successful");

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////

void HGTDAlignDBTool::printDB() const
{
    const AlignableTransformContainer* container = nullptr;

    if (detStore()->retrieve(container, m_dbRoot.value()).isFailure())
        return;

    ATH_MSG_INFO("HGTD alignment payload:");

    unsigned int n = 0;

    for (const auto* transform : *container) {

        for (auto itr = transform->begin();
             itr != transform->end();
             ++itr)
        {
            const Amg::Transform3D trans = Amg::CLHEPTransformToEigen(itr->transform());
            const Amg::Vector3D shift = trans.translation();

            ATH_MSG_INFO("Identifier : "
                         << itr->identify()
                         << " Shift = ("
                         << shift.x() << ", "
                         << shift.y() << ", "
                         << shift.z() << ")");

            ++n;
        }
    }

    ATH_MSG_INFO("Total aligned modules : " << n);
}

///////////////////////////////////////////////////////////////////

void HGTDAlignDBTool::sortTrans() const
{
    const AlignableTransformContainer* container = nullptr;

    if (detStore()->retrieve(container, m_dbRoot.value()).isFailure())
        return;

    for (const auto* transform : *container) {
        const_cast<AlignableTransform*>(transform)->sortv();
    }
}