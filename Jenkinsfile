properties([parameters([
  choice(choices: 'Debug\nRelease', description: '', name: 'BUILD_TYPE'),
  booleanParam(defaultValue: true, description: '', name: 'Linux'),
  booleanParam(defaultValue: false, description: '', name: 'ARMv7'),
  booleanParam(defaultValue: false, description: '', name: 'ARMv8'),
  booleanParam(defaultValue: false, description: '', name: 'MacOS'),
  booleanParam(defaultValue: false, description: 'Scheduled triggered build', name: 'Nightly'),
  booleanParam(defaultValue: false, description: 'Merge this PR to target after success build', name: 'Merge_PR'),
  booleanParam(defaultValue: false, description: 'Force building code coverage', name: 'Coverage'),
  booleanParam(defaultValue: false, description: 'Build docs', name: 'Doxygen'),
  booleanParam(defaultValue: false, description: 'Build Java bindings', name: 'JavaBindings'),
  choice(choices: 'Release\nDebug', description: 'Java Bindings Build Type', name: 'JBBuildType'),
  booleanParam(defaultValue: false, description: 'Build Python bindings', name: 'PythonBindings'),
  choice(choices: 'Release\nDebug', description: 'Python Bindings Build Type', name: 'PBBuildType'),
  choice(choices: 'python3\npython2', description: 'Python Bindings Version', name: 'PBVersion'),
  booleanParam(defaultValue: false, description: 'Build Android bindings', name: 'AndroidBindings'),
  choice(choices: '26\n25\n24\n23\n22\n21\n20\n19\n18\n17\n16\n15\n14', description: 'Android Bindings ABI Version', name: 'ABABIVersion'),
  choice(choices: 'Release\nDebug', description: 'Android bindings build type', name: 'ABBuildType'),
  choice(choices: 'arm64-v8a\narmeabi-v7a\narmeabi\nx86_64\nx86', description: 'Android bindings platform', name: 'ABPlatform'),
  string(defaultValue: '4', description: 'How much parallelism should we exploit. "4" is optimal for machines with modest amount of memory and at least 4 cores', name: 'PARALLELISM')])])

pipeline {
  environment {
    CCACHE_DIR = '/opt/.ccache'
    CCACHE_RELEASE_DIR = '/opt/.ccache-release'
    SORABOT_TOKEN = credentials('SORABOT_TOKEN')
    SONAR_TOKEN = credentials('SONAR_TOKEN')
    GIT_RAW_BASE_URL = "https://raw.githubusercontent.com/hyperledger/iroha"
    DOCKER_REGISTRY_BASENAME = "hyperledger/iroha"
    JENKINS_DOCKER_IMAGE_DIR = '/tmp/docker'
    GIT_COMMITER_EMAIL = ''

    IROHA_NETWORK = "iroha-0${CHANGE_ID}-${GIT_COMMIT}-${BUILD_NUMBER}"
    IROHA_POSTGRES_HOST = "pg-0${CHANGE_ID}-${GIT_COMMIT}-${BUILD_NUMBER}"
    IROHA_POSTGRES_USER = "pguser${GIT_COMMIT}"
    IROHA_POSTGRES_PASSWORD = "${GIT_COMMIT}"
    IROHA_POSTGRES_PORT = 5432

    DOCKER_AGENT_IMAGE = ''
    DOCKER_IMAGE_FILE = ''
    WORKSPACE_PATH = ''
    PREVIOUS_COMMIT = ''
  }

  options {
    buildDiscarder(logRotator(numToKeepStr: '20'))
    timestamps()
  }

  agent any
  stages {
    stage ('Pre-build') {
      agent { label 'master' }
      steps {
        script {
          load ".jenkinsci/enums.groovy"
          GIT_COMMITER_EMAIL = sh(script: """git --no-pager show -s --format='%ae' ${env.GIT_COMMIT}""", returnStdout: true).trim()
          if (GIT_LOCAL_BRANCH != "develop") {
            def builds = load ".jenkinsci/cancel-builds-same-job.groovy"
            builds.cancelSameJobBuilds()
          }
        }
      }
    }
    stage('Build') {
      parallel {
        stage ('Linux') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.Linux }
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'x86_64_aws_build' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def coverage = load ".jenkinsci/selected-branches-coverage.groovy"
              if (params.BUILD_TYPE == 'Debug') {
                if (params.Merge_PR && env.CHANGE_TARGET ==~ /master/) {
                  debugBuild.doDebugBuild(true)
                }
                else {
                  debugBuild.doDebugBuild(coverage.selectedBranchesCoverage())
                }
              }
              if (params.BUILD_TYPE == 'Release') {
                def releaseBuild = load ".jenkinsci/release-build.groovy"
                releaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            success {
              script {
                if (params.BUILD_TYPE == 'Release') {
                  def post = load ".jenkinsci/linux-post-step.groovy"
                  post.postStep()
                }
              }
            }
          }
        }
        stage('ARMv7') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.ARMv7 }
              // allOf {
              //   expression { return env.CHANGE_ID != null }
              //   expression { return env.GIT_PREVIOUS_COMMIT != null }
              //   expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk)/ }
              //   expression { return params.Merge_PR }
              // }
            }
          }
          agent { label 'armv7' }
          steps {
            script {
              if ( params.BUILD_TYPE == 'Debug' || params.Merge_PR ) {
                def debugBuild = load ".jenkinsci/debug-build.groovy"
                debugBuild.doDebugBuild()
              }
              else {
                def releaseBuild = load ".jenkinsci/release-build.groovy"
                releaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            success {
              script {
                if (params.BUILD_TYPE == 'Release') {
                  def post = load ".jenkinsci/linux-post-step.groovy"
                  post.postStep()
                }
              }
            }
          }
        }
        stage('ARMv8') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.ARMv8 }
              // allOf {
              //   expression { return env.CHANGE_ID != null }
              //   expression { return env.GIT_PREVIOUS_COMMIT != null }
              //   expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk)/ }
              //   expression { return params.Merge_PR }
              // }
            }
          }
          agent { label 'armv8' }
          steps {
            script {
              if ( params.BUILD_TYPE == 'Debug' || params.Merge_PR ) {
                def debugBuild = load ".jenkinsci/debug-build.groovy"
                def coverage = load ".jenkinsci/selected-branches-coverage.groovy"
                debugBuild.doDebugBuild()
              }
              else {
                def releaseBuild = load ".jenkinsci/release-build.groovy"
                releaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            success {
              script {
                if (params.BUILD_TYPE == 'Release') {
                  def post = load ".jenkinsci/linux-post-step.groovy"
                  post.postStep()
                }
              }
            }
          }
        }
        stage('MacOS') {
          when {
            beforeAgent true
            anyOf {
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return !env.GIT_PREVIOUS_COMMIT }
              }
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk)/ }
                expression { return params.Merge_PR }
              }
              expression { return params.MacOS }
            }
          }
          agent { label 'mac' }
          steps {
            script {
              if ( params.BUILD_TYPE == 'Debug' || params.Merge_PR ) {
                def macDebugBuild = load ".jenkinsci/mac-debug-build.groovy"
                def coverage = load ".jenkinsci/selected-branches-coverage.groovy"
                macDebugBuild.doDebugBuild()
              }
              else {
                def macReleaseBuild = load ".jenkinsci/mac-release-build.groovy"
                macReleaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            success {
              script {
                if (params.BUILD_TYPE == 'Release') {
                  def post = load ".jenkinsci/linux-post-step.groovy"
                  post.macPostStep()
                }
              }
            }
          }
        }
      }
    }
    stage('Pre-Coverage') {
      when {
        anyOf {
          expression { params.Coverage }  // by request
          allOf {
            expression { return env.CHANGE_ID }
            expression { return !env.GIT_PREVIOUS_COMMIT }
          }
          allOf {
            expression { return env.CHANGE_ID != null }
            expression { return env.GIT_PREVIOUS_COMMIT != null }
            expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk)/ }
            expression { return params.Merge_PR }
          }
          allOf {
            expression { return params.BUILD_TYPE == 'Debug' }
            expression { return env.GIT_LOCAL_BRANCH ==~ /master/ }
          }
        }
      }
      agent { label 'x86_64_aws_cov'}
      steps {
        script {
          def coverage = load '.jenkinsci/debug-build.groovy'
          coverage.doPreCoverageStep()
        }
      }
    }
    stage('Tests') {
      when {
        beforeAgent true
        expression { return params.BUILD_TYPE == "Debug" }
      }
      parallel {
        stage('Linux') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.Linux }
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'x86_64_aws_test' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              debugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
          post {
            cleanup {
              script {
                def clean = load ".jenkinsci/docker-cleanup.groovy"
                clean.doDockerNetworkCleanup()
              }
            }
          }
        }
        stage('ARMv7') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.ARMv7 }
              // allOf {
              //   expression { return env.CHANGE_ID != null }
              //   expression { return env.GIT_PREVIOUS_COMMIT != null }
              //   expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk)/ }
              //   expression { return params.Merge_PR }
              // }
            }
          }
          agent { label 'armv7' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              debugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
          post {
            cleanup {
              script {
                def clean = load ".jenkinsci/docker-cleanup.groovy"
                clean.doDockerNetworkCleanup()
              }
            }
          }
        }
        stage('ARMv8') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.ARMv8 }
              // allOf {
              //   expression { return env.CHANGE_ID != null }
              //   expression { return env.GIT_PREVIOUS_COMMIT != null }
              //   expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk)/ }
              //   expression { return params.Merge_PR }
              // }
            }
          }
          agent { label 'armv8' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              debugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
          post {
            cleanup {
              script {
                def clean = load ".jenkinsci/docker-cleanup.groovy"
                clean.doDockerNetworkCleanup()
              }
            }
          }
        }
        stage('MacOS') {
          when {
            beforeAgent true
            anyOf {
              allOf {
                expression { return env.CHANGE_ID }
                expression { return !env.GIT_PREVIOUS_COMMIT }
              }
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk)/ }
                expression { return params.Merge_PR }
              }
              expression { return params.MacOS }
            }
          }
          agent { label 'mac' }
          steps {
            script {
              def macDebugBuild = load ".jenkinsci/mac-debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              macDebugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
          post {
            cleanup {
              script {
                sh "pg_ctl -D /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/ stop"
              }
            }
          }
        }
      }
    }
    stage('Post-Coverage') {
      parallel {
        stage('lcov_cobertura') {
          when {
            beforeAgent true
            anyOf {
              expression { params.Coverage }  // by request
              allOf {
                expression { return env.CHANGE_ID }
                expression { return !env.GIT_PREVIOUS_COMMIT }
              }
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return params.Merge_PR }
              }
              allOf {
                expression { return params.BUILD_TYPE == 'Debug' }
                expression { return env.GIT_LOCAL_BRANCH ==~ /master/ }
              }
            }
          }
          agent { label 'x86_64_aws_cov' }
          steps {
            script {
              def coverage = load '.jenkinsci/debug-build.groovy'
              coverage.doPostCoverageCoberturaStep()
            }
          }
        }
        stage('sonarqube') {
          when {
            beforeAgent true
            anyOf {
              expression { params.Coverage }  // by request
              allOf {
                expression { return env.CHANGE_ID }
                expression { return !env.GIT_PREVIOUS_COMMIT }
              }
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return params.Merge_PR }
              }
              allOf {
                expression { return params.BUILD_TYPE == 'Debug' }
                expression { return env.GIT_LOCAL_BRANCH ==~ /master/ }
              }
            }
          }
          agent { label 'x86_64_aws_cov' }
          steps {
            script {
              def coverage = load '.jenkinsci/debug-build.groovy'
              coverage.doPostCoverageSonarStep()
            }
          }
        }
      }
    }
    stage ('Build rest') {
      parallel {
        stage('Build release') {
          when {
            beforeAgent true
            anyOf {
              allOf {
                expression { return params.BUILD_TYPE == 'Debug' }
                expression { return env.GIT_LOCAL_BRANCH ==~ /(develop|master|trunk)/ }
              }
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return env.CHANGE_TARGET ==~ /(master|develop)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'x86_64_aws_build' }
          steps {
            script {
              def releaseBuild = load '.jenkinsci/release-build.groovy'
              releaseBuild.doReleaseBuild()
            }
          }
          post {
            success {
              script {
                if (params.BUILD_TYPE == 'Release') {
                  def post = load ".jenkinsci/linux-post-step.groovy"
                  post.postStep()
                }
              }
            }
          }
        }
        stage('Build docs') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.Doxygen }
              expression { return GIT_LOCAL_BRANCH ==~ /(master|develop)/ }
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return env.CHANGE_TARGET ==~ /(master|develop)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'x86_64_aws_cov' }
          steps {
            script {
              def doxygen = load ".jenkinsci/doxygen.groovy"
              docker.image("${env.DOCKER_IMAGE}").inside {
                def scmVars = checkout scm
                doxygen.doDoxygen()
              }
            }
          }
        }
        stage('Build bindings') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.PythonBindings }
              expression { return params.JavaBindings }
              expression { return params.AndroidBindings }
              allOf {
                expression { return env.CHANGE_ID != null }
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return env.CHANGE_TARGET ==~ /(master|develop)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'x86_64_aws_build' }
          environment {
            JAVA_HOME = '/usr/lib/jvm/java-8-oracle'
          }
          steps {
            script {
              def bindings = load ".jenkinsci/bindings.groovy"
              def dPullOrBuild = load ".jenkinsci/docker-pull-or-build.groovy"
              def platform = sh(script: 'uname -m', returnStdout: true).trim()
              if (params.JavaBindings) {
                iC = dPullOrBuild.dockerPullOrUpdate("$platform-develop",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/develop/${platform}/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_PREVIOUS_COMMIT}/docker/develop/${platform}/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/develop/docker/develop/x86_64/Dockerfile",
                                                     ['PARALLELISM': params.PARALLELISM])
                iC.inside("-v /tmp/${env.GIT_COMMIT}/bindings-artifact:/tmp/bindings-artifact") {
                  bindings.doJavaBindings(params.JBBuildType)
                }
              }
              if (params.PythonBindings) {
                iC = dPullOrBuild.dockerPullOrUpdate("$platform-develop",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/develop/${platform}/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_PREVIOUS_COMMIT}/docker/develop/${platform}/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/develop/docker/develop/x86_64/Dockerfile",
                                                     ['PARALLELISM': params.PARALLELISM])
                iC.inside("-v /tmp/${env.GIT_COMMIT}/bindings-artifact:/tmp/bindings-artifact") {
                  bindings.doPythonBindings(params.PBBuildType)
                }
              }
              if (params.AndroidBindings) {
                iC = dPullOrBuild.dockerPullOrUpdate("android-${params.ABPlatform}-${params.ABBuildType}",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/android/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_PREVIOUS_COMMIT}/docker/android/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/develop/docker/android/Dockerfile",
                                                     ['PARALLELISM': params.PARALLELISM, 'PLATFORM': params.ABPlatform, 'BUILD_TYPE': params.ABBuildType])
                sh "curl -L -o /tmp/${env.GIT_COMMIT}/entrypoint.sh ${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/android/entrypoint.sh"
                sh "chmod +x /tmp/${env.GIT_COMMIT}/entrypoint.sh"
                iC.inside("-v /tmp/${env.GIT_COMMIT}/entrypoint.sh:/entrypoint.sh:ro -v /tmp/${env.GIT_COMMIT}/bindings-artifact:/tmp/bindings-artifact") {
                  bindings.doAndroidBindings(params.ABABIVersion)
                }
              }
            }
          }
          post {
            always {
              timeout(time: 600, unit: "SECONDS") {
                script {
                  try {
                    if (currentBuild.currentResult == "SUCCESS") {
                      def artifacts = load ".jenkinsci/artifacts.groovy"
                      def commit = env.GIT_COMMIT
                      if (params.JavaBindings) {
                        javaBindingsFilePaths = [ '/tmp/${GIT_COMMIT}/bindings-artifact/java-bindings-*.zip' ]
                        artifacts.uploadArtifacts(javaBindingsFilePaths, '/iroha/bindings/java')
                      }
                      if (params.PythonBindings) {
                        pythonBindingsFilePaths = [ '/tmp/${GIT_COMMIT}/bindings-artifact/python-bindings-*.zip' ]
                        artifacts.uploadArtifacts(pythonBindingsFilePaths, '/iroha/bindings/python')
                      }
                      if (params.AndroidBindings) {
                        androidBindingsFilePaths = [ '/tmp/${GIT_COMMIT}/bindings-artifact/android-bindings-*.zip' ]
                        artifacts.uploadArtifacts(androidBindingsFilePaths, '/iroha/bindings/android')
                      }
                    }
                  }
                  finally {
                    sh "rm -rf /tmp/${env.GIT_COMMIT}"
                    // cleanWs()
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  post {
    success {
      script {
        // merge pull request if everything is ok and clean stale docker images stored on EFS
        if ( params.Merge_PR ) {
          def merge = load ".jenkinsci/github-api.groovy"
          if (merge.mergePullRequest()) {
            currentBuild.result = "SUCCESS"
            def clean = load ".jenkinsci/docker-cleanup.groovy"
            clean.doStaleDockerImagesCleanup()
          }
          else {
            currentBuild.result = "FAILURE"
          }
        }
      }
    }
    cleanup {
      script {
        def post = load ".jenkinsci/linux-post-step.groovy"
        def clean = load ".jenkinsci/docker-cleanup.groovy"
        def notify = load ".jenkinsci/notifications.groovy"
        notify.notifyBuildResults()

        if (params.Linux || params.Merge_PR) {
          node ('x86_64_aws_test') {
            post.cleanUp()
          }
        }
        if (params.ARMv8) {
          node ('armv8') {
            post.cleanUp()
            clean.doDockerCleanup()
          }
        }
        if (params.ARMv7) {
          node ('armv7') {
            post.cleanUp()
            clean.doDockerCleanup()
          }
        }
        if (params.MacOS || params.Merge_PR) {
          node ('mac') {
            post.macCleanUp()
          }
        }
      }
    }
  }
}
