package testbed

import "time"

type (
	consoleResponse struct {
		Result   string         `json:"result"`
		Kind     string         `json:"kind"`
		Info     *ModemInfo     `json:"info"`
		Status   *ModemStatus   `json:"status"`
		Settings *ModemSettings `json:"settings"`
	}

	ModemInfo struct {
		Timestamp time.Time `json:"-"`
		Firmware  string    `json:"firmware"`
		Name      string    `json:"name"`
		Esp32     struct {
			ChipId string `json:"chip_id"`
		} `json:"esp32"`
		Git struct {
			Hash string `json:"hash"`
			Time string `json:"time"`
			Tag  string `json:"tag"`
		} `json:"git"`
	}

	ModemStatus struct {
		Timestamp time.Time `json:"-"`
		Uptime    int       `json:"uptime"`
		Wifi      struct {
			Mode string `json:"mode"`
			Ap   struct {
				Enabled bool   `json:"enabled"`
				Ssid    string `json:"ssid"`
				Ip      string `json:"ip"`
			} `json:"ap"`
			Sta struct {
				Enabled bool   `json:"enabled"`
				Ssid    string `json:"ssid"`
				Ip      string `json:"ip"`
			} `json:"sta"`
		} `json:"wifi"`
	}

	ModemSettings struct {
		Timestamp time.Time `json:"-"`
		Revision  int       `json:"revision"`
		General   struct {
			Name       string `json:"name"`
			Encryption bool   `json:"encryption"`
			Password   int    `json:"password"`
		} `json:"general"`

		Web struct {
			Protected bool   `json:"protected"`
			Username  string `json:"username"`
			Password  string `json:"password"`
		} `json:"web"`

		Wifi struct {
			Mode    string `json:"mode"`
			StaSsid string `json:"sta_ssid"`
			StaPsk  string `json:"sta_psk"`
			ApSsid  string `json:"ap_ssid"`
			ApPsk   string `json:"ap_psk"`
		} `json:"wifi"`

		Bluetooth struct {
			Enabled    bool `json:"enabled"`
			PinEnabled bool `json:"pin_enabled"`
			Pin        int  `json:"pin"`
		} `json:"bluetooth"`

		Mqtt struct {
			Enabled         bool   `json:"enabled"`
			Prefix          string `json:"prefix"`
			Server          string `json:"server"`
			Username        string `json:"username"`
			Password        string `json:"password"`
			PublishStatus   bool   `json:"publish_status"`
			PublishFormat   int    `json:"publish_format"`
			PublishInterval int    `json:"publish_interval"`
			HomeAssistant   bool   `json:"ha"`
		} `json:"mqtt"`

		Prometheus struct {
			Enabled bool `json:"enabled"`
		} `json:"prometheus"`
	}
)
