package tests

import (
	"github.com/ardumower/esp_modem/util/validation/tests/http_adapter"
	"os"
	"testing"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/ardumower/esp_modem/util/validation/testbed"
	"github.com/ardumower/esp_modem/util/validation/tests/ble"
	"github.com/ardumower/esp_modem/util/validation/tests/ui"
)

func Run() bool {
	bed, err := testbed.NewTestBed(serialPort())
	if err != nil {
		panic(err)
	}

	if isEnabled("ble") {
		_ = ble.BleValidationSuite(bed)
	}

	if isEnabled("ui") {
		_ = ui.UiValidationSuite(bed)
	}

	if isEnabled("http") {
		_ = http_adapter.ValidationSuite(bed)
	}

	return run(&testing.T{})
}

func run(t *testing.T) bool {
	RegisterFailHandler(Fail)
	return RunSpecs(t, "Validation Suite")
}

func isEnabled(key string) bool {
	if len(os.Args) < 3 {
		return true
	}

	for _, arg := range os.Args[2:] {
		if arg == key {
			return true
		}
	}

	return false
}

func serialPort() string {
	if len(os.Args) < 2 {
		if port, ok := os.LookupEnv("SERIAL_PORT"); ok {
			return port
		}
	} else {
		return os.Args[1]
	}

	panic("either define SERIAL_PORT or run with name of serial port as argument")
}
