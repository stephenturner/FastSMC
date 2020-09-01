# this script will run FastSMC on a simulated data as described in the paper (in FILES/FASTSMC_EXAMPLE/)
# parameters can be changed if desired

cd ../ASMC_BUILD_DIR/

./FastSMC_exe --inFileRoot FILES/FASTSMC_EXAMPLE/example \
        --outFileRoot FastSMC_output_example \
        --decodingQuantFile FILES/FASTSMC_EXAMPLE/example.decodingQuantities.gz \
        --mode array \
        --time 50 \
        --min_m 1.5 \
        --segmentLength \
        --GERMLINE \
        --perPairPosteriorMeans \
        --perPairMAP
