# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import sys

def patternToVars( Pattern):
    f = open(Pattern)
    lines = [line for line in f.readlines() if line.strip()]

    f.close()
    for i in range(len(lines)):
        lines[i] = (lines[i]).rstrip().split(' ')
    if len(lines[0]) != 1: 
        print('ERROR: The format of input pattern.dat is unknown.')
        sys.exit(-1)
    if int(lines[0][0]) != len(lines[1]): 
        print('ERROR: The format of input pattern.dat is unknown.')
        sys.exit(-1)
    if len(lines[2]) != 1: 
        print('ERROR: The format of input pattern.dat is unknown.')
        sys.exit(-1)
    if int(lines[2][0]) != len(lines[3]): 
        print('ERROR: The format of input pattern.dat is unknown.')
        sys.exit(-1)
    if len(lines[4]) != 1:
        print('ERROR: The format of input pattern.dat is unknown.')
        sys.exit(-1) 
    if int(lines[4][0]) != len(lines) - 5: 
        print('ERROR: The format of input pattern.dat is unknown.')
        sys.exit(-1) 
    DACs = list(map(int, lines[1]))
    Delays = list(map(int, lines[3]))
    ptrns = [ item for sublist in lines[5:] for item in sublist ]
    Pattern = []
    for i in range(0,len(ptrns)): 
        print(ptrns[i])
        Pattern += [int(ptrns[i],16)]
    return DACs, Delays, Pattern

def LArCalibDigitsMakerCfg(flags,DigitsKey=""):
    
    result = ComponentAccumulator()
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg,LArCalibIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))
    result.merge(LArCalibIdMappingCfg(flags))
    if flags.LArCalib.isSC:
       from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg,LArCalibIdMappingSCCfg
       result.merge(LArOnOffIdMappingSCCfg(flags))
       result.merge(LArCalibIdMappingSCCfg(flags))


    BoardIDs = {}
    BoardIDs["EMBA"] = [1023868928, 1040121856, 1024917504, 1024393216, 1025966080, 1025441792, 1027014656, 1026490368, 1028063232, 1027538944, 1029111808, 1028587520, 1030160384, 1029636096, 1031208960, 1030684672, 1032257536, 1031733248, 1033306112, 1032781824, 1034354688, 1033830400, 1035403264, 1034878976, 1036451840, 1035927552, 1037500416, 1036976128, 1038548992, 1038024704, 1039597568, 1039073280]
    BoardIDs["EMBC"] = [1015480320, 1014956032, 1014431744, 1013907456, 1013383168, 1012858880, 1012334592, 1011810304, 1011286016, 1010761728, 1010237440, 1009713152, 1009188864, 1008664576, 1008140288, 1007616000, 1007091712, 1023344640, 1022820352, 1022296064, 1021771776, 1021247488, 1020723200, 1020198912, 1019674624, 1019150336, 1018626048, 1018101760, 1017577472, 1017053184, 1016528896, 1016004608]

    BoardIDs["Barrel"] = BoardIDs["EMBA"] + BoardIDs["EMBC"]

    BoardIDs["EMECAstd"] = [1057947648, 1057423360, 1060044800, 1059520512, 1061617664, 1061093376, 1063714816, 1063190528, 1064763392, 1064239104, 1066860544, 1066336256, 1067909120, 1067384832, 1070006272, 1069481984]
    BoardIDs["EMECAspe"] = [1058603008, 1058635776, 1062273024, 1062305792, 1065418752, 1065451520, 1068564480, 1068597248]
    BoardIDs["EMECCstd"] = [1046937600, 1046413312, 1044840448, 1044316160, 1043267584, 1042743296, 1041170432, 1040646144, 1053229056, 1052704768, 1051131904, 1050607616, 1050083328, 1049559040, 1047986176, 1047461888]
    BoardIDs["EMECCspe"] = [1045495808, 1045528576, 1041825792, 1041858560, 1051787264, 1051820032, 1048641536, 1048674304]

    BoardIDs["EMECspe"] = BoardIDs["EMECAspe"]+BoardIDs["EMECCspe"]
    BoardIDs["EMECstd"] = BoardIDs["EMECAstd"]+BoardIDs["EMECCstd"]

    BoardIDs["HECA"] = [1058897920, 1062567936, 1065713664, 1068859392]
    BoardIDs["HECC"] = [1045790720, 1042120704, 1052082176, 1048936448]
    BoardIDs["FCALA"] = [1060601856]
    BoardIDs["FCALC"] = [1043824640]

    BoardIDs["HEC"] = BoardIDs["HECA"] + BoardIDs["HECC"]
    BoardIDs["FCAL"] = BoardIDs["FCALA"] + BoardIDs["FCALC"]
    
    Boards = [ BoardIDs["Barrel"], BoardIDs["EMECstd"], BoardIDs["EMECspe"], BoardIDs["HEC"], BoardIDs["FCAL"] ]

    # read and interpret parameters.dat

    DACs,Delays,Patterns=patternToVars(flags.LArCalib.Input.paramsFile)
    nPatterns=len(Patterns)
    
    print('CalibDigitsMakerConfig: ',DACs,' ',Delays,' ',Patterns)
    result.addEventAlgo(CompFactory.LArCalibDigitMaker("LArCalibDigitMaker", KeyList = [DigitsKey],
                                                       NTrigger = 100, DAC = [DACs,DACs,DACs,DACs,DACs],
                                                       Delay = Delays, Pattern = [Patterns,Patterns,Patterns,Patterns,Patterns], 
                                                       nPattern = [nPatterns,nPatterns,nPatterns,nPatterns,nPatterns],
                                                       BoardIDs = Boards)) 

    from AthenaCommon.SystemOfUnits import ns
    scmap="LArOnOffIdMapSC" if flags.LArCalib.isSC else ""
    scclmap="LArCalibIdMapSC" if flags.LArCalib.isSC else ""
    result.addEventAlgo(CompFactory.LArCalibDigitsAccumulator("LArCalibDigitsAccumulator", KeyList = [DigitsKey],
                                                             LArAccuCalibDigitContainerName = "", 
                                                             CalibMapSCKey=scclmap, ScCablingKey=scmap,
                                                             DelayScale = (25./240.)*ns, SampleShift = 0,
                                                             KeepFullyPulsedSC = True, KeepOnlyPulsed = True,
                                                             isSC = flags.LArCalib.isSC, DropPercentTrig = 0))

    return result
