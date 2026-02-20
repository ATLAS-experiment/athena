# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from .powheg_base import PowhegBase


class PowhegV1(PowhegBase):
    """! Base class for PowhegBox V1 processes.

    All V1 processes inherit from this class.

    @author James Robinson  <james.robinson@cern.ch>
    """

    def __init__(self, base_directory, executable_name, **kwargs):
        """! Constructor.

        @param base_directory  path to PowhegBox code.
        @param executable_name folder containing appropriate PowhegBox executable.
        @param warning_output list of patterns which if found in the output will be treated as warning in the log.
        @param error_output list of patterns which if found in the output will be treated as error in the log.
        @param info_output list of patterns which if found in the output will be treated as info in the log.
        """
        super(PowhegV1, self).__init__(base_directory, "POWHEG-BOX-NoUserProcesses", executable_name, is_reweightable=False, warning_output = [], info_output = [], error_output = [], **kwargs)

    @property
    def default_PDFs(self):
        """! Default PDFs for this process."""
        __PDF_list = list(range(260000, 260101))    # NNPDF30_nlo_as_0118 central with eigensets
        __PDF_list += [266000, 265000]              # NNPDF30_nlo_as_0119 and NNPDF30_nlo_as_0117
        __PDF_list += [303200]                      # NNPDF30_nnlo_as_0118_hessian
        __PDF_list += [27400, 27100]                # MSHT20nnlo_as118, MSHT20nlo_as118
        __PDF_list += [14000, 14400]                # CT18NNLO, CT18NLO
        __PDF_list += [304400, 304200]              # NNPDF31_nnlo_as_0118_hessian, NNPDF31_nlo_as_0118_hessian
        __PDF_list += [331500, 331100]              # NNPDF40_nnlo_as_01180_hessian, NNPDF40_nlo_as_01180
        __PDF_list += [14200, 14300, 14100]         # CT18ANNLO, CT18XNNLO and CT18ZNNLO
        __PDF_list += list(range(93300, 93343))     # PDF4LHC21_40_pdfas with eigensets
        __PDF_list += [338500, 338520, 338540]      # NNPDF40MC_lo_as_01180, NNPDF40MC_nlo_as_01180, NNPDF40MC_nnlo_as_01180
        return __PDF_list

    @property
    def default_scales(self):
        """! Default scale variations for this process."""
        return [[1.0, 1.0, 1.0, 0.5, 0.5, 2.0, 2.0],\
                [1.0, 0.5, 2.0, 0.5, 1.0, 1.0, 2.0]]

    @property
    def files_for_cleanup(self):
        """! Wildcarded list of files created by this process that can be deleted."""
        return [
            "FlavRegList",
            "pwg*.top",
            "pwgseeds.dat",
            "pwgcounters*.dat",
            "pwgubsigma.dat",
            "pwhg_checklimits"
        ]

    @property
    def integration_file_names(self):
        """! Wildcarded list of integration files that might be created by this process."""
        return [
            "pwgbtildeupb*.dat",
            "pwgfullgrid.dat",
            "pwggrid*.dat",
            "pwgremnupb*.dat",
            "pwgstat*.dat",
            "pwgubound*.dat",
            "pwgxgrid*.dat"
        ]

    @property
    def mandatory_integration_file_names(self):
        """! Wildcarded list of integration files that are needed for this process."""
        """! If some of the patterns don't match any files before running, a warning will be made to inform that no pre-made integration grid will be used."""
        return self.integration_file_names

    @property
    def powheg_version(self):
        """! Version of PowhegBox process."""
        return "V1"
