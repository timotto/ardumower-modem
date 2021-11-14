package readline

import "strings"

type LineListener func(string)

type ReadLine struct {
	cb   LineListener
	eol  string
	line string
}

func NewReadLine(eol string, cb LineListener) *ReadLine {
	return &ReadLine{
		cb:  cb,
		eol: eol,
	}
}

func (r *ReadLine) Write(data []byte) (int, error) {
	for _, c := range data {
		r.line += string([]byte{c})
		r.process()
	}

	return len(data), nil
}

func (r *ReadLine) WriteString(data string) {
	r.Write([]byte(data))
}

func (r *ReadLine) process() {
	if !strings.HasSuffix(r.line, r.eol) {
		return
	}

	line := strings.TrimSuffix(r.line, r.eol)
	r.line = ""
	r.cb(line)
}
