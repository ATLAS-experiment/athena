# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

""" Helper functions for configuring MET chains"""

from __future__ import annotations
from typing import Any, Optional

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ..Menu.SignatureDicts import METChainParts_Default, METChainParts
from ..Config.MenuComponents import (
    ChainStep,
    InEventRecoCA,
    SelectionCA,
    MenuSequenceCA,
)
from .StepOutput import StepOutput
from copy import copy
from ..CommonSequences.FullScanDefs import trkFSRoI
from AthenaCommon.Logging import logging
from TrigEFMissingET.TrigEFMissingETConfig import getMETMonTool
from abc import ABC, abstractmethod
from string import ascii_uppercase
from TrigMissingETHypo.TrigMissingETHypoConfig import TrigMETHypoToolFromDict
from DecisionHandling.DecisionHandlingConfig import ComboHypoCfg
import functools

def streamer_hypo_tool(chainDict):
    return CompFactory.TrigStreamerHypoTool(chainDict["chainName"])


log = logging.getLogger(__name__)

# The keys from the MET chain dict that directly affect reconstruction
# The order here is important as it also controls the dict -> string conversion

recoKeys = [
    "EFrecoAlg",
    "calib",
    "constitType",
    "constitmod",
    "jetCalib",
    "nSigma",
    "addInfo",
]
metFSRoIs: list[str | None] = ["", None, trkFSRoI]


def metRecoDictToString(recoDict: dict[str, str], skipDefaults: bool = True) -> str:
    """Convert a dictionary containing reconstruction keys to a string

    Any key (from recoKeys) missing will just be skipped.

    Parameters
    ----------
    recoDict : dict[str, str]
        The reconstruction dictionary
    skipDefaults : bool, optional
        If true, skip any values that match the default, by default True

    Returns
    -------
    str
        The fixed string representation
    """
    return "_".join(
        recoDict[k]
        for k in recoKeys
        if k in recoDict
        and (not skipDefaults or recoDict[k] != METChainParts_Default[k])
    )


def stringToMETRecoDict(string: str, onlyRecoKeys: bool = False) -> dict[str, Any]:
    """Convert a string to a MET reco dict. Optionally only keep entries contained in recoKeys. 
    Necessary for correctly configuring algorithms as inputs to NN.
    """
    defaults = copy(METChainParts_Default)
    if onlyRecoKeys:
        defaults = {key: defaults[key] for key in recoKeys}
    # Now go through the parts of the string and fill in the dict
    for part in string.split("_"):
        for key, values in METChainParts.items():
            if not isinstance(values, list):
                continue
            if part in values:
                if isinstance(defaults[key], list):
                    defaults[key].append(part)  # type: ignore[attr-defined]
                else:
                    defaults[key] = part
    return defaults


class AlgConfig(ABC):
    """Base class to describe algorithm configurations

    Each individual 'EFrecoAlg' should be described by *one* AlgConfig subclass.
    It must provide its list of required inputs to the constructor and override
    the make_fex method

    The name of fexAlg *must* be self.fexName and the METContainerKey property
    *must* be set to self.outputKey (but this class usually should take care of
    this).

    The subclass must also implement the @classmethod 'algType' which returns
    the EFrecoAlg string that it describes.
    """

    @classmethod
    def algType(cls) -> str:
        """The algorithm that this object configures - this corresponds to the
        EFrecoAlg in the METChainParts dictionary

        Note that no two subclasses of AlgConfig can describe the same algorithm
        (as identified by this string).
        """
        raise NotImplementedError("algType not implemented by subclass!")

    def __init__(self, **recoDict: str):
        """Initialise the base class

        =========
        Arguments
        =========
        recoDict: Pass *all* the keys required for the recoDict
        """

        # Make sure that we got *all* the keys (i.e. the subclass didn't
        # inadvertently steal one of them from us)
        alg_type = self.algType()
        assert all(k in recoDict for k in recoKeys), (
            f"AlgConfig.__init__ for {alg_type} did not receive all the recoKeys"
            " - this suggests a problem in the subclass __init__ method!"
        )
        self.recoDict = copy(recoDict)
        self._suffix = metRecoDictToString(recoDict)

    @property
    def outputKey(self) -> str:
        """The MET container object produced by this algorithm"""
        from TrigEDMConfig.TriggerEDM import recordable

        return recordable("HLT_MET_{}".format(self._suffix))

    @property
    def fexName(self) -> str:
        """The name of the algorithm made by this configuration"""
        return "EFMET_{}".format(self._suffix)

    def interpret_reco_dict(self) -> dict[str, Any]:
        """Return a version of the reco dict with any necessary post-processing"""
        return self.recoDict

    def getMonTool(self, flags):
        """Create the monitoring tool"""
        return getMETMonTool(flags)

    def name_step(self, idx) -> str:
        return f"step{ascii_uppercase[idx]}_{self._suffix}"

    @abstractmethod
    def make_reco_algs(self, flags, **recoDict) -> StepOutput:
        """Create the reconstruction sequences including the FEX

        Returns a list of CAs split by step
        """

    def _append_fex(self, flags, fex, inputs: Optional[StepOutput] = None) -> None:
        """Append the FEX to the output object
        
        Finalizes the FEX by setting the monitoring tool and output name.
        """
        fex.MonTool = self.getMonTool(flags)
        fex.METContainerKey = self.outputKey
        acc = ComponentAccumulator()
        acc.addEventAlgo(fex, primary=True)
        return StepOutput.create(acc, inputs, MET=self.outputKey)

    def make_reco_ca(self, flags):
        """Make the reconstruction sequences for the new JO style"""
        # Retrieve the inputs
        log.verbose("Create inputs for %s", self._suffix)
        steps, inputs = self.inputRegistry.build_steps(
            flags, self._inputs, metFSRoIs, self.recoDict, return_ca=True
        )
        # Create the FEX and add it to the last input sequence
        fex = self.make_fex_accumulator(flags, self.fexName, inputs)
        fex.MonTool = self.getMonTool(flags)
        fex.METContainerKey = self.outputKey
        steps[-1].addEventAlgo(fex)
        return steps

    def make_accumulator_steps(self, flags, chainDict):
        """Make the full accumulator steps"""

        # Get the reco sequences
        reco_sequences = self.make_reco_algs(flags, **self.interpret_reco_dict()).steps
        output_steps = []
        # build up the output steps
        # We have to merge together the CAs containing the reconstruction sequences
        # and the InEventRecoCAs that contain the input makers
        for step_idx, reco_sequence in enumerate(reco_sequences):
            reco_acc = self.inputMakerCA(step_idx)
            sel_acc = None
            step_name = f"Empty_METStep{step_idx}"
            if (
                reco_acc is not None
            ):  # Define reco and hypo algorithms only if reco_acc is not None
                if step_idx == len(reco_sequences) - 1:
                    # If this is the last step we have to add the hypo alg
                    hypo_alg = self.make_hypo_alg()
                    hypo_tool = TrigMETHypoToolFromDict
                else:
                    # Otherwise, we add a passthrough hypo
                    hypo_alg = self.make_passthrough_hypo_alg(step_idx)
                    hypo_tool = streamer_hypo_tool

                # Now merge the reco sequence into the InEventRecoCA
                reco_acc.mergeReco(reco_sequence)

                # Create a selection CA
                sel_acc = SelectionCA("METAthSeq_" + self.name_step(step_idx))
                # Merge in the reconstruction sequence
                sel_acc.mergeReco(reco_acc)
                # Add its hypo alg
                sel_acc.addHypoAlgo(hypo_alg)

                step_name = sel_acc.name

            # Build the menu sequence and create the actual chainStep
            output_steps.append(
                ChainStep(
                    name=step_name,
                    multiplicity=[] if sel_acc is None else [1],
                    chainDicts=[chainDict],
                    Sequences=[]
                    if sel_acc is None
                    else [
                        functools.partial(make_MET_menu_sequence, flags, sel_acc, hypo_tool)                                                
                    ],
                    comboHypoCfg=ComboHypoCfg
                )
            )

        return output_steps
    

    def make_hypo_alg(self):
        """The hypo alg used for this configuration"""
        return CompFactory.TrigMissingETHypoAlg(
            f"METHypoAlg_{self._suffix}", METContainerKey=self.outputKey
        )

    def make_passthrough_hypo_alg(self, step):
        return CompFactory.TrigStreamerHypoAlg(
            "METPassThroughHypo_" + self.name_step(step)
        )

    def inputMakerCA(self, idx):
        """Get the InEventRecoCA for the given step index"""
        from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
        from AthenaConfiguration.ComponentFactory import CompFactory
        from ..CommonSequences.FullScanDefs import trkFSRoI

        name = self.name_step(idx) + "Reco"
        if idx == 0:
            return InEventRecoCA(name, inputMaker=clusterFSInputMaker())
        elif idx == 1:
            return None
        elif idx == 2:
            return InEventRecoCA(
                name,
                inputMaker=CompFactory.InputMakerForRoI(
                    "IM_Jet_TrackingStep",
                    RoITool=CompFactory.ViewCreatorInitialROITool(),
                    RoIs=trkFSRoI,
                ),
            )
        else:
            raise KeyError(f"No input maker for step {idx}")

    @classmethod
    def _get_subclasses(cls):
        """Provides a way to iterate over all subclasses of this class"""
        for subcls in cls.__subclasses__():
            for subsubcls in subcls.__subclasses__():
                yield subsubcls
            yield subcls

    @classmethod
    def fromRecoDict(cls, EFrecoAlg, **recoDict) -> AlgConfig:
        for subcls in cls._get_subclasses():
            if subcls.algType() == EFrecoAlg:
                return subcls(EFrecoAlg=EFrecoAlg, **recoDict)

        raise ValueError("Unknown EFrecoAlg '{}' requested".format(EFrecoAlg))

def make_MET_menu_sequence(flags, sel_acc, hypo_tool):
    return MenuSequenceCA(flags, selectionCA=sel_acc, HypoToolGen=hypo_tool)

# Load all the defined configurations
from . import AlgConfigs

# Make sure that there is an AlgConfig for every EFrecoAlg
AlgConfigs.test_configs()
