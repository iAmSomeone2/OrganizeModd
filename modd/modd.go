package modd

import (
	"log"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

/*
	NOTE: DateTimeOriginal in a .modd file is the elapsed number of days
	since Dec 30, 1899.
*/

const xmlHeader string = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
const dataHeader string = "<plist version=\"1.0\"><dict><key>MetaDataList</key><array><dict>"
const dataFooter string = "</dict></array><key>XMLFileType</key><string>ModdXML</string></dict></plist>"

// Modd is a structured representation of the data contained in a .modd file.
type Modd struct {
	name             string
	checkCode        string
	dateTimeOriginal float64
	dateTimeActual   time.Time
	duration         float64
	fileSize         uint64
	vtList           []string
}

func (m Modd) String() string {
	var mStr strings.Builder

	mStr.WriteString("{ Name = ")
	mStr.WriteString(m.name)
	mStr.WriteString(", CheckCode = ")
	mStr.WriteString(m.checkCode)
	mStr.WriteString(", DateTimeOriginal = ")
	mStr.WriteString(strconv.FormatFloat(m.dateTimeOriginal, 'f', 15, 64))
	mStr.WriteString(", Duration = ")
	mStr.WriteString(strconv.FormatFloat(m.duration, 'f', 15, 64))
	mStr.WriteString(", FileSize = ")
	mStr.WriteString(strconv.FormatUint(m.fileSize, 10))
	mStr.WriteString("}")

	return mStr.String()
}

// cleanText reads the text from the .modd file and converts it into a more readable
// string.
func cleanText(moddText string) string {
	// Start by removing the unneeded lines of the file
	moddText = strings.Replace(moddText, xmlHeader, "", 2)
	moddText = strings.Replace(moddText, dataHeader, "", 2)
	moddText = strings.Replace(moddText, dataFooter, "", 2)

	// Clean up tags
	moddText = strings.ReplaceAll(moddText, "<key>", "")
	moddText = strings.ReplaceAll(moddText, "</key>", ",")
	moddText = strings.ReplaceAll(moddText, "<string>", "")
	moddText = strings.ReplaceAll(moddText, "</string>", "\n")
	moddText = strings.ReplaceAll(moddText, "<real>", "")
	moddText = strings.ReplaceAll(moddText, "</real>", "\n")
	moddText = strings.ReplaceAll(moddText, "<integer>", "")
	moddText = strings.ReplaceAll(moddText, "</integer>", "\n")
	moddText = strings.ReplaceAll(moddText, "<array>", "\n")
	moddText = strings.ReplaceAll(moddText, "</array>", "")

	moddText = strings.Trim(moddText, "\n")

	return moddText
}

func (m Modd) setActualTime() {
	location, err := time.LoadLocation("Local")
	if err != nil {
		log.Print(err)
	}

	year := uint64(m.dateTimeOriginal)/365 + 1899
	month := (uint64(m.dateTimeOriginal) / 365)
}

// GetModd reads in the raw XML data from the .modd file and returns a Modd struct.
func GetModd(moddText string, moddFilePath string) Modd {
	// Get the name of the modd
	moddFileName := filepath.Base(moddFilePath)
	ext := filepath.Ext(moddFileName)
	moddFileName = strings.Replace(moddFileName, ext, "", 2)

	moddText = cleanText(moddText)
	var modd Modd
	modd.name = moddFileName
	moddLines := strings.Split(moddText, "\n")
	for _, line := range moddLines {
		var err error
		values := strings.Split(line, ",")
		switch strings.ToLower(values[0]) {
		case "checkcode":
			modd.checkCode = values[1]
			break
		case "datetimeoriginal":
			modd.dateTimeOriginal, err = strconv.ParseFloat(values[1], 64)
			if err != nil {
				log.Fatal(err)
			}
			break
		case "duration":
			modd.duration, err = strconv.ParseFloat(values[1], 64)
			if err != nil {
				log.Fatal(err)
			}
			break
		case "filesize":
			modd.fileSize, err = strconv.ParseUint(values[1], 10, 64)
			if err != nil {
				log.Fatal(err)
			}
			break
		default:
			log.Printf("Parsing '%s' not currently implemented.\n", values[0])
		}
	}

	return modd
}
