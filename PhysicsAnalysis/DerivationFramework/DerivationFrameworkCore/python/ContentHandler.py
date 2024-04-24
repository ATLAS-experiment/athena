# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr
from AthenaCommon.AlgSequence import AlgSequence
import PyUtils.Logging as L
msg = L.logging.getLogger('DerivationFramework__ContentHandler')
msg.setLevel(L.logging.INFO)

class ContentHandler:
	def __init__(self,inputName,namesAndTypes):
		self.name = inputName
		self.AppendToDictionary = {}
		self.NamesAndTypes = namesAndTypes
	
	def mainContainerLine(self,containerName):
		theDictionary = self.NamesAndTypes.copy()
		theDictionary.update (self.AppendToDictionary)
		line = ''
		if containerName in theDictionary.keys():
			line = theDictionary[containerName]+"#"+containerName
		else:
			msg.warning('Collection with name %s not found in input file or in definitions from user. No action will be taken for this collection.', containerName)
		return line
		

	def GetContent(self,contentList,wholeContentList):
		mainOutput = []
		auxOutput = {}
		for item in contentList:
			components = item.split(".")
			if len(components)==0:
				continue	
			mainItem = self.mainContainerLine(components[0])
			if mainItem=='':
				continue
			if len(components)==1:
				# deal with main containers
				if (mainItem not in mainOutput):
					mainOutput.append(mainItem)
			if len(components)>1:
				# All variables needed
				if (components[0]+"." in wholeContentList):
					if (components[0] not in auxOutput.keys()):
						auxOutput[components[0]] = ""
					continue
				# All variables needed but user did it via smart slimming list with ContainerAux.
				if (len(components)==2):
					if (components[1] == ""):
						wholeContentList.append(components[0]+".")
						auxOutput[components[0]] = ""
						continue	
				if (components[0] not in auxOutput.keys()):
					auxOutput[components[0]] = components[1:]
				if (components[0] in auxOutput.keys()):
					auxOutput[components[0]] = list(set(auxOutput[components[0]]) | set(components[1:]))
		return mainOutput,auxOutput 
