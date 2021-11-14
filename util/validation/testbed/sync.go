package testbed

import (
	"fmt"
	"time"
)

func (b *Testbed) syncModemState() error {
	before := time.Now()

	req := func() {
		b.sendString("echo off\n")
		b.sendString("json\n")
		b.sendString("info\n")
		b.sendString("status\n")
		b.sendString("dump settings\n")
	}

	req()
	next := time.Now().Add(time.Second)
	timeout := time.Now().Add(5 * time.Second)
	for {
		if time.Now().After(timeout) {
			return fmt.Errorf("timeout waiting for modem responses, received: info=%v status=%v settings=%v", b.info.Timestamp.After(before), b.status.Timestamp.After(before), b.settings.Timestamp.After(before))
		}

		if time.Now().After(next) {
			next = time.Now().Add(time.Second)
			req()
			continue
		}

		time.Sleep(100 * time.Millisecond)

		if b.info.Timestamp.Before(before) {
			continue
		}

		if b.status.Timestamp.Before(before) {
			continue
		}

		if b.settings.Timestamp.Before(before) {
			continue
		}

		return nil
	}
}
