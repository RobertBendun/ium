node {
	checkout scm

	def local_image = docker.build("local-image")

	local_image.inside {
		stage('Build') {
			checkout([$class: 'GitSCM', branches: [[name: 'ztm']], extensions: [], userRemoteConfigs: [[url: 'https://git.wmi.amu.edu.pl/s452639/ium_452639']]])
			sh 'cd src; ./prepare-ztm-data.sh'
			archiveArtifacts artifacts: 'stop_times.train.tsv,stop_times.test.tsv,stop_times.valid.tsv', followSymlinks: false
		}
	}
}

