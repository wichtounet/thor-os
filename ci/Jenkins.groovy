node {
   stage 'git'
   checkout([$class: 'GitSCM', branches: [[name: '*/develop']], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'SubmoduleOption', disableSubmodules: false, recursiveSubmodules: true, reference: '', trackingSubmodules: false]], submoduleCfg: [], userRemoteConfigs: [[url: 'https://github.com/wichtounet/thor-os.git']]])

   stage 'pre-analysis'
   sh 'cppcheck --xml-version=2 --enable=all --std=c++11 kernel/src kernel/include tstl/include tstl/test_suite tlib/include tlib/src  printf/include tlib/include tlib/src 2> cppcheck_report.xml'
   sh 'sloccount --duplicates --wide --details kernel/src kernel/include tstl/include tstl/test_suite tlib/include tlib/src printf/include tlib/include tlib/src > sloccount.sc'

   stage 'sonar'
   sh '/opt/sonar-runner/bin/sonar-runner'
}
