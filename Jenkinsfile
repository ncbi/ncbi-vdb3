pipeline {
    agent { docker { image 'gcc:5' } }
    stages {
        stage('build') {
            steps {
                sh 'make'
            }
        }
    }
}
