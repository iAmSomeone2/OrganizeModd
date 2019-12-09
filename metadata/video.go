package metadata

import (
	"crypto/sha256"
	"errors"
	"fmt"
	"log"
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
	UnkownContainer
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
	case UnkownContainer:
		returnStr.WriteString("unkown")
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
	UnknownVidCodec
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
	case UnknownVidCodec:
		returnStr.WriteString("unkown")
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
	// UnknownAudCodec is the catchall
	UnknownAudCodec
)

func (ac AudioCodec) String() string {
	var returnStr strings.Builder

	switch ac {
	case AC3:
		returnStr.WriteString("ac3/Dolby Digital")
		break
	case AAC:
		returnStr.WriteString("aac/mp4")
		break
	case VORBIS:
		returnStr.WriteString("ogg-vorbis")
		break
	case UnknownAudCodec:
		returnStr.WriteString("unkown")
		break
	}

	return returnStr.String()
}

// Video is a struct representing all of the relevant metadata for a video file.
type Video struct {
	FileStat     os.FileInfo // OS-provided stats from the file. Includes base name and size
	Location     string      // Location of file on system
	CreationTime time.Time   // Time when video was first created
	Duration     float64     // Number of seconds in video file
	VidContainer Container   // Container type of the video file.
	VidCodec     VideoCodec  // Video encoding type
	AudCodec     AudioCodec  // Audio encoding type
	SHA256Hash   []byte      // SHA-256 hash of the file contents
	LinkedModd   *Modd       // Modd struct associated with this video file
}

func (video Video) String() string {
	var result strings.Builder

	result.WriteString(fmt.Sprintf(`{ %x }`, video.SHA256Hash))

	return result.String()
}

// MarshalJSON provides the functionality to convert the Video struct to a JSON
// format
func (video Video) MarshalJSON() ([]byte, error) {
	var jsonStr strings.Builder
	var err error

	jsonStr.WriteString(fmt.Sprintf(`{"name":"%s","size":%d,"location":"%s"`,
		video.FileStat.Name(), video.FileStat.Size(), video.Location))
	dateTimeTxt, err := video.CreationTime.MarshalJSON()
	jsonStr.WriteString(fmt.Sprintf(`,"creation_time":%s,"duration":%0.15f`,
		dateTimeTxt, video.Duration))
	jsonStr.WriteString(fmt.Sprintf(`,"container":"%s","video_codec":"%s","audio_codec":"%s"`,
		video.VidContainer, video.VidCodec, video.AudCodec))
	jsonStr.WriteString(fmt.Sprintf(`,"sha256_hash":"%x","linked_modd":`, video.SHA256Hash))
	moddJSON, err := video.LinkedModd.MarshalJSON()
	jsonStr.Write(moddJSON)
	jsonStr.WriteRune('}')

	return []byte(jsonStr.String()), err
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
func (video *Video) computeHash(done chan bool) {
	hash := sha256.New()
	data := make([]byte, video.FileStat.Size()/10)
	file, err := os.OpenFile(video.Location, os.O_RDONLY, 0755)
	if err != nil {
		log.Print(err)
	}
	defer file.Close()

	readCount, err := file.Read(data)
	if err != nil {
		log.Print(err)
	} else if int64(readCount) != video.FileStat.Size()/10 {
		log.Print(errors.New("could not read entire file"))
	}

	_, err = hash.Write(data)
	if err != nil {
		log.Print(err)
	}

	video.SHA256Hash = hash.Sum(nil)
	data = nil // Hopefully, that clears up the RAM
	done <- true
}

// VideoFromModd reads in the modd struct, locates the associated video in the filesystem,
// and returns a video struct describing that video.
func (modd *Modd) VideoFromModd() Video {
	var video Video

	// Determine location
	video.Location = determineLocation(modd.Location)
	videoStat, err := os.Stat(video.Location)
	if err != nil {
		log.Panic(err)
	}
	video.FileStat = videoStat

	// Compute the hash in a separate goroutine
	done := make(chan bool)
	go video.computeHash(done)

	// Fill in known fields
	video.CreationTime = modd.DateTimeActual
	video.Duration = modd.Duration
	video.LinkedModd = modd

	// Deal with getting containers and codecs
	vidExt := filepath.Ext(video.Location)
	switch vidExt {
	case ".mpg", ".MPG", ".mpeg", ".MPEG":
		video.VidContainer = MPEG
		break
	case ".mp4", ".MP4", ".m4v", ".M4V":
		video.VidContainer = MP4
		break
	case ".mkv", ".MKV":
		video.VidContainer = MKV
		break
	default:
		video.VidContainer = UnkownContainer
	}

	// Codec detection isn't ready yet.
	video.VidCodec = UnknownVidCodec
	video.AudCodec = UnknownAudCodec

	<-done // Wait for hash algo to complete
	return video
}
