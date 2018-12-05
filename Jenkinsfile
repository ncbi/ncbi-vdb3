pipeline {
    agent { docker { image 'ncbi-vdb3:jenkins' } }
    stages {
        stage('build') {
            steps {
                sh 'rm -f Makefile.config;make DEFAULT_OUTDIR=outdir DEFAULT_BUILD=rel test'
            }
        }
    }
}