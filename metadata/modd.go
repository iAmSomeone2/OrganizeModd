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

// TimeZone provides the values needed to compute metadata date/time offsets.
type TimeZone uint64

const (
	// EST (Eastern Standard Time)
	EST TimeZone = (5 + iota) * 3600
	// CST (Central Standard Time)
	CST
	// MST (Mountain Standard Time)
	MST
	// PST (Pacific Standard Time)
	PST
)

// Modd is a structured representation of the data contained in a .modd file.
type Modd struct {
	Name             string    // Derived from file name
	Location         string    // Location of .modd file in filesystem
	CheckCode        uint64    // Unknown algorithm
	DateTimeOriginal float64   // Measured in days since Dec. 30, 1899
	DateTimeActual   time.Time // Unix-standard version of dateTimeOriginal
	Duration         float64   // Seconds
	FileSize         uint64    // Bytes
	VtList           []VT      // Unkown purpose. Potentially video timings
}

// FORMATTING FUNCTIONS

// String provides the default printing interface for Modd
func (m Modd) String() string {
	var mStr strings.Builder

	mStr.WriteString(fmt.Sprintf("{ Name = %s", m.Name))
	mStr.WriteString(fmt.Sprintf(", CheckCode = %d", m.CheckCode))
	mStr.WriteString(fmt.Sprintf(", DateTimeOriginal = %0.15f", m.DateTimeOriginal))
	mStr.WriteString(", DateTimeActual = ")
	mStr.WriteString(fmt.Sprintf("%s", m.DateTimeActual))
	mStr.WriteString(fmt.Sprintf(", Duration = %0.15f", m.Duration))
	mStr.WriteString(fmt.Sprintf(", FileSize = %s", strconv.FormatUint(m.FileSize, 10)))
	mStr.WriteString(fmt.Sprintf(", VTList = %v}", m.VtList))

	return mStr.String()
}

// MarshalJSON provides the functionality to convert the Modd struct to a JSON
// format.
func (m Modd) MarshalJSON() ([]byte, error) {
	var jsonSb strings.Builder

	jsonSb.WriteString(fmt.Sprintf("{\"name\":\"%s\",", m.Name))
	jsonSb.WriteString(fmt.Sprintf("\"checkCode\":%d,", m.CheckCode))
	jsonSb.WriteString(fmt.Sprintf("\"dateTimeOriginal\":%0.15f,", m.DateTimeOriginal))
	dateTimeTxt, err := m.DateTimeActual.MarshalJSON()
	jsonSb.WriteString(fmt.Sprintf("\"dateTimeActual\":%s,", dateTimeTxt))
	jsonSb.WriteString(fmt.Sprintf("\"duration\":%0.15f,", m.Duration))
	jsonSb.WriteString(fmt.Sprintf("\"fileSize\":%s,", strconv.FormatUint(m.FileSize, 10)))
	// Deal with the VTList
	jsonSb.WriteString("\"vtList\":[")
	for i, item := range m.VtList {
		vtTxt, _ := item.MarshalJSON()
		jsonSb.WriteString(fmt.Sprintf("%s", vtTxt))
		if i < len(m.VtList)-1 {
			jsonSb.WriteRune(',')
		}
	}
	jsonSb.WriteRune(']')

	jsonSb.WriteString("}")
	return []byte(jsonSb.String()), err
}

// Equals computes the equality of two Modd structs
func (m Modd) Equals(m1 Modd) bool {
	if m.Name != m1.Name {
		return false
	}
	if m.CheckCode != m1.CheckCode {
		return false
	}
	if m.DateTimeOriginal != m1.DateTimeOriginal {
		return false
	}
	if m.DateTimeActual != m1.DateTimeActual {
		return false
	}
	if m.Duration != m1.Duration {
		return false
	}
	if m.FileSize != m1.FileSize {
		return false
	}

	// Check the VT list here
	if len(m.VtList) != len(m1.VtList) {
		return false
	}

	for i := 0; i < len(m.VtList); i++ {
		if !m.VtList[i].Equals(m1.VtList[i]) {
			return false
		}
	}

	return true
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

	return moddText
}

func (m *Modd) setActualTime(tz TimeZone) {
	// Convert original time to seconds instead of days
	originalSecs := uint64(m.DateTimeOriginal * 86400)
	// Convert to seconds from Unix epoch
	epochSecs := (originalSecs - uinxMinusMSEpoch) + uint64(tz)

	m.DateTimeActual = time.Unix(int64(epochSecs), 0).UTC()
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
	modd.Location = moddFilePath
	modd.Name = moddFileName
	moddLines := strings.Split(moddText, "\n")
	for _, line := range moddLines {
		var err error
		if !readArray {
			values := strings.Split(line, ",")
			switch strings.ToLower(values[0]) {
			case "checkcode":
				checkCode, err := strconv.ParseUint(values[1], 16, 32)
				if err != nil {
					log.Fatal(err)
				}
				modd.CheckCode = checkCode
				break
			case "datetimeoriginal":
				modd.DateTimeOriginal, err = strconv.ParseFloat(values[1], 64)
				if err != nil {
					log.Fatal(err)
				}
				modd.setActualTime(CST)
				break
			case "duration":
				modd.Duration, err = strconv.ParseFloat(values[1], 64)
				if err != nil {
					log.Fatal(err)
				}
				break
			case "filesize":
				modd.FileSize, err = strconv.ParseUint(values[1], 10, 64)
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
			modd.VtList = append(modd.VtList, vtVal)
		}
	}

	return modd
}
