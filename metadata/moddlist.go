package metadata

import (
	"errors"
	"fmt"
	"strings"
)

// TODO: Modify so that the struct acts like a set.

// ModdList struct provides a struct for handling a list of Modds
type ModdList struct {
	data []Modd // Actual Modds
	len  uint64 // Number of items currently in list
	size uint64 // Current size of list
}

// Len returns the number of items in the list.
func (list ModdList) Len() uint64 {
	return list.len
}

// Size returns the current capacity of the list.
func (list ModdList) Size() uint64 {
	return list.size
}

// String provides the default printing interface for ModdList
func (list ModdList) String() string {
	var resultStr strings.Builder

	resultStr.WriteRune('[')
	for i := 0; uint64(i) < list.len; i++ {
		resultStr.WriteString(fmt.Sprintf("%s", list.data[i]))
		if uint64(i) < list.len-1 {
			resultStr.WriteRune(',')
		}
	}
	resultStr.WriteRune(']')

	return resultStr.String()
}

// MarshalJSON provides the functionality to convert the ModdList struct to a JSON
// format.
func (list ModdList) MarshalJSON() ([]byte, error) {
	var jsonStr strings.Builder
	jsonStr.WriteRune('[')
	for i := 0; uint64(i) < list.len; i++ {
		moddJSON, err := list.data[i].MarshalJSON()
		if err != nil {
			return []byte(""), err
		}
		jsonStr.Write(moddJSON)
		if uint64(i) < list.len-1 {
			jsonStr.WriteRune(',')
		}
	}
	jsonStr.WriteRune(']')

	return []byte(jsonStr.String()), nil
}

// MakeModdList creates and returns an initial Modd list of the specified size.
func MakeModdList(initialSize uint64) ModdList {
	var moddList ModdList

	moddList.data = make([]Modd, initialSize)
	moddList.len = 0
	moddList.size = initialSize

	return moddList
}

// expand doubles the size of the list.
func (list *ModdList) expand() {
	newSize := list.size * 2
	newList := make([]Modd, newSize)

	var i uint64
	for i = 0; i < list.len; i++ {
		newList[i] = list.data[i]
	}

	list.data = newList
	list.size = newSize
}

// Append adds the specified Modd to the end of the ModdList.
func (list *ModdList) Append(modd Modd) {
	// Resize list if it will be too small
	if list.len+1 == list.size {
		list.expand()
	}

	list.data[list.len] = modd
	list.len++
}

// Concat appends the data from a secondary list into the primary one.
func (list *ModdList) Concat(list1 ModdList) {
	// This is the point where I remembered that this isn't C
	list.data = append(list.data, list1.data...)
}

// Get retrieves the Modd from the specified index.
func (list ModdList) Get(i uint64) (Modd, error) {
	var returnModd Modd
	if i >= list.len || i < 0 {
		return returnModd, errors.New("out of bounds")
	}

	returnModd = list.data[i]

	return returnModd, nil
}

// Set reassigns an item in the list to a new modd object
func (list *ModdList) Set(i uint64, modd Modd) error {
	if i >= list.len || i < 0 {
		return errors.New("out of bounds")
	}

	list.data[i] = modd

	return nil
}
