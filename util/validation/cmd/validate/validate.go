package main

import (
	"os"

	"github.com/ardumower/esp_modem/util/validation/tests"
)

func main() {
	if !tests.Run() {
		os.Exit(1)
	}
}
