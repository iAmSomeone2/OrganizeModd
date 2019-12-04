package main

import (
	"log"
	"os"
	"path/filepath"
	"strings"

	"github.com/iAmSomeone2/organizemodd/metadata"
)

const testFilePath string = "/home/bdavidson/Videos/Home_Videos/1-12-2010/20091224204921.modd"
const testDir string = "/home/bdavidson/Videos/Home_Videos/1-12-2010/"
const testDbPath string = "./db/library.sqlite"

func main() {
	moddList := metadata.MakeModdList(10)

	err := filepath.Walk(testDir, func(path string, info os.FileInfo, err error) error {
		if !info.IsDir() && strings.ToLower(filepath.Ext(path)) == ".modd" {
			moddFile, err := os.OpenFile(path, os.O_RDONLY, 0755)
			if err != nil {
				return err
			}

			moddBytes := make([]byte, info.Size())
			numRead, err := moddFile.Read(moddBytes)
			if err != nil {
				log.Fatal(err)
			} else if numRead != len(moddBytes) {
				log.Print("Didn't fill byte array.")
			}
			moddFile.Close()

			moddText := string(moddBytes)

			moddList.Append(metadata.GetModd(moddText, path))
		}
		return nil
	})
	if err != nil {
		log.Panic(err)
	}

	// Open database connection
	database := metadata.ConnectToDb(testDbPath)
	defer database.Close()

	// err = moddData.AddModdToDb(database)
	// if err != nil {
	// 	log.Panic(err)
	// }

	// video := moddData.VideoFromModd()
	// videoJSON, _ := video.MarshallJSON()
	// fmt.Printf("%s\n", videoJSON)
}
