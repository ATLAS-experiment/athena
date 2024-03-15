/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include <IsolationSelection/IsoVariableHelper.h>
#include <xAODBase/IParticleHelpers.h>
#include <xAODPrimitives/IsolationHelpers.h>
namespace CP {

    //######################################################################################################
    //                                      IsoVariableHelper
    //######################################################################################################
    IsoVariableHelper::IsoVariableHelper(xAOD::Iso::IsolationType type, const std::string& backupPreFix, const std::string& isoDecSuffix) :
        asg::AsgMessaging{"IsoVariableHelper_ "+ std::string(xAOD::Iso::toString(type))},
        m_isoType(type),
        m_BackupIso(!backupPreFix.empty()),
        m_dec_IsoIsBackup("IsBackup_" + std::string(xAOD::Iso::toString(type)) + (backupPreFix.empty() ? "" : "_") + backupPreFix),
        m_acc_IsoIsBackup("IsBackup_" + std::string(xAOD::Iso::toString(type)) + (backupPreFix.empty() ? "" : "_") + backupPreFix),
        m_acc_iso_variable(xAOD::Iso::toString(type)),
        m_dec_iso_variable(xAOD::Iso::toString(type) + (isoDecSuffix.empty() ? "" : "_") + isoDecSuffix),
        m_acc_iso_backup(std::string(xAOD::Iso::toString(type)) + (backupPreFix.empty() ? "" : "_") + backupPreFix),
        m_dec_iso_backup(std::string(xAOD::Iso::toString(type)) + (backupPreFix.empty() ? "" : "_") + backupPreFix) {}

    CorrectionCode IsoVariableHelper::getOriginalIsolation(const xAOD::IParticle* particle, float& value) const {
        if (!particle) {
            ATH_MSG_ERROR(__func__<<"() -- "<<__LINE__<<": no particle given");
            return CorrectionCode::Error;
        }
        if (!m_BackupIso) {
            return getIsolationFromOriginal(particle, value);
        } else {
            if (!m_acc_IsoIsBackup.isAvailable(*particle) || !m_acc_IsoIsBackup(*particle)) {
                ATH_MSG_WARNING(__func__<<"() -- "<<__LINE__<<":"
                        <<" No isolation value was backuped thus far. "
                        <<"Did you call the BackupIsolation before for "
                        <<SG::AuxTypeRegistry::instance().getName(m_acc_IsoIsBackup.auxid()));
                return CorrectionCode::Error;
            } else {
                value = m_acc_iso_backup(*particle);
            }
        }
        return CorrectionCode::Ok;
    }
    CorrectionCode IsoVariableHelper::getIsolationFromOriginal(const xAOD::IParticle* particle, float& value) const {
        const xAOD::IParticle* originalParticle = xAOD::getOriginalObject(*particle);
        if (originalParticle && getIsolation(originalParticle, value) == CorrectionCode::Error)
            return CorrectionCode::Error;
        else if (!originalParticle) {
            ATH_MSG_DEBUG(__func__<<"() -- "<<__LINE__<<": No orignal object was found");
            return getIsolation(particle, value);
        }
        return CorrectionCode::Ok;
    }
    CorrectionCode IsoVariableHelper::getIsolation(const xAOD::IParticle* particle, float& value) const {
        if (!particle || !m_acc_iso_variable.isAvailable(*particle)) {
           ATH_MSG_ERROR(__func__<<"() -- "<<__LINE__<<": Failed to retrieve isolation "<<
                         SG::AuxTypeRegistry::instance().getName(m_acc_iso_variable.auxid()));
            return CorrectionCode::Error;
        }
        value = m_acc_iso_variable(*particle);
        return CorrectionCode::Ok;
    }
    CorrectionCode IsoVariableHelper::backupIsolation(const xAOD::IParticle* particle) const {
        if (!particle) {
            ATH_MSG_ERROR(__func__<<"() -- "<<__LINE__<<": no particle given");
            return CorrectionCode::Error;
        }
        if (m_BackupIso && (!m_acc_IsoIsBackup.isAvailable(*particle) || !m_acc_IsoIsBackup(*particle))) {
            float Isovalue = 0;
            if (getIsolation(particle, Isovalue) == CorrectionCode::Error) { 
                return CorrectionCode::Error; 
            }
            m_dec_IsoIsBackup(*particle) = true;
            m_dec_iso_backup(*particle) = Isovalue;
        }
        return CorrectionCode::Ok;
    }
    CorrectionCode IsoVariableHelper::setIsolation(const xAOD::IParticle* particle, float value) const {
        if (!particle) {
             ATH_MSG_ERROR(__func__<<"() -- "<<__LINE__<<": no particle given");
            return CorrectionCode::Error;
        }
        if (std::isnan(value) || std::isinf(value)) {
            ATH_MSG_ERROR(__func__<<"() -- "<<__LINE__<<": the value is not a number");
            return CorrectionCode::Error;
        }
        m_dec_iso_variable(*particle) = value;
        return CorrectionCode::Ok;
    }
    IsoType IsoVariableHelper::isotype() const { return m_isoType; }
    std::string IsoVariableHelper::name() const { return std::string(xAOD::Iso::toCString(isotype())); }

}  // namespace CP
