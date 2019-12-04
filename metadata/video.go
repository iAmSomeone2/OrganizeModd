package metadata

import (
	"os"
	"path/filepath"
	"strings"
	"time"
)

var videoExtensions = [...]string{".mpg", ".MPG", ".mpeg", ".MPEG", ".mp4", ".MP4",
	".m4v", ".M4V", ".mkv", ".MKV"}

// Container represents the supported container types.
type Container int

// Enum for Video Containers
const (
	MPEG Container = iota + 1
	MP4
	MKV
)

// String provides the default printing interface for Container
func (cont Container) String() string {
	var returnStr strings.Builder
	switch cont {
	case MPEG:
		returnStr.WriteString("mpeg")
		break
	case MP4:
		returnStr.WriteString("mp4")
		break
	case MKV:
		returnStr.WriteString("Matroska")
		break
	}

	return returnStr.String()
}

// VideoCodec represents the supported video codecs
type VideoCodec int

// Enumerator for Codec consts
const (
	MPEG2 VideoCodec = iota + 1
	X264
	X265
)

// String provides the default printing interface for VideoCodec
func (vc VideoCodec) String() string {
	var returnStr strings.Builder

	switch vc {
	case MPEG2:
		returnStr.WriteString("mpeg2")
		break
	case X264:
		returnStr.WriteString("x264")
		break
	case X265:
		returnStr.WriteString("x265")
		break
	}

	return returnStr.String()
}

// AudioCodec represents the supported audio codecs
type AudioCodec int

const (
	// AC3 audio codec
	AC3 AudioCodec = iota + 1
	// AAC audio codec
	AAC
	// VORBIS audio codec
	VORBIS
)

// Video is a struct representing all of the relevant metadata for a video file.
type Video struct {
	Name         string     // Name of file not including extension
	Location     string     // Location of file on system
	CreationTime time.Time  // Time when video was first created
	Duration     float64    // Number of seconds in video file
	FileSize     uint64     // Number of bytes in file
	VidContainer Container  // Container type of the video file.
	VidCodec     VideoCodec // Video encoding type
	AudCodec     AudioCodec // Audio encoding type
	SHA256Hash   []byte     // SHA-256 hash of the file contents
	LinkedModd   *Modd      // Modd struct associated with this video file
}

// determineLocation finds the location of a video file based on where its .modd file is
// located.
func determineLocation(location string) string {
	var videoPath string
	fileName := filepath.Base(location)
	ext := filepath.Ext(fileName)
	fileName = strings.Replace(fileName, ext, "", 2)
	directory := filepath.Dir(location)

	// Replace extension in path until the file is found
	// The written file extension does not define the actual file contents
	for _, ext := range videoExtensions {
		testName := fileName + ext
		videoPath = filepath.Join(directory, testName)
		// If os.Stat throws an error, then the file shouldn't exist.
		_, err := os.Stat(videoPath)
		if err == nil {
			break
		}
	}

	return videoPath
}

// computeHash reads in the video file and creates an SHA-256 hash from it.
func (vid Video) computeHash() {

}

// VideoFromModd reads in the modd struct, locates the associated video in the filesystem,
// and returns a video struct describing that video.
func VideoFromModd(modd *Modd) Video {
	var video Video

	// Fill in known fields
	video.Name = modd.Name
	video.CreationTime = modd.DateTimeActual
	video.Duration = modd.Duration
	video.FileSize = modd.FileSize
	video.LinkedModd = modd

	// Determine location
	video.Location = determineLocation(modd.Location)

	// Compute the hash in a separate goroutine
	go video.computeHash()

	return video
}
