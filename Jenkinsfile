pipeline {
    agent { docker { image 'vdb3' } }
    stages {
        stage('build') {
            steps {
                shell 'rm -f Makefile.config;make DEFAULT_OUTDIR=outdir DEFAULT_BUILD=rel test'
            }
        }
    }
}
