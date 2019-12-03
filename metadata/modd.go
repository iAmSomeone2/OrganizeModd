package metadata

import (
	"fmt"
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

// Time constants
const uinxMinusMSEpoch uint64 = 2209161600
const minSecs uint64 = 60
const hourSecs uint64 = 3600
const daySecs uint64 = 86400
const monthSecs uint64 = 2629743
const yearSecs uint64 = 31556926

// Modd is a structured representation of the data contained in a .modd file.
type Modd struct {
	name             string    // Derived from file name
	checkCode        string    // Unknown algorithm
	dateTimeOriginal float64   // Measured in days since Dec. 30, 1899
	dateTimeActual   time.Time // Unix-standard version of dateTimeOriginal
	duration         float64   // Seconds
	fileSize         uint64    // Bytes
	vtList           []VT      // Unkown purpose. Potentially video timings
}

// FORMATTING FUNCTIONS

// String provides the default printing interface for Modd
func (m Modd) String() string {
	var mStr strings.Builder

	mStr.WriteString(fmt.Sprintf("{ Name = %s", m.name))
	mStr.WriteString(fmt.Sprintf(", CheckCode = %s", m.checkCode))
	mStr.WriteString(fmt.Sprintf(", DateTimeOriginal = %0.15f", m.dateTimeOriginal))
	mStr.WriteString(", DateTimeActual = ")
	mStr.WriteString(fmt.Sprintf("%d-%d-%d", m.dateTimeActual.Year(), m.dateTimeActual.Month(), m.dateTimeActual.Day()))
	mStr.WriteString(fmt.Sprintf(", Duration = %0.15f", m.duration))
	mStr.WriteString(fmt.Sprintf(", FileSize = %d", m.fileSize))
	mStr.WriteString(fmt.Sprintf(", VTList = %v}", m.vtList))

	return mStr.String()
}

// MarshallJSON provides the functionality to convert the Modd struct to a JSON
// format.
func (m Modd) MarshallJSON() ([]byte, error) {
	var jsonSb strings.Builder

	jsonSb.WriteString(fmt.Sprintf("{\"Name\":\"%s\",", m.name))
	jsonSb.WriteString(fmt.Sprintf("\"CheckCode\":\"%s\",", m.checkCode))
	jsonSb.WriteString(fmt.Sprintf("\"DateTimeOriginal\":%0.15f,", m.dateTimeOriginal))
	dateTimeTxt, err := m.dateTimeActual.MarshalJSON()
	jsonSb.WriteString(fmt.Sprintf("\"DateTimeActual\":%s,", dateTimeTxt))
	jsonSb.WriteString(fmt.Sprintf("\"Duration\":%0.15f,", m.duration))
	jsonSb.WriteString(fmt.Sprintf("\"FileSize\":%d,", m.fileSize))
	// Deal with the VTList
	jsonSb.WriteString("\"VTList\":[")
	for i, item := range m.vtList {
		vtTxt, _ := item.MarshallJSON()
		jsonSb.WriteString(fmt.Sprintf("%s", vtTxt))
		if i < len(m.vtList)-1 {
			jsonSb.WriteRune(',')
		}
	}
	jsonSb.WriteRune(']')

	jsonSb.WriteString("}")
	return []byte(jsonSb.String()), err
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
	moddText = strings.ReplaceAll(moddText, "<array>", "[\n")
	moddText = strings.ReplaceAll(moddText, "</array>", "]")

	moddText = strings.TrimSpace(moddText)

	fmt.Println(moddText)
	return moddText
}

func (m *Modd) setActualTime() {
	// Convert original time to seconds instead of days
	originalSecs := uint64(m.dateTimeOriginal * 86400)
	// Convert to seconds from Unix epoch
	epochSecs := originalSecs - uinxMinusMSEpoch

	m.dateTimeActual = time.Unix(int64(epochSecs), 0)
}

// GetModd reads in the raw XML data from the .modd file and returns a Modd struct.
func GetModd(moddText string, moddFilePath string) Modd {
	readArray := false

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
		if !readArray {
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
				modd.setActualTime()
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
			case "vtlist":
				// VTList is an array, so it will need to be split and converted
				if values[1] == "[" {
					readArray = true
				}
			default:
				log.Printf("Parsing '%s' not currently implemented.\n", values[0])
			}
		} else {
			// Read in the VTList array until the closing square bracket is reached
			if strings.Contains(line, "]") {
				readArray = false
				continue
			}
			vtVal := GetVT(line)
			modd.vtList = append(modd.vtList, vtVal)
		}
	}

	return modd
}
