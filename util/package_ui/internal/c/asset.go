package c

import (
	"fmt"
	"strings"
	"time"
)

type Asset struct {
	Index int
	Path  string
	Data  []byte
	Etag  string
	Time  time.Time
}

func (a Asset) DataLine() string {
	return fmt.Sprintf("const unsigned char asset_%d_data[%d] PROGMEM = %s;\n", a.Index, len(a.Data), asByteArray(a.Data))
}

func (a Asset) AsStruct() string {
	return fmt.Sprintf(`  {
    path: "%s",
    mime: "%s",
    data: asset_%d_data,
    size: %d,
    etag: "%s",
    time: "%s",
  }`, a.Path, a.mimeType(), a.Index, len(a.Data), a.Etag, a.Time.Format(time.RFC3339))
}

func (a Asset) mimeType() string {
	var suffix string
	if index := strings.LastIndex(a.Path, "."); index > -1 {
		suffix = a.Path[index+1:]
	}
	switch suffix {
	case "html":
		return "text/html"
	case "js":
		return "application/javascript"
	case "css":
		return "text/css"
	case "png":
		return "image/png"
	case "ico":
		return "image/x-icon"
	default:
		return "text/plain"
	}
}
