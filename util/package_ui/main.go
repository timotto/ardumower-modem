package main

import (
	"fmt"

	"github.com/ardumower/esp_modem/util/package_ui/internal/c"
)

func dataDirectory() string {
	return "../../ui/public/"
}

func bundleFile() string {
	return "../../src/asset_bundle.h"
}

func main() {

	bundle, err := c.CollectAssetBundle(dataDirectory(), true)
	if err != nil {
		panic(err)
	}

	bundle.GenerateHeaderFile(bundleFile())

	fmt.Printf("Bundle size: %s\n", formatBytes(bundle.Size()))
}

func formatBytes(bytes int) string {
	var divisor float64
	var unit string

	switch {
	case bytes > 1048576:
		divisor = 1048576
		unit = "MB"
	case bytes > 1024:
		divisor = 1024
		unit = "kB"
	default:
		divisor = 1
		unit = "byte"
	}

	return fmt.Sprintf("%.1f %s", float64(bytes)/divisor, unit)
}
