# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory

# This configuration fragment contains the baseline Sherpa settings
# and other higher-level configs that the users will call to create
# the CA object in the setupProcess method of their sample config class
# in the top-level jO.

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("SherpaConfig")

# Helper functions from SherpaUtils
from Sherpa_i.SherpaUtils import (
    build_sherpa3_base_fragment,
    validate_sherpa3_yaml_fragment,
    write_pretty_fragment
)


def SherpaBaseCfg(flags, name="Sherpa_i", **kwargs):
    """
    Public methods for Sherpa CA configuration fragment, 
    migrated from Sherpa_i/share/common/Base_Fragment.py.
    This is the the method that users should call (if needed)
    in their jO. We don't want them to change the BaseFragment 
    so we pop it from kwargs before calling the internal function.
    """
    kwargs.pop("BaseFragment", None) 

    return _SherpaBaseCfg(flags, name=name, **kwargs)


def _SherpaBaseCfg(flags, name="Sherpa_i", _base_fragment="", **kwargs):
    """
    Internal Sherpa base fragment builder.
    """

    kwargs.setdefault("PluginCode", "")

    # Build the Base.yaml fragment from an optional internal prefix plus Sherpa3 defaults.
    base_fragment = _base_fragment
    if base_fragment and not base_fragment.endswith("\n"):
        base_fragment += "\n"
    kwargs["BaseFragment"] = base_fragment + build_sherpa3_base_fragment(flags)
    validate_sherpa3_yaml_fragment("BaseFragment", kwargs["BaseFragment"])
    if "RunCard" in kwargs:
        validate_sherpa3_yaml_fragment("RunCard", kwargs["RunCard"])

    # Create CA object and add Sherpa algorithm
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(CompFactory.Sherpa_i(name, **kwargs))

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Sherpa"]))

    return ca


def Sherpa3_PDF4LHC21_Cfg(flags, **kwargs):
    """Fragment for setting up Sherpa 3 with the PDF4LHC21 tune"""
    
    from os import environ
    sherpa_version = environ.get("SHERPAVER")
    if sherpa_version is None:
        raise RuntimeError("SHERPAVER is not set in the environment.")
    if not sherpa_version.startswith("3."):
        raise RuntimeError("Sherpa3_PDF4LHC21_Cfg requires Sherpa 3.")

    # Nominal PDF settings
    pdf_fragment = write_pretty_fragment(
        """
        PDF_LIBRARY: LHAPDFSherpa
        USE_PDF_ALPHAS: 1
        PDF_SET: PDF4LHC21_40_pdfas
        """
    )

    # Enable PDF variations by default
    pdf_fragment += write_pretty_fragment(
        """
        PDF_VARIATIONS:
        - PDF4LHC21_40_pdfas*
        - MSHT20nnlo_as118
        - CT18NNLO_as_0118
        - NNPDF31_nnlo_as_0118_hessian
        - NNPDF40_nnlo_as_01180_hessian
        - CT18ANNLO
        - CT18XNNLO
        - CT18ZNNLO
        """
    )

    # Append PDF settings through the internal BaseFragment prefix and create the CA object.
    ca = _SherpaBaseCfg(flags, _base_fragment=pdf_fragment, **kwargs)

    # Broadcast tune to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Tune="PDF4LHC21"))

    return ca
