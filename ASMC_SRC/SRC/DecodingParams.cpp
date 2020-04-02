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


#include <exception>
#include <iostream>
#include <string>
//#include <sys/types.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include "DecodingParams.hpp"
#include "Types.hpp"

using namespace std;

DecodingParams::DecodingParams()
      : inFileRoot("")
      , decodingQuantFile("")
      , outFileRoot(inFileRoot)
      , jobs(1)
      , jobInd(1)
      , decodingModeString("array")
      , decodingSequence(false)
      , usingCSFS(true)
      , compress(false)
      , useAncestral(false)
      , skipCSFSdistance(0.f)
      , noBatches(false)
      , doPosteriorSums(false)
      , doPerPairPosteriorMean(false)
      , expectedCoalTimesFile("")
      , withinOnly(false)
      , doMajorMinorPosteriorSums(false)
      , doPerPairMAP(false)
  {
  }

DecodingParams::DecodingParams(string _inFileRoot,
        string _decodingQuantFile,
        string _outFileRoot,
        int _jobs,
        int _jobInd,
        string _decodingModeString,
        bool _decodingSequence,
        bool _usingCSFS,
        bool _compress,
        bool _useAncestral,
        float _skipCSFSdistance,
        bool _noBatches,
        bool _doPosteriorSums,
        bool _doPerPairPosteriorMean,
        string _expectedCoalTimesFile,
        bool _withinOnly,
        bool _doMajorMinorPosteriorSums
        )
      : inFileRoot(_inFileRoot)
      , decodingQuantFile(_decodingQuantFile)
      , outFileRoot(_outFileRoot)
      , jobs(_jobs)
      , jobInd(_jobInd)
      , decodingModeString(_decodingModeString)
      , decodingSequence(_decodingSequence)
      , usingCSFS(_usingCSFS)
      , compress(_compress)
      , useAncestral(_useAncestral)
      , skipCSFSdistance(_skipCSFSdistance)
      , noBatches(_noBatches)
      , doPosteriorSums(_doPosteriorSums)
      , doPerPairPosteriorMean(_doPerPairPosteriorMean)
      , expectedCoalTimesFile(_expectedCoalTimesFile)
      , withinOnly(_withinOnly)
      , doMajorMinorPosteriorSums(_doMajorMinorPosteriorSums)
      , doPerPairMAP(false)
  {
     if(!processOptions()) throw std::exception();
  }


bool DecodingParams::processCommandLineArgs(int argc, char *argv[]) {

  namespace po = boost::program_options;

  po::options_description options;
  options.add_options()
  ("inFileRoot", po::value<string>(&inFileRoot)->required(),
   "Prefix of hap|haps|hap.gz|haps.gz and sample|samples file")
  ("decodingQuantFile", po::value<string>(&decodingQuantFile),
   "Decoding quantities file")
  ("jobs", po::value<int>(&jobs)->default_value(0),
   "Number of jobs being done in parallel")
  ("jobInd", po::value<int>(&jobInd)->default_value(0),
   "Job index (1..jobs)")
  ("outFileRoot", po::value<string>(&outFileRoot),
   "Output file for sum of posterior distribution over pairs (default: --inFileRoot argument)")
  ("mode", po::value<string>(&decodingModeString)->default_value("array"),
   "Decoding mode. Choose from {sequence, array}.")
  ("compress", po::bool_switch(&compress)->default_value(false),
   "Compress emission to binary (no CSFS)")
  // note: currently flipping major/minor when reading data. Instead, assume it's correctly coded to begin with
  ("useAncestral", po::bool_switch(&useAncestral)->default_value(false),
   "Assume ancestral alleles are coded as 1 in input (will assume 1 = minor otherwise)")
  // for debugging and pedagogical reasons.
 // ("noBatches", po::bool_switch(&noBatches)->default_value(false),
 //  "Do not decode in batches (for debugging, will remove)")
  ("skipCSFSdistance", po::value<float>(&skipCSFSdistance)->default_value(0.),
   "Genetic distance between two CSFS emissions")
  // main tasks
  ("majorMinorPosteriorSums", po::bool_switch(&doMajorMinorPosteriorSums)->default_value(false),
   "Output file for sum of posterior distribution over pairs, partitioned by major/minor alleles. O(sitesxstates) output")
  ("posteriorSums", po::bool_switch(&doPosteriorSums)->default_value(false),
   "Output file for sum of posterior distribution over pairs. O(sitesxstates) output");
  // ("perPairMAP", po::bool_switch(&doPerPairMAP)->default_value(false),
  //  "output per-pair MAP at each site. O(sitesxsamples^2) output")
  // ("perPairPosteriorMeans", po::value<string>(&expectedCoalTimesFile),
  //  "output per-pair posterior means at each site. O(sitesxsamples^2) output")
  // ("withinOnly", po::bool_switch(&withinOnly)->default_value(false),
  //  "Only decode pairs within unphased individuals");

  po::options_description visible("Options");
  visible.add(options);

  po::options_description all("All options");
  all.add(options);
  all.add_options()
  ("bad-args", po::value< vector <string> >(), "bad args")
  ;

  po::positional_options_description positional_desc;
  positional_desc.add("bad-args", -1); // for error-checking command line

  po::variables_map vm;
  po::command_line_parser cmd_line(argc, argv);
  cmd_line.options(all);
  cmd_line.style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing);
  cmd_line.positional(positional_desc);

  try {
    po::store(cmd_line.run(), vm);

    po::notify(vm); // throws an error if there are any problems

    if (vm.count("bad-args")) {
      cerr << "ERROR: Unknown options:";
      vector <string> bad_args = vm["bad-args"].as< vector <string> >();
      for (uint i = 0; i < bad_args.size(); i++) cerr << " " << bad_args[i];
      cerr << endl;
      return false;
    }
  } catch (po::error &e) {
    cerr << "ERROR: " << e.what() << endl << endl;
    cerr << options << endl;
    return false;
  }

  if(processOptions()) {
    if (!doPosteriorSums && !doPerPairMAP && !doPerPairPosteriorMean && !doMajorMinorPosteriorSums) {
         cerr << "ERROR: At least one of --posteriorSums, --majorMinorPosteriorSums, must be specified"
              << endl;
         return false;
    }
  }
  else
      return false;
  return true;
}

bool DecodingParams::processCommandLineArgsFastSMC(int argc, char *argv[]) {
  namespace po = boost::program_options;

  FastSMC = true;

  string decodingModeString;

  po::options_description options;
  options.add_options()
      ("inFileRoot", po::value<string>(&inFileRoot)->required(),
       "Prefix of hap|haps|hap.gz|haps.gz and sample|samples file.")
      ("map", po::value<string>(&map)->required(),
       "Genetic map file.")
      ("outFileRoot", po::value<string>(&outFileRoot)->required(),
       "Output file for sum of posterior distribution over pairs.")
      ("decodingQuantFile", po::value<string>(&decodingQuantFile),
       "Decoding quantities file")
      ("mode", po::value<string>(&decodingModeString)->default_value("array"),
       "Decoding mode. Choose from {sequence, array}. [default = array]")
      ("time", po::value<int>(&time)->default_value(100),
       "Time threshold to define IBD in number of generations. [default = 100] ")
      ("jobs", po::value<int>(&jobs)->default_value(1),
       "number of jobs being done in parallel. [default = 1]")
      ("jobInd", po::value<int>(&jobInd)->default_value(1),
       "job index (1..jobs). [default = 1]")
      ("bin", po::bool_switch(&BIN_OUT)->default_value(false),
       "Binary output [default off]")
      ("batchSize", po::value<int>(&batchSize)->default_value(16),
       "Batch size [default = 16]")
      ("recall", po::value<int>(&recallThreshold)->default_value(3),
       "Recall level from 0 to 3 (higher value means higher recall). [default = 3]")

      // TASKS
      ("perPairMAP", po::bool_switch(&doPerPairMAP)->default_value(false),
       "Output per-pair MAP for each IBD segment. [default 0/off]")
      ("perPairPosteriorMeans", po::bool_switch(&doPerPairPosteriorMean)->default_value(false),
       "Output per-pair posterior means for each IBD segment. [default 0/off]")
      ("noConditionalAgeEstimates", po::bool_switch(&noConditionalAgeEstimates)->default_value(false),
       "Do not condition the age estimates on the TMRCA being between present time and t generations ago (where t is the time threshold). [default 0/off]")
      ("withinOnly", po::bool_switch(&withinOnly)->default_value(false),
       "Only decode pairs within unphased individuals. [default 0/off]")

      // TODO: currently flipping major/minor when reading data. Instead, assume it's correctly coded to begin with
      ("useAncestral", po::bool_switch(&useAncestral)->default_value(false),
       "Assume ancestral alleles are coded as 1 in input (will assume 1 = minor otherwise). [default 0/off]")
      ("compress", po::bool_switch(&compress)->default_value(false),
       "Compress emission to binary (no CSFS). [default 0/off]")

      // TODO: for debug. remove?
      ("noBatches", po::bool_switch(&noBatches)->default_value(false),
       "Do not decode in batches. [default 0/off]")
      ("skipCSFSdistance", po::value<float>(&skipCSFSdistance)->default_value(std::numeric_limits<float>::quiet_NaN()),
       "Genetic distance between two CSFS emissions")

      //GERMLINE options
      ("GERMLINE", po::bool_switch(&GERMLINE)->default_value(false),
       "Use of GERMLINE to pre-process IBD segments [default 0/off]")
      ("min_m", po::value<float>(&min_m)->default_value(1.0),
       "Minimum match length (in cM). [default = 1.0]")
      ("skip", po::value<float>(&skip)->default_value(0.0),
       "Skip words with (seeds/samples) less than this value [default 0.0]")
      ("min_maf", po::value<float>(&min_maf)->default_value(0.0),
       "Minimum minor allele frequency [default 0.0]")
      ("gap", po::value<int>(&gap)->default_value(1),
       "Allowed gaps [default 1]")
      ("max_seeds", po::value<int>(&max_seeds)->default_value(0),
       "Dynamic hash seed cutoff [default 0/off]");

  po::options_description visible("Options");
  visible.add(options);

  po::options_description all("All options");
  all.add(options);
  all.add_options()
      ("bad-args", po::value< vector <string> >(), "bad args")
      ;

  po::positional_options_description positional_desc;
  positional_desc.add("bad-args", -1); // for error-checking command line

  po::variables_map vm;
  po::command_line_parser cmd_line(argc, argv);
  cmd_line.options(all);
  cmd_line.style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing);
  cmd_line.positional(positional_desc);

  try {
    po::store(cmd_line.run(), vm);

    po::notify(vm); // throws an error if there are any problems

    if (vm.count("bad-args")) {
      cerr << "ERROR: Unknown options:";
      vector <string> bad_args = vm["bad-args"].as< vector <string> >();
      for (uint i = 0; i < bad_args.size(); i++) cerr << " " << bad_args[i];
      cerr << endl;
      return false;
    }

    if (GERMLINE) {
      if (withinOnly) {
        cerr << "--GERMLINE & --withinOnly cannot be used together. Please remove one of the two flags." << endl;
        exit(1);
      }
      if (time <= 0) {
        cerr << "--time must be a positive integer." << endl;
        exit(1);
      }
    }

    if ( batchSize == 0 || batchSize%8 != 0) {
      cerr << "--batchSize must be strictly positive and a multiple of 8." << endl;
      exit(1);
    }

    if (compress) {
      if (useAncestral) {
        cerr << "--compress & --useAncestral cannot be used together. A compressed emission cannot use ancestral allele information." << endl;
        exit(1);
      }
      if (!isnan(skipCSFSdistance)) {
        cerr << "--compress & --skipCSFSdistance cannot be used together. --compress is a shorthand for --skipCSFSdistance Infinity." << endl;
        exit(1);
      }
      skipCSFSdistance = std::numeric_limits<float>::infinity();
    }
    else {
      if (isnan(skipCSFSdistance)) {
        // default: use CSFS at all sites
        skipCSFSdistance = 0.f;
      }
    }

    if (skipCSFSdistance != std::numeric_limits<float>::infinity()) {
      usingCSFS = true;
    }

    boost::algorithm::to_lower(decodingModeString);
    if (decodingModeString == string("sequence")) {
      decodingSequence = true;
      if (useAncestral) {
        decodingMode = DecodingMode::sequence;
        foldData = false;
      }
      else {
        decodingMode = DecodingMode::sequenceFolded;
        foldData = true;
      }
    } else if (decodingModeString == string("array")) {
      decodingSequence = false;
      if (useAncestral) {
        decodingMode = DecodingMode::array;
        foldData = false;
      }
      else {
        decodingMode = DecodingMode::arrayFolded;
        foldData = true;
      }
    } else {
      cerr << "ERROR. Unknown decoding mode: " << decodingModeString << endl;
      exit(1);
    }

    if (decodingQuantFile.empty()) {
      cout << "Setting --decodingQuantFile to --inFileRoot + .decodingQuantities.bin" << endl;
      decodingQuantFile = inFileRoot + ".decodingQuantities.bin";
    }

    if ((jobs == 0) != (jobInd == 0)) {
      cerr << "ERROR: --jobs and --jobInd must either both be set or both be unset" << endl;
      return false;
    }

    if (jobs == 0) {
      jobs = 1;
      jobInd = 1;
    }

    if (jobInd <= 0 || jobInd > jobs || jobs <= 0) {
      cerr << "ERROR: --jobInd must be between 1 and --jobs inclusive" << endl;
      return false;
    }

    bool valid_job = false;
    int x = 1;
    int u = 1;
    int prev_u = u;
    for (int i = 0; i < 200 ; i++) {
      if ( u == jobs ) {
        valid_job = true;
        break;
      } else if ( u > jobs ) {
        break;
      }
      x = x + 2;
      prev_u = u ;
      u = u + x;
    }

    if ( !valid_job ) {
      cerr << "ERROR: jobs value is incorrect. You should use either " << prev_u << " or " << u << endl;
      return false;
    }

    if ( recallThreshold < 0 || recallThreshold > 3 ) {
      cerr << "ERROR: --recall must be between 0 and 3. " << prev_u << " or " << u << endl;
      return false;
    }

    if (outFileRoot.empty()) {
      outFileRoot = inFileRoot;
      if (jobs > 0) {
        outFileRoot += "." + std::to_string(jobInd) + "-" + std::to_string(jobs);
      }
    }

//    inFileRoot = inFileRoot;

  } catch (po::error &e) {
    cerr << "ERROR: " << e.what() << endl << endl;
    cerr << options << endl;
    return false;
  }

  cout << endl;
  cout << "---------------------------" << endl;
  cout << "        ASMC OPTIONS       " << endl;
  cout << "---------------------------" << endl;

  cout << "Input will have prefix : " << inFileRoot << endl;
  cout << "Map file is : " << map << endl;
  cout << "Decoding quantities file : " << decodingQuantFile << endl;
  cout << "Output will have prefix : " << outFileRoot << "." << jobInd << "." << jobs;
  if (GERMLINE) {
    cout << ".gasmc";

  } else {
    cout << ".asmc";
  }
  if (BIN_OUT) {
    cout << ".bibd" << endl;
  } else {
    cout << ".ibd.gz" << endl;
  }
  cout << "Binary output ? " << BIN_OUT << endl;
  cout << "Time threshold to define IBD in generations : " << time << endl;
  cout << "Use batches ? " << !noBatches << endl;
  if ( !noBatches ) { cout << "Batch size : " << batchSize << endl; }
  cout << "Running job " << jobInd << " of " << jobs << endl;
  cout << "Recall level " << recallThreshold << endl;
  cout << "skipCSFSdistance is " << skipCSFSdistance << endl;
  cout << "compress ? " << compress << endl;
  cout << "useAncestral ? " << useAncestral << endl;
  cout << "doPerPairPosteriorMean ? " << doPerPairPosteriorMean << endl;
  cout << "doPerPairMAP ? " << doPerPairMAP << endl;
  cout << "noConditionalAgeEstimates ? " << noConditionalAgeEstimates << endl;
  cout << "Use GERMLINE as a preprocessing step ? " << GERMLINE << endl;

  if (GERMLINE) {
    cout << endl;
    cout << "---------------------------" << endl;
    cout << "      GERMLINE OPTIONS     " << endl;
    cout << "---------------------------" << endl;
    cout << "Minimum match length (in cM) : " << min_m << endl;
    cout << "Skipping words with (seeds/samples) less than " << skip << endl;
    cout << "Minimum minor allele frequency : " << min_maf << endl;
    cout << "Allowed gaps " << gap << endl;
    cout << "Dynamic hash seed cutoff : " << max_seeds << endl;
  }

  return true;
}

bool DecodingParams::processOptions() {

    if (compress) {
      if (useAncestral) {
        cerr << "--compress & --useAncestral cannot be used together. A compressed emission cannot use ancestral allele information." << endl;
        exit(1);
      }
      if (!isnan(skipCSFSdistance)) {
        cerr << "--compress & --skipCSFSdistance cannot be used together. --compress is a shorthand for --skipCSFSdistance Infinity." << endl;
        exit(1);
      }
      skipCSFSdistance = std::numeric_limits<float>::infinity();
    }
    else {
      if (isnan(skipCSFSdistance)) {
        // default: use CSFS at all sites
        skipCSFSdistance = 0.f;
      }
    }

    if (!expectedCoalTimesFile.empty()) {
      doPerPairPosteriorMean = true;
    }

    if (skipCSFSdistance != std::numeric_limits<float>::infinity()) {
      usingCSFS = true;
    }

    boost::algorithm::to_lower(decodingModeString);
    if(decodingModeString == string("sequence"))
        decodingModeOverall = DecodingModeOverall::sequence;
    else if(decodingModeString == string("array"))
        decodingModeOverall = DecodingModeOverall::array;
    else {
       cerr << "Decoding mode should be one of {sequence, array}";
       return false;
    }

    if (decodingModeOverall == DecodingModeOverall::sequence) {
      decodingSequence = true;
      if (useAncestral) {
        decodingMode = DecodingMode::sequence;
        foldData = false;
      }
      else {
        decodingMode = DecodingMode::sequenceFolded;
        foldData = true;
      }
    } else if (decodingModeOverall == DecodingModeOverall::array) {
      decodingSequence = false;
      if (useAncestral) {
        decodingMode = DecodingMode::array;
        foldData = false;
      }
      else {
        decodingMode = DecodingMode::arrayFolded;
        foldData = true;
      }
    } else {
      cerr << "ERROR. Unknown decoding mode: " << decodingModeString << endl;
      exit(1);
    }

    if (decodingQuantFile.empty()) {
      cout << "Setting --decodingQuantFile to --inFileRoot + .decodingQuantities.bin" << endl;
      decodingQuantFile = inFileRoot + ".decodingQuantities.bin";
    }

    if ((jobs == 0) != (jobInd == 0)) {
      cerr << "ERROR: --jobs and --jobInd must either both be set or both be unset" << endl;
      return false;
    }

    if (jobs == 0) {
      jobs = 1;
      jobInd = 1;
    }

    if (jobInd <= 0 || jobInd > jobs) {
      cerr << "ERROR: --jobInd must be between 1 and --jobs inclusive" << endl;
      return false;
    }
    if (outFileRoot.empty()) {
      outFileRoot = inFileRoot;
      if (jobs > 0) {
        outFileRoot += "." + std::to_string(jobInd) + "-" + std::to_string(jobs);
      }
    }
   return true;
}

