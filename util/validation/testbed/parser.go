package testbed

import (
	"encoding/json"
	"fmt"
	"strings"
	"time"
)

func (b *Testbed) handleLineFromModem(line string) {
	if !strings.HasPrefix(line, "{") {
		return
	}

	var res consoleResponse
	if err := json.Unmarshal([]byte(line), &res); err != nil {
		fmt.Printf("Testbed failed to decode modem console response: %v\n", err.Error())
		return
	}

	if res.Info != nil {
		b.info = *res.Info
		b.info.Timestamp = time.Now()
	}

	if res.Status != nil {
		b.status = *res.Status
		b.status.Timestamp = time.Now()
	}

	if res.Settings != nil {
		b.settings = *res.Settings
		b.settings.Timestamp = time.Now()
	}
}
