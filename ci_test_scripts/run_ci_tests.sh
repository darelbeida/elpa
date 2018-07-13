#!/bin/bash
set -e
set -x

#some defaults
makeTasks=1
mpiTasks=2
matrixSize=150
nrEV=$matrixSize
blockSize=16
ompThreads=1
configueArg=""
skipStep=0
batchCommand=""
interactiveRun="yes"

function usage() {
	cat >&2 <<-EOF

		Call all the necessary steps to perform an ELPA CI test

		Usage:
		  run_ci_tests [-c configure arguments] [-j makeTasks] [-h] [-t MPI Tasks] [-m matrix size] [-n number of eigenvectors] [-b block size] [-o OpenMP threads] [-s skipStep] [-q submit command] [-i interactive run]

		Options:
		 -c configure arguments
		    Line of arguments passed to configure call
		 -t MPI Tasks
		    Number of MPI processes used during test runs of ELPA tests

		 -m Matrix size
		    Size of the mxm matrix used during runs of ELPA tests

		 -n Number of eigenvectors
		    Number of eigenvectors to be computed during runs of ELPA tests

		 -b block size
		    Block size of block-cyclic distribution during runs of ELPA tests

		 -o OpenMP threads
		    Number of OpenMP threads used during runs of ELPA tests

		 -j makeTaks
		    Number of processes make should use during build (default 1)

		 -s skipStep
		    Skip the test run if 1 (default 0)

		 -q submit command
		    Job steps will be submitted via command to a batch system (default no submission)

		 -i interactive_run
		    if "yes" NO no batch command will be triggered

		 -h
		    Print this help text
	EOF
}


while getopts "c:t:j:m:n:b:o:s:q:i:h" opt; do
	case $opt in
		j)
			makeTasks=$OPTARG;;
		t)
			mpiTasks=$OPTARG;;
		m)
			matrixSize=$OPTARG;;
		n)
			nrEV=$OPTARG;;
		b)
			blockSize=$OPTARG;;
		o)
			ompThreads=$OPTARG;;
		c)
			configureArgs=$OPTARG;;
		s)
			skipStep=$OPTARG;;
		q)
			batchCommand=$OPTARG;;
		i)
			interactiveRun=$OPTARG;;
		:)
			echo "Option -$OPTARG requires an argument" >&2;;
		h)
			usage
			exit 1;;
		*)
			exit 1;;
	esac
done

if [ $skipStep -eq 1 ]
then
  echo "Skipping the test since option -s has been specified"
  exit 0
else
  if [ "$batchCommand" == "srun" ]
  then
    if [ "$interactiveRun" == "no" ]
    then
      echo "Running with $batchCommand with $SRUN_COMMANDLINE_CONFIGURE"
#      $batchCommand --ntasks-per-core=1 --ntasks=1 --cpus-per-task=1 $SRUN_COMMANDLINE_CONFIGURE bash -c ' {source /etc/profile.d/modules.sh && source ./ci_test_scripts/ci-env-vars && eval  ./configure $configureArgs; }'
      $batchCommand --ntasks-per-core=1 --ntasks=1 --cpus-per-task=1 $SRUN_COMMANDLINE_CONFIGURE ./ci_test_scripts/configure_step.sh "$configureArgs"

      if [ $? -ne 0 ]; then cat config.log && exit 1; fi
      $batchCommand --ntasks-per-core=1 --ntasks=1 --cpus-per-task=8 $SRUN_COMMANDLINE_BUILD ./ci_test_scripts/build_step.sh $makeTasks
      if [ $? -ne 0 ]; then exit 1; fi
      $batchCommand --ntasks-per-core=1 --ntasks=1 --cpus-per-task=2 $SRUN_COMMANDLINE_RUN ./ci_test_scripts/test_step.sh $mpiTasks $ompThreads "TEST_FLAGS=\" $matrixSize $nrEV $blockSize \" "
      if [ $? -ne 0 ]; then exit 1; fi

      grep -i "Expected %stop" test-suite.log && exit 1 || true ;
      if [ $? -ne 0 ]; then exit 1; fi
    else
    #eval ./configure $configureArgs
    ./ci_test_scripts/configure_step.sh "$configureArgs"

    if [ $? -ne 0 ]; then cat config.log && exit 1; fi
    
    make -j $makeTasks
    if [ $? -ne 0 ]; then exit 1; fi
    
    OMP_NUM_THREADS=$ompThreads make check TASKS=$mpiTasks TEST_FLAGS="$matrixSize $nrEV $blockSize" || { cat test-suite.log; exit 1; }
    if [ $? -ne 0 ]; then exit 1; fi
     
    grep -i "Expected %stop" test-suite.log && exit 1 || true ;
    if [ $? -ne 0 ]; then exit 1; fi

    fi
  else
    #eval ./configure $configureArgs
    ./ci_test_scripts/configure_step.sh "$configureArgs"

    if [ $? -ne 0 ]; then cat config.log && exit 1; fi
    
    make -j $makeTasks
    if [ $? -ne 0 ]; then exit 1; fi
    
    OMP_NUM_THREADS=$ompThreads make check TASKS=$mpiTasks TEST_FLAGS="$matrixSize $nrEV $blockSize" || { cat test-suite.log; exit 1; }
    if [ $? -ne 0 ]; then exit 1; fi
     
    grep -i "Expected %stop" test-suite.log && exit 1 || true ;
    if [ $? -ne 0 ]; then exit 1; fi
  fi
fi

