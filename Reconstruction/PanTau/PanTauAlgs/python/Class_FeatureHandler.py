# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import string

class FeatureHandler:
    
    def getFeatureNames(self):
        listFeatNames = []
        for iFeature in self.m_Feature_Names:
            listFeatNames += [self.m_Feature_Names[iFeature]]
        return listFeatNames
    #end getFeatureNames
    
    def getFeatureTypes(self):
        listFeatTypes = []
        for iFeature in self.m_Feature_Types:
            listFeatTypes += [self.m_Feature_Types[iFeature]]
        return listFeatTypes
    #end getFeatureTypes
    
    def getFeatureDefaultValues(self):
        listFeatDefaults = []
        for iFeature in self.m_Feature_Defaults:
            listFeatDefaults += [self.m_Feature_Defaults[iFeature]]
        return listFeatDefaults
    #end getFeatureDefaultValues
    
    def addToFeatures(self, theBaseName, theVarTypeName, theFeatureType, defaultValueOverride=0):
        """ Add a single feature with given name, type and default value """
        
        #set feature name and default value
        featureName     = theVarTypeName + "_" + theBaseName
        defaultValue    = self.m_DefaultValues[theVarTypeName]
        if defaultValueOverride != 0:
            defaultValue    = defaultValueOverride
        
        #add to features
        self.m_Feature_Names[featureName]    = featureName
        self.m_Feature_Defaults[featureName] = defaultValue
        self.m_Feature_Types[featureName]    = theFeatureType
    #end addToFeatures
    
    def addToFeatures_FullName(self, completeFeatureName, theVarTypeName, theFeatureType, defaultValueOverride=0):
        #set feature name and default value
        featureName     = completeFeatureName
        defaultValue    = self.m_DefaultValues[theVarTypeName]
        if defaultValueOverride != 0:
            defaultValue    = defaultValueOverride
        
        #add to features
        self.m_Feature_Names[featureName]    = featureName
        self.m_Feature_Defaults[featureName] = defaultValue
        self.m_Feature_Types[featureName]    = theFeatureType
    #end addToFeatures_FullName
    
    
    def addToFeatures_AllTypes(self, theBaseName, theVarTypeName, theFeatureType, defaultValueOverride=0):
        """ Add the feature described by theName, theType and theDefault for all known constituent types"""
        
        #set default value
        defaultValue    = self.m_DefaultValues[theVarTypeName]
        if defaultValueOverride != 0:
            defaultValue = defaultValueOverride
        
        #fill feature for all types
        for iConstType in self.m_ConstituentTypes:
            featureName     = iConstType + "_" + theVarTypeName + "_" + theBaseName
            
            self.m_Feature_Names[featureName]    = featureName
            self.m_Feature_Defaults[featureName] = defaultValue
            self.m_Feature_Types[featureName]    = theFeatureType
        #end loop over types
    #end addToFeatures_AllTypes
    
    
    def addToFeatures_AllTypes_AllEnergyVariants(self, theBaseName, theVarTypeName, theFeatureType, defaultValueOverride=0):
        """ Add the feature described by theName, theType and theDefault for all known constituent types"""
        for iEnergyType in self.m_EnergyVariantsList:
            featureName     = theBaseName + iEnergyType
            self.addToFeatures_AllTypes(featureName, theVarTypeName, theFeatureType, defaultValueOverride)
        #end loop over energy types
    #end addToFeatures_AllTypes
    
    def __init__(self):
        
        #setup the varType prefixes
        # they are passed to the feature extractor later on - avoids errors due to typos
        self.m_VarTypeName_Ratio        = "Ratio"
        self.m_VarTypeName_Mean         = "Mean"
        self.m_VarTypeName_StdDev       = "StdDev"
        self.m_VarTypeName_HLV          = "HLV"
        self.m_VarTypeName_DeltaR       = "DeltaR"
        self.m_VarTypeName_JetMoment    = "JetMoment"
        self.m_VarTypeName_Combined     = "Combined"
        self.m_VarTypeName_Basic        = "Basic"
        self.m_VarTypeName_PID          = "PID"
        self.m_VarTypeName_Shots        = "Shots"
        
        #setup the default values for the different variable types
        self.m_DefaultValues = {}
        self.m_DefaultValues[self.m_VarTypeName_Ratio]        = -0.2
        self.m_DefaultValues[self.m_VarTypeName_Mean]         = -0.2
        self.m_DefaultValues[self.m_VarTypeName_StdDev]       = -0.2
        self.m_DefaultValues[self.m_VarTypeName_HLV]          = -4000.0
        self.m_DefaultValues[self.m_VarTypeName_DeltaR]       = -0.2
        self.m_DefaultValues[self.m_VarTypeName_JetMoment]    = -0.2
        self.m_DefaultValues[self.m_VarTypeName_Combined]     = -10.0
        self.m_DefaultValues[self.m_VarTypeName_Basic]        = -5.0
        self.m_DefaultValues[self.m_VarTypeName_PID]          = -9.0
        self.m_DefaultValues[self.m_VarTypeName_Shots]        = -2.0
        
        #setup the constituent types
        self.m_ConstituentTypeName_All      = "All"
        self.m_ConstituentTypeName_Charged  = "Charged"
        self.m_ConstituentTypeName_Neutral  = "Neutral"
        self.m_ConstituentTypeName_Pi0Neut  = "Pi0Neut"
        
        self.m_ConstituentTypes = []
        #baseline
        self.m_ConstituentTypes += [self.m_ConstituentTypeName_Charged]
        self.m_ConstituentTypes += [self.m_ConstituentTypeName_Neutral]
        self.m_ConstituentTypes += [self.m_ConstituentTypeName_Pi0Neut]
        
        self.m_EnergyVariantsList = ["EtAllConsts"]
        # ==============================================
        
        
        #setup member variables to hold feature info
        self.m_Feature_Names      = {}
        self.m_Feature_Defaults   = {}
        self.m_Feature_Types      = {}
        
        #fill member variables
        self.addSingleVariables()
        self.addMultiplicities()
        self.addTypeSpecificFeatures_PID()
        self.addTypeSpecificFeatures_Shots()
        self.addTypeSpecificFeatures_Ratios()
        self.addTypeSpecificFeatures_StdDevs()
        self.addTypeSpecificFeatures_HLV()
        self.addTypeSpecificFeatures_JetMoment()
        self.addCombinedFeatures_TypeVsType()
        
    #end init
    
    
    def dump(self):
        print("config_FeatureCalculator: Dumping features:")
        for iFeature in sorted(self.m_Feature_Names.iterkeys()):
            featName    = self.m_Feature_Names[iFeature]
            featType    = self.m_Feature_Types[iFeature]
            featDefault = self.m_Feature_Defaults[iFeature]
            print("\t" + string.ljust(featName, 50) + " of type " + featType + " defaults to " + string.rjust(str(featDefault), 10) )
            #print("\t" + featName + " of type " + featType + " defaults to " + str(featDefault) )
        #end for over features
        print("config_FeatureCalculator: Done dumping features")
    #end dump
    
    
    def addSingleVariables(self):
        Variables = []
        Variables += ["isPanTauCandidate"]
        Variables += ["RecoMode"]
        Variables += ["RecoMode_PanTau"]
        Variables += ["BDTValue_1p0n_vs_1p1n"]
        Variables += ["BDTValue_1p1n_vs_1pXn"]
        Variables += ["BDTValue_3p0n_vs_3pXn"]
        for iVar in Variables:
            curDefVal = self.m_DefaultValues[self.m_VarTypeName_Basic]
            if iVar == "isPanTauCandidate":
                curDefVal = 0
            if iVar == "RecoMode" or iVar == "RecoMode_PanTau":
                curDefVal = -10
            self.addToFeatures(iVar, self.m_VarTypeName_Basic, "F", curDefVal)
        #end loop over variables
    #end def addSingleVariables
    
    
    def addMultiplicities(self):
        Variables = self.m_ConstituentTypes
        
        for iVar in Variables:
            featName = "N" + iVar + "Consts"
            self.addToFeatures(featName, self.m_VarTypeName_Basic, "F")
        #end loop over variables
        
        #add the inclusive one
        self.addToFeatures("N" + self.m_ConstituentTypeName_All     + "Consts", self.m_VarTypeName_Basic, "F")
        
    #end def addMultiplicities
   
    def addTypeSpecificFeatures_PID(self):
        Variables = ["BDTValues"]
        
        MaxNum = 4
        
        for iVar in Variables:
            for iNum in range(1, MaxNum):
                featName = iVar + "_BDTSort_" + str(iNum)
                self.addToFeatures_AllTypes(featName, self.m_VarTypeName_PID, "F")
            #end loop over num
        #end loop over variables
    #end addTypeSpecificFeatures
    
    
    def addTypeSpecificFeatures_Shots(self):
        
        Variables = []
        Variables += ["NPhotonsInSeed"]
        for iVar in Variables:
            featName = iVar
            curDefVal = self.m_DefaultValues[self.m_VarTypeName_Shots]
            
            self.addToFeatures_FullName(self.m_ConstituentTypeName_Neutral + "_" + self.m_VarTypeName_Shots + "_" + featName, self.m_VarTypeName_Shots, "F", curDefVal)
            self.addToFeatures_FullName(self.m_ConstituentTypeName_Pi0Neut + "_" + self.m_VarTypeName_Shots + "_" + featName, self.m_VarTypeName_Shots, "F", curDefVal)
        #end loop over variables
    #end addTypeSpecificFeatures_Shots
    
    def addTypeSpecificFeatures_Ratios(self):
        
        Variables_WithEnergyTypes = []
        Variables_WithEnergyTypes += ["EtOver"]
        Variables_WithEnergyTypes += ["1stBDTEtOver"]
        for iVar in Variables_WithEnergyTypes:
            self.addToFeatures_AllTypes_AllEnergyVariants(iVar, self.m_VarTypeName_Ratio, "F")
        #end loop over variables with energy types
    #end addTypeSpecificFeatures_Ratios
    
    def addTypeSpecificFeatures_StdDevs(self):
        
        Variables_WithEnergyTypes = []
        Variables_WithEnergyTypes += ["Et_Wrt"]
        for iVar in Variables_WithEnergyTypes:
            self.addToFeatures_AllTypes_AllEnergyVariants(iVar, self.m_VarTypeName_StdDev, "F")
    #end addTypeSpecificFeatures_StdDevs
    
    
    def addTypeSpecificFeatures_HLV(self):
        Variables = []
        Variables += ["SumM"]
        for iVar in Variables:
            self.addToFeatures_AllTypes(iVar, self.m_VarTypeName_HLV, "F", -200.)
            
        #end loop over variables
    #end addTypeSpecificFeatures_HLV
    
    def addTypeSpecificFeatures_JetMoment(self):
        Variables = []
        Variables += ["EtDRxTotalEt"]
        for iVar in Variables:
            self.addToFeatures_AllTypes(iVar, self.m_VarTypeName_JetMoment, "F")
        #end loop over variables
    #end addTypeSpecificFeatures_JetMoment
    
    def addCombinedFeatures_TypeVsType(self):
        Types = []
        Types += ["Charged"]
        Types += ["Neutral"]
        Types += ["Pi0Neut"]
        
        Variables = []
        Variables += [ ["Log", "EtOver", "Et"] ]
        Variables += [ ["DeltaR1st", "To1st", ""] ]
        for iType in Types:
            for jType in Types:
                
                #skip if types are equal
                if iType == jType:
                    continue
                
                for iVar in Variables:
                    featName = iVar[0] + iType + iVar[1] + jType + iVar[2]
                    #if iVar[2] != "-":
                        #featName = featName + iVar[2]
                    self.addToFeatures(featName, self.m_VarTypeName_Combined, "F")
                #end loop over variables
            #end loop over jType
        #end loop over iType
    #end addCombinedFeatures_TypeVsType
        
#end class config_FeatureCalculator



