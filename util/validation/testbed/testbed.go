package testbed

import (
	"encoding/json"
	"fmt"
	"os"
	"time"

	"github.com/tarm/serial"
)

type (
	Testbed struct {
		serialPort string
		staSsid    string
		staPsk     string
		out        chan []byte
		started    bool

		info     ModemInfo
		status   ModemStatus
		settings ModemSettings
	}
)

func NewTestBed(serialPort string) (*Testbed, error) {
	staSsid, ok := os.LookupEnv("VALIDATE_WIFI_STA_SSID")
	if !ok {
		return nil, fmt.Errorf("VALIDATE_WIFI_STA_SSID must be defined")
	}
	staPsk, ok := os.LookupEnv("VALIDATE_WIFI_STA_PSK")
	if !ok {
		return nil, fmt.Errorf("VALIDATE_WIFI_STA_PSK must be defined")
	}

	b := &Testbed{
		serialPort: serialPort,
		staSsid:    staSsid,
		staPsk:     staPsk,
		out:        make(chan []byte),
	}

	c := &serial.Config{Name: b.serialPort, Baud: 115200}
	if s, err := serial.OpenPort(c); err != nil {
		return nil, err
	} else {
		go b.worker(s)
	}

	b.sendString("\necho off\n")
	time.Sleep(100 * time.Millisecond)
	b.started = true

	if err := b.syncModemState(); err != nil {
		return nil, err
	}

	if err := b.assertUniqueName(); err != nil {
		return nil, err
	}

	return b, nil
}

func (b *Testbed) DeviceName() string {
	return fmt.Sprintf("MowerSim-%s", b.info.Esp32.ChipId)
}

func (b *Testbed) uploadSettings() error {
	data, err := json.Marshal(&b.settings)
	if err != nil {
		return err
	}

	b.sendString("load settings\n")
	b.send(data)
	b.sendString("\n")

	time.Sleep(2 * time.Second)

	return b.syncModemState()
}

func (b *Testbed) sendString(data string) {
	b.send([]byte(data))
}

func (b *Testbed) send(data []byte) {
	b.out <- data
}
