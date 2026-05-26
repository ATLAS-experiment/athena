# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from enum import Enum

class MadGraphPDFSets(Enum):
    """Enum-class to store strongly-typed PDF sets for MadGraph
    CA configuration fragments."""

    NNPDF30NLOnf4 = "NNPDF30NLOnf4"

    def get_pdf_set_from_enum(self):
        """Return a new PDF-setting dictionary for this preset."""
        if self is MadGraphPDFSets.NNPDF30NLOnf4:
            return {
                # NNPDF nf 4 not available as Hessian PDF
                "central_pdf": 260400,  # NNPDF30_nlo_as_0118_nf_4
                "pdf_variations": [260400, 93700],  # NNPDF30_nlo_as_0118_nf_4 + PDF4LHC21_40_pdfas_nf4
                "alternative_pdfs": [
                    266400,  # NNPDF30_nlo_as_0119_nf_4
                    265400,  # NNPDF30_nlo_as_0117_nf_4
                    261400,  # NNPDF30_nnlo_as_0118_nf_4
                    28300,   # MSHT20nnlo_nf4
                    27700,   # MSHT20nlo_nf4
                    320900,  # NNPDF31_nnlo_as_0118_nf_4
                    320500,  # NNPDF31_nlo_as_0118_nf_4
                    334300,  # NNPDF40_nnlo_as_01180_nf_4
                    334700,  # NNPDF40_nlo_as_01180_nf_4
                    13191,   # CT14nlo_NF4
                    25510,   # MMHT2014nlo68clas118_nf4
                    # no CT18 nf4 PDFs available
                ],
                "scale_variations": [0.5, 1.0, 2.0],
            }
        raise ValueError(f"Unsupported MadGraphPDFSets value: {self}")


def get_pdf_set(pdf_set):
    """Return the MadGraph PDF setting for a strongly-typed preset."""
    if not isinstance(pdf_set, MadGraphPDFSets):
        raise TypeError(
            f"pdf_set must be a MadGraphPDFSets value, got {type(pdf_set).__name__}."
        )
    return pdf_set.get_pdf_set_from_enum()
