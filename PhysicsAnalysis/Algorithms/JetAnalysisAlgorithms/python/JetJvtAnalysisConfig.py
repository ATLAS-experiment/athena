# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class JetJvtAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the JVT sequence"""

    def __init__ (self) :
        super (JetJvtAnalysisConfig, self).__init__ ()
        self.setBlockName('JVT')
        self.addDependency('OverlapRemoval', required=False)
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. Typically "
            "not needed here.")
        self.addOption ('enableFJvt', False, type=bool,
            info="whether to enable forward JVT calculations.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + self.postfix

    def makeAlgs (self, config) :

        if config.dataType() is DataType.Data: return

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        # Set up the per-event jet efficiency scale factor calculation algorithm
        alg = config.createAlgorithm( 'CP::AsgEventScaleFactorAlg', 'JvtEventScaleFactorAlg' )
        preselection = config.getFullSelection (self.containerName, '')
        alg.preselection = preselection + '&&no_jvt' if preselection else 'no_jvt'
        alg.scaleFactorInputDecoration = 'jvt_effSF_%SYS%'
        alg.scaleFactorOutputDecoration = 'jvt_effSF_%SYS%'
        alg.particles = config.readName (self.containerName)

        config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration, 'weight_jvt_effSF' + postfix)

        if self.enableFJvt:
            alg = config.createAlgorithm( 'CP::AsgEventScaleFactorAlg', 'ForwardJvtEventScaleFactorAlg' )
            preselection = config.getFullSelection (self.containerName, '')
            alg.preselection = preselection + '&&no_fjvt' if preselection else 'no_fjvt'
            alg.scaleFactorInputDecoration = 'fjvt_effSF_%SYS%'
            alg.scaleFactorOutputDecoration = 'fjvt_effSF_%SYS%'
            alg.particles = config.readName (self.containerName)

            config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration, 'weight_fjvt_effSF' + postfix)

