package main

import (
	"log"
	"os"

	"github.com/iAmSomeone2/organizemodd/metadata"
)

const testFilePath string = "/home/bdavidson/Videos/Home_Videos/1-12-2010/20091224204921.modd"
const testDbPath string = "./db/library.sqlite"

func main() {
	moddFile, err := os.OpenFile(testFilePath, os.O_RDONLY, 0755)
	if err != nil {
		log.Fatal(err)
	}

	fileStats, _ := moddFile.Stat()

	moddBytes := make([]byte, fileStats.Size())

	numRead, err := moddFile.Read(moddBytes)
	if err != nil {
		log.Fatal(err)
	} else if numRead != len(moddBytes) {
		log.Print("Didn't fill byte array.")
	}

	moddFile.Close()

	// Open database connection
	database := metadata.ConnectToDb(testDbPath)
	defer database.Close()

	moddText := string(moddBytes)

	moddData := metadata.GetModd(moddText, testFilePath)
	err = metadata.AddModdToDb(moddData, database)
	if err != nil {
		log.Panic(err)
	}

}
