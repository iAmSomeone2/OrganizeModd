package metadata

import (
	"database/sql"
	"fmt"
	"log"
	"strings"

	// Connector for sql db
	_ "github.com/mattn/go-sqlite3"
)

const dbType string = "sqlite3"

const prepareModdTable string = "CREATE TABLE IF NOT EXISTS modd (checkCode INTEGER UNIQUE, name TEXT, dateTime INTEGER, videoDuration REAL, videoFileSize INTEGER, moddFileLocation TEXT UNIQUE, PRIMARY KEY(checkCode))"
const prepareVideoTable string = "CREATE TABLE IF NOT EXISTS video (id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, name TEXT, moddCheckCode TEXT UNIQUE, dateTime INTEGER, duration REAL, fileLocation TEXT, fileSize INTEGER, FOREIGN KEY(moddCheckCode) REFERENCES modd(checkCode))"

// ConnectToDb creates a database connection to the local db and returns a pointer to
// a sql.DB struct.
func ConnectToDb(path string) *sql.DB {
	database, err := sql.Open(dbType, path)
	if err != nil {
		log.Fatal(err)
	}

	// Set up any missing tables if they don't already exist
	statement, err := database.Prepare(prepareModdTable)
	if err != nil {
		log.Fatal(err)
	}
	defer statement.Close()
	statement.Exec()

	statement, err = database.Prepare(prepareVideoTable)
	if err != nil {
		log.Fatal(err)
	}
	statement.Exec()

	return database
}

// AddModdToDb adds a .modd file to the database if it does not already exist.
//
// The function will update a prexisting entry.
func (modd Modd) AddModdToDb(db *sql.DB) error {
	// Check if the modd's checkCode is already in use in the db.
	rows, err := db.Query(fmt.Sprintf(`SELECT * FROM modd WHERE checkCode == %d`, modd.CheckCode))
	if err != nil {
		return err
	}
	defer rows.Close()

	if rows.Next() {
		rows.Close()
		// fmt.Println("Updating exisiting row in modd table...")
		// At least one row is present if this is true. Update with the new modd values.
		err := modd.UpdateModdInDb(db)

		return err
	}
	// Insert new modd entry
	// fmt.Println("Adding new modd entry...")
	statement, err := db.Prepare(
		`INSERT INTO "modd" (checkCode, name, dateTime, videoDuration, videoFileSize, moddFileLocation) VALUES (?, ?, ?, ?, ?, ?)`)
	_, err = statement.Exec(modd.CheckCode, modd.Name, modd.DateTimeActual.Unix(), modd.Duration, modd.FileSize, modd.Location)
	defer statement.Close()
	if err != nil {
		return err
	}

	return nil
}

// UpdateModdInDb updates a modd entry if it is already present.
func (modd Modd) UpdateModdInDb(db *sql.DB) error {
	var statementStr strings.Builder
	rows, err := db.Query(fmt.Sprintf(`SELECT * FROM modd WHERE checkCode LIKE %d`, modd.CheckCode))
	if err != nil {
		return err
	}
	var tempModd Modd
	tempModd.DateTimeOriginal = -1
	rows.Scan(
		tempModd.CheckCode, tempModd.Name,
		tempModd.DateTimeActual, tempModd.Duration, tempModd.FileSize, tempModd.Location)
	rows.Close()

	// Construct SQL statement
	statementStr.WriteString(fmt.Sprintf(`UPDATE modd SET name = "%s"`, modd.Name))
	statementStr.WriteString(fmt.Sprintf(`, dateTime = %d`, modd.DateTimeActual.Unix()))
	statementStr.WriteString(fmt.Sprintf(`, videoDuration = %0.15f`, modd.Duration))
	statementStr.WriteString(fmt.Sprintf(`, videoFileSize = %d`, modd.FileSize))
	if tempModd.Location != modd.Location {
		statementStr.WriteString(fmt.Sprintf(`, moddFileLocation = "%s"`, modd.Location))
	}
	statementStr.WriteString(fmt.Sprintf(` WHERE checkCode LIKE %d`, modd.CheckCode))

	statement, err := db.Prepare(statementStr.String())
	if err != nil {
		return err
	}
	defer statement.Close()
	_, err = statement.Exec()
	if err != nil {
		return err
	}

	return nil
}
