class TextServiceClass {

  private _lang = 'en'

  public invalidTextFor(settingsKey: string): string {
    const keyParts = settingsKey.split('.')

    const result = crawl(texts.language[this._lang].settings, [...keyParts, 'invalidText'])

    if (result) return result;

    const label = crawl(texts.language[this._lang].settings, [...keyParts, 'label'])

    if (label) return `${label} cannot be empty.`;

    return undefined;
  }

  public actionResultFor(actionKey: string, resultKey: string): string {
    const fallback = `${resultKey}: ${actionKey}`

    const item = texts.language[this._lang].actions[actionKey]
    if (!item) return fallback

    const result = item[resultKey]
    if (!result) return fallback

    return result
  }
}

const crawl = (o: any, path: string[]) => (!o || !path || !path.length) ? o : crawl(o[path[0]], path.slice(1))

const texts = {
  language: {
    en: {
      actions: {
        "save-settings": {
          success: "Settings saved",
          error: "Failed to save settings"
        },
        "clear-pairings": {
          success: "Bluetooth Pairings cleared",
          error: "Failed clear Bluetooth Pairings"
        },
      },
      settings: {
        general: {
          name: {
            label: "Device name",
            helpText: "The name is used during Bluetooth discovery, as WiFi hostname, MQTT topic, etc.",
            invalidText: "The name cannot be empty. It must consist of letters, numbers, and \"-\" or \"_\" only and it must start with a letter."
          },
          encryption: {
            label: "ArduMower Encryption",
          },
          password: {
            label: "ArduMower encryption password",
            helpText: "The password configured in config.h",
          }
        },
        web: {
          protected: {
            label: "Protect this web interface",
          },
          username: {
            label: "Username",
            invalidText: "The username cannot be empty."
          },
          password: {
            label: "Password",
            invalidText: "The password cannot be empty."
          }
        },
        wifi: {
          mode: {
            label: "Mode"
          },
          sta_ssid: {
            label: "SSID",
            helpText: "Name of your WiFi network",
            invalidText: "The SSID cannot be empty"
          },
          sta_psk: {
            label: "PSK",
            helpText: "Password for your WiFi network",
            invalidText: "The WiFi password cannot be empty"
          },
          ap_ssid: {
            label: "SSID",
            helpText: "Name of the WiFi network",
            invalidText: "The SSID cannot be empty"
          },
          ap_psk: {
            label: "PSK",
            helpText: "Password for the WiFi network",
            invalidText: "The WiFi password cannot be empty"
          }
        },
        bluetooth: {
          enabled: {
            label: "Control and monitor your ArduMower with Bluetooth",
          },
          pin_enabled: {
            label: "Bluetooth pairings require a PIN code"
          },
          pin: {
            label: "PIN",
            invalidText: "The PIN cannot be empty and must be 4 - 6 digits long"
          }
        },
        relay: {
          enabled: {
            label: "Connect to your ArduMower through a Relay server"
          },
          url: {
            label: "WebSocket URL of the relay, eg wss://fluffy-unicorn-8422.relay.example.com"
          },
          username: {
            label: "Username"
          },
          password: {
            label: "Password"
          },
        },
        mqtt: {
          enabled: {
            label: "Enabled"
          },
          server: {
            label: "Server URL",
            invalidText: "The server URL should look like mqtt://hostname:port. If no port is specified it defaults to 1883. The scheme mqtt:// can be omitted."
          },
          prefix: {
            label: "Topic prefix"
          },
          username: {
            label: "Username"
          },
          password: {
            label: "Password"
          },
          publish_status: {
            label: "Publish updates"
          },
          publish_format: {
            label: "Publish format"
          },
          publish_interval: {
            label: "Seconds between updates"
          },
          ha: {
            label: "HomeAssistant integration"
          }
        },
        prometheus: {
          enabled: {
            label: "Enabled"
          }
        }
      }
    }
  }
}

export const TextService = new TextServiceClass()
