package metadata

import (
	"log"
	"strings"
)

// VideoMap is a struct designed for handling a large set of Video structs in
// which they are organized by their hashes. This is really just a wrapper for
// a normal map with additional formatting and management methods added.
type VideoMap struct {
	data map[string]*Video
}

func (vm VideoMap) String() string {
	var result strings.Builder

	result.WriteString("[ ")
	for _, value := range vm.data {
		result.WriteString(value.String())
		result.WriteRune(' ')
	}
	result.WriteRune(']')

	return result.String()
}

// MakeVideoMap initializes and returns a new VideoMap.
func MakeVideoMap() VideoMap {
	var vm VideoMap
	vm.data = make(map[string]*Video)

	return vm
}

// VideoMapFromModdSet reads in a modd set, attempts to make Video structs from each entry,
// and returns a VideoMap.
func VideoMapFromModdSet(ms ModdSet) VideoMap {
	vm := MakeVideoMap()

	var i uint64
	for i = 0; i < ms.Len(); i++ {
		modd, err := ms.Get(i)
		if err != nil {
			log.Fatal(err)
		}
		vid := modd.VideoFromModd()
		vm.Append(&vid)
	}

	return vm
}

// Append assigns a new Video pointer to the VideoMap
func (vm *VideoMap) Append(vid *Video) {
	vm.data[string(vid.SHA256Hash)] = vid
}

// Get returns the Video associated with the key
func (vm VideoMap) Get(key []byte) *Video {
	return vm.data[string(key)]
}
