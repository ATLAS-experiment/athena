# Superchic basic configuration

evgenConfig.generators += ["SuperChic"]

from Superchic_i.SuperChicUtils import SuperChicConfig, SuperChicRun
scConfig = SuperChicConfig(runArgs)

SuperChicRun(scConfig, genSeq)




