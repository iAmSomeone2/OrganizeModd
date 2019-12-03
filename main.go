package main

import (
	"fmt"
	"log"
	"os"

	"github.com/iAmSomeone2/organizemodd/metadata"
)

const testFilePath string = "/home/bdavidson/Videos/Home_Videos/1-12-2010/20091224204921.modd"

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

	moddText := string(moddBytes)

	moddData := metadata.GetModd(moddText, testFilePath)
	marshallTxt, _ := moddData.MarshallJSON()
	fmt.Printf("%s\n", marshallTxt)
}
