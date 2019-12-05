package metadata

import (
	"fmt"
	"log"
	"strconv"
	"strings"
)

// VT is an unknown type listed in the .modd file.
type VT struct {
	field0 uint64
	field1 uint64
	field2 float64
	field3 float64
	field4 float64
	field5 uint64
}

// GetVT parses a VT-formatted string and returns VT struct
func GetVT(vtText string) VT {
	var vt VT
	vtLines := strings.Split(vtText, ":")
	var err error
	vt.field0, err = strconv.ParseUint(vtLines[0], 10, 64)
	if err != nil {
		log.Fatal(err)
	}
	vt.field1, err = strconv.ParseUint(vtLines[1], 10, 64)
	if err != nil {
		log.Fatal(err)
	}
	vt.field2, err = strconv.ParseFloat(vtLines[2], 64)
	if err != nil {
		log.Fatal(err)
	}
	vt.field3, err = strconv.ParseFloat(vtLines[3], 64)
	if err != nil {
		log.Fatal(err)
	}
	vt.field4, err = strconv.ParseFloat(vtLines[4], 64)
	if err != nil {
		log.Fatal(err)
	}
	vt.field5, err = strconv.ParseUint(vtLines[5], 10, 64)
	if err != nil {
		log.Fatal(err)
	}

	return vt
}

func (vt VT) String() string {
	var vtStr strings.Builder

	vtStr.WriteString(
		fmt.Sprintf(
			"{%d, %d, %f, %f, %f, %d}",
			vt.field0, vt.field1, vt.field2, vt.field3, vt.field4, vt.field5))

	return vtStr.String()
}

// MarshalJSON provides the functionality to convert the VT struct to a JSON
// format.
func (vt VT) MarshalJSON() ([]byte, error) {
	var vtSb strings.Builder

	vtSb.WriteString(
		fmt.Sprintf(
			"{\"field0\":%d,\"field1\":%d,\"field2\":%f,\"field3\":%f,\"field4\":%f,\"field5\":%d}",
			vt.field0, vt.field1, vt.field2, vt.field3, vt.field4, vt.field5))

	return []byte(vtSb.String()), nil
}
