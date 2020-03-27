import os
import unittest
import numpy as np
from pathlib import Path
from asmc import (
    HMM,
    DecodingQuantities,
    DecodingParams,
    Data,
    DecodingMode,
    FastSMC,
)


class TestASMCRegression(unittest.TestCase):

    def setUp(self):
        inFileRoot = "FILES/EXAMPLE/exampleFile.n300.array"
        decodingQuantFile = "FILES/DECODING_QUANTITIES" \
            "/30-100-2000.decodingQuantities.gz"
        self.sequenceLength = Data.countHapLines(inFileRoot)
        self.params = DecodingParams(inFileRoot, decodingQuantFile,
                                     doPosteriorSums=True)
        self.decodingQuantities = DecodingQuantities(decodingQuantFile)
        self.data = Data(inFileRoot, self.sequenceLength,
                         self.decodingQuantities.CSFSSamples,
                         self.params.foldData, self.params.usingCSFS)
        self.hmm = HMM(self.data, self.decodingQuantities, self.params,
                       not self.params.noBatches, 1)

    def test_regression(self):
        oldSumOverPairs = np.loadtxt(Path(__file__).parent / 'data' /
                                      'regression_test_original.gz')
        self.hmm.decodeAll(self.params.jobs, self.params.jobInd)
        ret = self.hmm.getDecodingReturnValues()
        self.assertEqual(np.allclose(ret.sumOverPairs, oldSumOverPairs), True)


class TestFastSMCRegression(unittest.TestCase):

    def setUp(self):
        self.file_dir = os.path.join(os.getcwd(), 'FILES', 'FASTSMC_EXAMPLE')
        self.name_prefix = 'out.25.n300.chr2.len30.dens1.disc10-20-2000.demoCEU.mapnorm.array'

        # Create decoding params object with required options
        self.params = DecodingParams()
        self.params.decodingQuantFile = os.path.join(self.file_dir, f'{self.name_prefix}.decodingQuantities.gz')
        self.params.inFileRoot = os.path.join(self.file_dir, self.name_prefix)
        self.params.outFileRoot = os.path.join('/tmp/FastSMCresults')
        self.params.decodingModeString = 'array'
        self.params.decodingMode = DecodingMode.arrayFolded
        self.params.foldData = True
        self.params.usingCSFS = True
        self.params.batchSize = 32
        self.params.recallThreshold = 3
        self.params.min_m = 1.5
        self.params.GERMLINE = True
        self.params.BIN_OUT = False
        self.params.time = 50
        self.params.noConditionalAgeEstimates = True
        self.params.doPerPairMAP = True
        self.params.doPerPairPosteriorMean = True

        self.decodingQuantities = DecodingQuantities(self.params.decodingQuantFile)
        self.sequenceLength = Data.countHapLines(self.params.inFileRoot)

        self.data = Data(self.params.inFileRoot, self.sequenceLength, self.decodingQuantities.CSFSSamples,
                         self.params.foldData, self.params.usingCSFS, self.params.jobInd, self.params.jobs)

        self.hmm = HMM(self.data, self.decodingQuantities, self.params, not self.params.noBatches, 1)
        self.hmm.decodeAll(self.params.jobs, self.params.jobInd)

        self.FastSMC = FastSMC()
        self.FastSMC.run(self.params, self.data, self.hmm)

    def test_regression(self):
        pass
