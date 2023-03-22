node {
    stage('Preparation') {
        properties([
            parameters([
                string(
                    defaultValue: 'Hello World!',
                    description: 'Tekst do wyświetlenie',
                    name: 'INPUT_TEXT',
                    trim: false
                )
            ])
        ])
    }
    stage('Hello') {
        //Wypisz wartość parametru w konsoli (To nie jest polecenie bash, tylko groovy!)
        echo "INPUT_TEXT: ${INPUT_TEXT}"
        //Wywołaj w konsoli komendę "figlet", która generuje ASCI-art
        sh "figlet \"${INPUT_TEXT}\" | tee output.txt"
    }
    stage('Goodbye') {
        echo 'Goodbye!'
        //Zarchiwizuj wynik
        archiveArtifacts 'output.txt'
    }
}

