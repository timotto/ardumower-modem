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

		relayUrl      string
		relayUser     string
		relayPassword string

		out     chan []byte
		started bool

		info     ModemInfo
		status   ModemStatus
		settings ModemSettings
	}
)

func NewTestBed(serialPort string) (*Testbed, error) {
	b := &Testbed{
		serialPort: serialPort,
		out:        make(chan []byte),
	}

	var err error
	if b.staSsid, err = mustEnv("VALIDATE_WIFI_STA_SSID"); err != nil {
		return nil, err
	}
	if b.staPsk, err = mustEnv("VALIDATE_WIFI_STA_PSK"); err != nil {
		return nil, err
	}

	if b.relayUrl, err = mustEnv("VALIDATE_RELAY_SERVER_URL"); err != nil {
		return nil, err
	}
	if b.relayUser, err = mustEnv("VALIDATE_RELAY_USERNAME"); err != nil {
		return nil, err
	}
	if b.relayPassword, err = mustEnv("VALIDATE_RELAY_PASSWORD"); err != nil {
		return nil, err
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

func mustEnv(key string) (string, error) {
	if val, ok := os.LookupEnv(key); !ok {
		return "", fmt.Errorf("%v must be defined", key)
	} else {
		return val, nil
	}
}
