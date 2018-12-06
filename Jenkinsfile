pipeline
{
    agent
    {
        docker
        {
            image 'ncbi-vdb3.ubuntu.ci'
            args '-v /var/run/docker.sock:/var/run/docker.sock'
        }
    }
    stages {
        stage('build') {
            steps {
                sh 'rm -f Makefile.config;make DEFAULT_OUTDIR=outdir DEFAULT_BUILD=rel -i test '
            }
        }
    }
}