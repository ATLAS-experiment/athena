#ifndef FEELECTRONHELPER_H
#define FEELECTRONHELPER_H

#include "AsgMessaging/AsgMessaging.h"
#include "xAODEgamma/ElectronContainer.h"

class FEElectronHelper : public asg::AsgMessaging {

    public:
        FEElectronHelper();
        ~FEElectronHelper() {};

        bool checkElectronLinks(const std::vector < ElementLink< xAOD::ElectronContainer > >& FE_ElectronLinks, const std::string& qualityString) const;

};
#endif