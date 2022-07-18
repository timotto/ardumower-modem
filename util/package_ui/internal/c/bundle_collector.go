package c

import (
	"bytes"
	"compress/gzip"
	"crypto/sha256"
	b64 "encoding/base64"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
)

type bundleCollector struct {
	b *AssetBundle
	i int

	gzip bool
	path string
}

func CollectAssetBundle(path string, gzip bool) (*AssetBundle, error) {
	c := &bundleCollector{
		b: &AssetBundle{},
		i: 0,

		gzip: gzip,
		path: strings.TrimSuffix(path, "/"),
	}

	if err := c.Collect(); err != nil {
		return nil, err
	}

	return c.Bundle(), nil
}

func (b *bundleCollector) Collect() error {
	return filepath.Walk(b.path, b.collect)
}

func (b *bundleCollector) collect(path string, info fs.FileInfo, err error) error {
	if info.IsDir() {
		return nil
	}

	if strings.HasSuffix(path, ".js.map") {
		return nil
	}

	path = strings.Replace(path, "\\", "/", -1)
	name := strings.TrimPrefix(path, b.path)
	if name == "" || name == "/" {
		return nil
	}

	data, err := b.fileContent(path)
	if err != nil {
		return fmt.Errorf("cannot read %s: %w", path, err)
	}

	a := &Asset{
		Index: b.nextIndex(),
		Path:  name,
		Data:  data,
		Etag:  etagFor(data),
		Time:  info.ModTime(),
	}
	b.b.Add(a)

	return nil
}

func (b *bundleCollector) nextIndex() int {
	i := b.i
	b.i++

	return i

}

func (b *bundleCollector) fileContent(path string) ([]byte, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	if !b.gzip {
		return io.ReadAll(f)
	}

	buf := &bytes.Buffer{}
	w := gzip.NewWriter(buf)
	if _, err := io.Copy(w, f); err != nil {
		return nil, err
	}
	if err := w.Close(); err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (b *bundleCollector) Bundle() *AssetBundle {
	return b.b
}

func etagFor(data []byte) string {
	h := sha256.New()
	_, _ = h.Write(data)
	buf := &bytes.Buffer{}
	enc := b64.NewEncoder(b64.URLEncoding, buf)
	_, _ = enc.Write(h.Sum(nil))
	_ = enc.Close()
	val := buf.String()

	return val[:16]
}
