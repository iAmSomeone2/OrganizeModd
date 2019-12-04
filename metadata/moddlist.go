package metadata

import "errors"

// ModdList struct provides a struct for handling a list of Modds
type ModdList struct {
	data []Modd // Actual Modds
	len  uint64 // Number of items currently in list
	size uint64 // Current size of list
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

	for i, modd := range list.data {
		newList[i] = modd
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

// Get retrieves the Modd from the specified index.
func (list ModdList) Get(i uint64) (Modd, error) {
	var returnModd Modd
	if i >= list.len || i < 0 {
		return returnModd, errors.New("out of bounds")
	}

	returnModd = list.data[i]

	return returnModd, nil
}
