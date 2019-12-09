package main

import (
	"database/sql"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"

	"github.com/iAmSomeone2/organizemodd/metadata"
)

const testDir string = "/home/bdavidson/Videos/Home_Videos/"
const testDbPath string = "./db/library.sqlite"

func importModds(root string) metadata.ModdSet {
	moddList := metadata.MakeModdSet(10)

	err := filepath.Walk(root, func(path string, info os.FileInfo, err error) error {
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
		} else if info.IsDir() && path != root {
			// Follow directory and append the results to the current ModdSet
			moddList.Concat(importModds(path))
		}
		return nil
	})
	if err != nil {
		log.Panic(err)
	}

	return moddList
}

func updateModdTable(list metadata.ModdSet, db *sql.DB) {
	var i uint64
	for i = 0; i < list.Len(); i++ {
		modd, err := list.Get(i)
		if err != nil {
			log.Panic(err)
		}
		err = modd.AddModdToDb(db)
		if err != nil {
			log.Panic(err)
		}
	}
}

func main() {
	fmt.Printf("Looking for \"modd\" files...\n")
	moddSet := importModds(testDir)

	// Open database connection
	database := metadata.ConnectToDb(testDbPath)
	defer database.Close()

	fmt.Printf("Updating modd database...\n")
	updateModdTable(moddSet, database)

	// Attempt to find videos associated with the modd files.
	fmt.Printf("Looking for matching video files...\n")
	vidMap := metadata.VideoMapFromModdSet(moddSet)
	fmt.Printf("%s\n", vidMap.String())
}
