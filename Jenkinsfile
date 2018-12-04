pipeline {
    agent { docker { image 'vdb3' } }
    stages {
        stage('build') {
            steps {
                shell 'make'
            }
        }
    }
}
