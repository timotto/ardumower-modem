export namespace Settings {
  export interface General {
    name: string
    encryption: boolean
    password: number
    has_password: boolean
  }
  export interface Web {
    protected: boolean
    username: string
    password: string
    has_password: boolean
  }
  export interface WiFi {
    mode: 'sta' | 'ap' | 'off'
    sta_ssid: string
    sta_psk: string
    has_sta_psk: boolean
    ap_ssid: string
    ap_psk: string
    has_ap_psk: boolean
  }
  export interface Bluetooth {
    enabled: boolean
    pin_enabled: boolean
    pin: number
    has_pin: boolean
  }
  export interface Mqtt {
    enabled: boolean
    server: string
    prefix: string
    username: string
    password: string
    has_password: boolean
    publish_status: boolean
    publish_format: 'json' | 'text' | 'both'
    publish_interval: number
    ha: boolean
  }
  export interface Prometheus {
    enabled: boolean
  }
}

export interface Settings {
  initialized: boolean
  revision: number
  general: Settings.General
  web: Settings.Web
  wifi: Settings.WiFi
  bluetooth: Settings.Bluetooth
  mqtt: Settings.Mqtt
  prometheus: Settings.Prometheus
}

export interface Info {
  git_hash: string
  git_time: string;
  git_tag: string;
  uptime: number
}

export interface Status {
  uptime: number
}
