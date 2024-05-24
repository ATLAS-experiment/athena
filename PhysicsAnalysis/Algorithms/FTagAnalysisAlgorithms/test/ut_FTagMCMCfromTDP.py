import unittest

from FTagAnalysisAlgorithms.FTagAnalysisConfig import  parseTDPdatabase
from PathResolver import PathResolver


class TestParseTDPDatabase(unittest.TestCase):

    def test_parseTDPdatabase(self):

        # Run 2 tests
        reference = {
            410470: 'pythia8',
            700168: 'sherpa2210',
            600020: 'herwigpp713',
            502957: 'amcatnlopythia8',
            504337: 'herwigpp721',
            999999: None,
        }
        for dsid in reference.keys():
            result = parseTDPdatabase('dev/AnalysisTop/TopDataPreparation/XSection-MC16-13TeV_JESinfo.data',
                                      dsid)
            self.assertEqual(result, reference[dsid])

        # Run 3 tests
        reference = {
            601229: 'pythia8',
            700660: 'sherpa2212',
            513105: 'amcatnlopythia8',
            999999: None,
        }
        for dsid in reference.keys():
            result = parseTDPdatabase('dev/AnalysisTop/TopDataPreparation/XSection-MC21-13p6TeV.data',
                                      dsid)
            self.assertEqual(result, reference[dsid])

if __name__ == '__main__':
    unittest.main()
