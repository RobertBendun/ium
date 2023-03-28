node {
    stage('Build') {
			checkout([$class: 'GitSCM', branches: [[name: 'ztm']], extensions: [], userRemoteConfigs: [[url: 'https://git.wmi.amu.edu.pl/s452639/ium_452639']]])
			sh './prepare-ztm-data.sh'
			archiveArtifacts artifacts: 'train.csv,test.csv', followSymlinks: false
    }
}

