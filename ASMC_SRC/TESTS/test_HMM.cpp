//    This file is part of ASMC, developed by Pier Francesco Palamara.
//
//    ASMC is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ASMC is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ASMC.  If not, see <https://www.gnu.org/licenses/>.

#include "catch.hpp"

#include "FileUtils.hpp"
#include <sstream>

// Hack to allow direct testing of private methods
#define private public
#include "HMM.hpp"
#undef private


TEST_CASE("test hmm functions", "[HMM]")
{
  DecodingParams params(
      ASMC_FILE_DIR "/EXAMPLE/exampleFile.n300.array",
      ASMC_FILE_DIR "/DECODING_QUANTITIES/30-100-2000.decodingQuantities.gz");
  DecodingQuantities decodingQuantities(params.decodingQuantFile.c_str());
  int sequenceLength = Data::countHapLines(params.hapsFileRoot.c_str());
  Data data(params.hapsFileRoot.c_str(), sequenceLength, decodingQuantities.CSFSSamples,
      params.foldData, params.usingCSFS);
  HMM hmm(data, decodingQuantities, params, !params.noBatches);

  REQUIRE(data.individuals.size() > 20);

  SECTION("test decode pair summarize")
  {
    PairObservations pairObs = hmm.makePairObs(data.individuals[0], 1, 0, data.individuals[0], 2, 0);
    vector<vector<float>> decodeResult = hmm.decode(pairObs);
    pair<vector<float>, vector<float>> decodeSummary = hmm.decodeSummarize(pairObs);
    // check that the MAP and posterior mean are the same length
    REQUIRE(decodeSummary.first.size() == decodeSummary.second.size());
    REQUIRE(decodeSummary.first.size() == decodeResult[0].size());
  }

  SECTION("test decode pair")
  {
    REQUIRE(hmm.getBatchBuffer().size() == 0);
    hmm.decodePair(0, 9);
    REQUIRE(hmm.getBatchBuffer().size() == 4);
    hmm.decodePair(1, 1);
    REQUIRE(hmm.getBatchBuffer().size() == 5);
  }

  SECTION("test decode pairs")
  {
    REQUIRE(hmm.getBatchBuffer().size() == 0);
    hmm.decodePairs({ 0, 1 }, { 9, 1 });
    REQUIRE(hmm.getBatchBuffer().size() == 5);
  }

  SECTION("test finishDecoding")
  {
    REQUIRE(hmm.getBatchBuffer().size() == 0);
    hmm.decodePair(0, 9);
    REQUIRE(hmm.getBatchBuffer().size() == 4);
    hmm.finishDecoding();
    REQUIRE(hmm.getBatchBuffer().size() == 0);
  }

  SECTION("test fill up buffer")
  {
    // default batch size is 64
    for (int i = 1; i <= 64 / 4; ++i) {
      hmm.decodePair(0, i);
    }

    // buffer should be empty now
    REQUIRE(hmm.getBatchBuffer().size() == 0);
  }

  SECTION("test rounding")
  {
    REQUIRE(hmm.roundMorgans(1e-12f) == Approx(1e-10f));
    REQUIRE(hmm.roundMorgans(0.123f) == Approx(0.123f));
    REQUIRE(hmm.roundMorgans(2.34f) == Approx(2.34f));
    REQUIRE(hmm.roundMorgans(10.25f) == Approx(10.3f));
  }
}
