# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from .powheg_base import PowhegBase
import glob


class PowhegRES(PowhegBase):
    """! Base class for PowhegBox RES processes.

    All RES processes inherit from this class.

    @author James Robinson  <james.robinson@cern.ch>
    """

    def __init__(self, base_directory, executable_name,warning_output = [], info_output = [], error_output = [], **kwargs):
        """! Constructor.

        @param base_directory  path to PowhegBox code.
        @param executable_name folder containing appropriate PowhegBox executable.
        @param is_LO           True if this is a leading-order process.
        @param warning_output list of patterns which if found in the output will be treated as warning in the log.
        @param error_output list of patterns which if found in the output will be treated as error in the log.
        @param info_output list of patterns which if found in the output will be treated as info in the log.
        """
        super(PowhegRES, self).__init__(base_directory, "POWHEG-BOX-RES", executable_name, warning_output = warning_output, info_output = info_output, error_output = error_output, **kwargs)

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
    def default_PDFs_nnlo(self):
        """! Default PDFs for this process."""
        __PDF_list = list(range(261000, 261101))    # NNPDF30_nnlo_as_0118 central with eigensets
        __PDF_list += [270000, 269000]              # NNPDF30_nnlo_as_0119 and NNPDF30_nnlo_as_0117
        #  __PDF_list += [303200]                      # NNPDF30_nnlo_as_0118_hessian
        __PDF_list += [27400]                       # MSHT20nnlo_as118
        __PDF_list += [14000]                       # CT18NNLO
        __PDF_list += [304400]                      # NNPDF31_nnlo_as_0118_hessian
        __PDF_list += [331500]                      # NNPDF40_nnlo_as_01180_hessian
        __PDF_list += [14200, 14300, 14100]         # CT18ANNLO, CT18XNNLO and CT18ZNNLO
        __PDF_list += list(range(93300, 93343))     # PDF4LHC21_40_pdfas with eigensets
        #  __PDF_list += [338500, 338520, 338540]      # NNPDF40MC_lo_as_01180, NNPDF40MC_nlo_as_01180, NNPDF40MC_nnlo_as_01180
        return __PDF_list

    @property
    def default_PDFs_nnlo_nf_4(self):
        """! Default PDFs for this process."""
        __PDF_list = list(range(261400, 261501))    # NNPDF30_nnlo_as_0118_nf_4 central with eigensets
        __PDF_list += [266400, 265400]              # NNPDF30_nnlo_as_0119_nf_4 and NNPDF30_nnlo_as_0117_nf_4
        #  __PDF_list += [303200]                   # NNPDF30_nnlo_as_0118_hessian
        __PDF_list += [28300]                       # MSHT20nnlo_nf4
        __PDF_list += [14084]                       # CT18NNLO_NF4
        __PDF_list += [320900]                      # NNPDF31_nnlo_as_0118_nf_4
        __PDF_list += [334300]                      # NNPDF40_nnlo_as_01180_nf_4
        __PDF_list += list(range(93700, 93743))     # PDF4LHC21_40_pdfas_nf4 with eigensets
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
            "allborn_equiv",
            "FlavRegList",
            "mint*.top",
            "parameters.ol",
            "pwg*.top",
            "pwgboundviolations*.dat",
            "pwgcounters*.dat",
            "pwgseeds.dat",
            "pwhg_checklimits",
            "sigreal_btl0_equiv",
            "sigregular_equiv",
            "sigvirtual_equiv"
        ]


    @property
    def integration_file_names(self):
        """! Wildcarded list of integration files that might be created by this process."""
        return [
            "pwg*upb*.dat",
            "pwg*xgrid*.dat",
            "pwgfullgrid*.dat",
            "pwggrid*.dat",
            "pwgubound*.dat",
            "pwg*stat.dat",
        ]

    @property
    def mandatory_integration_file_names(self):
        """! Wildcarded list of integration files that are needed for this process."""
        return self.integration_file_names

    @property
    def powheg_version(self):
        """! Version of PowhegBox process."""
        return "RES"

    def stage_is_completed(self, stage):
        """! Set whether the specified POWHEG-BOX generation stage is complete."""
        if stage == 1:
            required_files = ["pwg*xgrid*.dat"]
        elif stage == 2:
            required_files = ["pwg*upb*.dat", "pwggrid*.dat"]
        elif stage == 3:
            required_files = ["pwgfullgrid*.dat", "pwgubound*.dat"]
        else:
            return False

        # Check that required files have been found
        for required_file in required_files:
            if not glob.glob(required_file):
                return False
        return True
