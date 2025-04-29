package ble

import (
	"bytes"
	"sync"
	"time"

	"github.com/ardumower/esp_modem/util/validation/testbed"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"tinygo.org/x/bluetooth"
)

const (
	serviceUUIDString        = "0000ffe0-0000-1000-8000-00805f9b34fb"
	characteristicUUIDString = "0000ffe1-0000-1000-8000-00805f9b34fb"
)

var adapter = bluetooth.DefaultAdapter

func BleValidationSuite(bed *testbed.Testbed) bool {
	if err := adapter.Enable(); err != nil {
		panic(err)
	}

	return Describe("Bluetooth LE", func() {
		var deviceBluetoothName string
		BeforeEach(func() {
			Expect(bed.AssertBluetoothEnabledWithoutPin()).ToNot(HaveOccurred())
			deviceBluetoothName = bed.DeviceName()
		})

		It("is visible in a scan", func() {
			var actualResult bluetooth.ScanResult
			var found bool
			var err error

			done := false
			go func() {
				actualResult, found, err = scanFor(deviceBluetoothName)
				done = true
			}()

			Eventually(func() bool { return done }, "5s").Should(BeTrue())
			Expect(err).ToNot(HaveOccurred())
			Expect(found).To(BeTrue())
			Expect(actualResult.LocalName()).To(Equal(deviceBluetoothName))
		})

		It("allows communication", func() {
			foundDevice, found, err := scanFor(deviceBluetoothName)
			Expect(err).ToNot(HaveOccurred())
			Expect(found).To(BeTrue())

			device, err := adapter.Connect(foundDevice.Address, bluetooth.ConnectionParams{})
			Expect(err).ToNot(HaveOccurred())

			serviceUUID, err := bluetooth.ParseUUID(serviceUUIDString)
			Expect(err).ToNot(HaveOccurred())

			services, err := device.DiscoverServices([]bluetooth.UUID{serviceUUID})
			Expect(len(services)).ToNot(Equal(0))
			service := services[0]

			characteristicUUID, err := bluetooth.ParseUUID(characteristicUUIDString)
			Expect(err).ToNot(HaveOccurred())

			chars, err := service.DiscoverCharacteristics([]bluetooth.UUID{characteristicUUID})
			Expect(len(chars)).ToNot(Equal(0))
			char := chars[0]

			rxBuffer := &bytes.Buffer{}
			Expect(char.EnableNotifications(func(value []byte) {
				_, err := rxBuffer.Write(value)
				Expect(err).ToNot(HaveOccurred())
			})).ToNot(HaveOccurred())

			_, err = char.WriteWithoutResponse([]byte("AT+V,0x16\n"))
			Expect(err).ToNot(HaveOccurred())

			Eventually(func() string { return rxBuffer.String() }, 5*time.Second).Should(Equal("V,Ardumower Sunray,1.0.219,1,52,0x4f\r\n"))

			Expect(device.Disconnect()).ToNot(HaveOccurred())
		})
	})
}

func scanFor(name string) (bluetooth.ScanResult, bool, error) {
	var result bluetooth.ScanResult
	var found bool = false
	var err error

	wg := &sync.WaitGroup{}
	wg.Add(1)
	go func() {
		defer wg.Done()
		err = adapter.Scan(func(adapter *bluetooth.Adapter, device bluetooth.ScanResult) {
			if device.LocalName() != name {
				return
			}
			result = device
			found = true
			err = adapter.StopScan()
		})
	}()
	wg.Wait()

	return result, found, err
}
