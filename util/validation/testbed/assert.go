package testbed

import (
	"fmt"
	"time"
)

func (b *Testbed) AssertBluetoothEnabledWithoutPin() error {
	var check = func() bool { return b.settings.Bluetooth.Enabled && !b.settings.Bluetooth.PinEnabled }
	if check() {
		return nil
	}

	b.settings.Bluetooth.Enabled = true
	b.settings.Bluetooth.PinEnabled = false

	if err := b.uploadSettings(); err != nil {
		return err
	}

	if !check() {
		return fmt.Errorf("failed to enable bluetooth, still enabled=%v pin_enabled=%v", b.settings.Bluetooth.Enabled, b.settings.Bluetooth.PinEnabled)
	}

	return nil
}

func (b *Testbed) AssertWifiStaIp() (string, error) {
	var checkConfig = func() bool {
		return b.settings.Wifi.Mode == "sta" &&
			b.settings.Wifi.StaSsid == b.staSsid &&
			b.settings.Wifi.StaPsk == b.staPsk
	}
	var checkConnection = func() bool {
		return b.status.Wifi.Sta.Enabled && b.status.Wifi.Sta.Ip != "" && b.status.Wifi.Sta.Ip != "0.0.0.0"
	}

	if !checkConfig() {
		b.settings.Wifi.Mode = "sta"
		b.settings.Wifi.StaSsid = b.staSsid
		b.settings.Wifi.StaPsk = b.staPsk

		if err := b.uploadSettings(); err != nil {
			return "", err
		}

		if !checkConfig() {
			return "", fmt.Errorf("failed to change wifi settings")
		}
	}

	timeout := time.Now().Add(5 * time.Second)
	for !checkConnection() {
		if time.Now().After(timeout) {
			return "", fmt.Errorf("timeout")
		}
		b.sendString("status\n")
		time.Sleep(time.Second)
	}

	return b.status.Wifi.Sta.Ip, nil
}

func (b *Testbed) AssertWebWithCredentials() (string, string, error) {
	var check = func() bool {
		return b.settings.Web.Protected &&
			b.settings.Web.Username != "" &&
			b.settings.Web.Password != ""
	}

	if !check() {
		b.settings.Web.Protected = true
		b.settings.Web.Username = "test-user"
		b.settings.Web.Password = fmt.Sprintf("pwd-%v", time.Now().Unix())

		if err := b.uploadSettings(); err != nil {
			return "", "", err
		}

		if !check() {
			return "", "", fmt.Errorf("failed to change web settings")
		}
	}

	return b.settings.Web.Username, b.settings.Web.Password, nil
}

func (b *Testbed) assertUniqueName() error {
	name := b.DeviceName()
	if b.info.Name == name {
		return nil
	}

	b.settings.General.Name = name

	if err := b.uploadSettings(); err != nil {
		return err
	}

	if b.info.Name != name {
		return fmt.Errorf("failed to change name to %v, name is %v instead", name, b.info.Name)
	}

	return nil
}
