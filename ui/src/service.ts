import type { Info, Settings, Status } from './model'
import { SaveSuccess } from './stores/success'
import { Invalid } from './stores/invalid'
import { InfoStore } from './stores/info'
import { StatusStore } from './stores/status'
import { Error as ErrorStore } from './stores/error'

const apiPath = (p: string) => `/api/modem/${p}`

class SettingsServiceClass {
  constructor() {
    this.loadFirstInfo()
  }

  public async load(): Promise<Settings> {
    const res = await fetch(apiPath('settings'))
    await assertOk(res, 'load settings')
    try {
      const body = await res.json()
      fillSecrets(body)

      return body;
    } catch (err) {
      throw new ServiceError("load settings", { action: "decode", error: err.message }, 0, "")
    }
  }

  public async save(s: Settings, original: Settings): Promise<Settings> {
    try {
      stripFillers(s, original)
      s.revision = original.revision + 1
      const before = await info()
      await this.saveToBackend(s)
      await modemReboot(before)
      SaveSuccess.set({action: 'save-settings', date: new Date()})
      Invalid.set("")
      return this.load()
    } catch (err) {
      if (err instanceof ServiceError) {
        if (err.details.action === 'invalid')
          Invalid.set(err.details.error)
      }
      throw err
    }
  }

  private async saveToBackend(s: Settings): Promise<Settings> {
    const res = await fetch(apiPath('settings'), {
      method: 'POST',
      body: JSON.stringify(s),
      headers: {
        'Content-Type': 'application/json',
      }
    })
    await assertOk(res, 'save settings')
    return res.json()
  }

  private async loadFirstInfo(): Promise<void> {
    try {
      await info()
      await status()
    } catch (err) {
      ErrorStore.set(err)
    }
  }
}

class BluetoothServiceClass {
  public async resetPairings() {
    try {
      const before = await info()
      const res = await fetch(apiPath('bluetooth/reset'), { method: 'POST' })
      await assertOk(res, 'reset Bluetooth pairings')
      await res.json()
      await modemReboot(before)
      SaveSuccess.set({action: 'clear-pairings', date: new Date()})
    } catch (err) {
      ErrorStore.set(err)
    }
  }
}

const info = async (timeout: number = 5000): Promise<Info> => {
  const controller = new AbortController();
  const id = setTimeout(() => controller.abort(), timeout);

  const res = await fetch(apiPath('info'), { signal: controller.signal })
  clearTimeout(id);

  await assertOk(res, 'get modem info')
  try {
    const info = await res.json()
    InfoStore.set(info)

    return info
  } catch (err) {
    throw new ServiceError("get modem info", { action: "decode", error: err.message }, 0, "")
  }
}

const status = async (timeout: number = 5000): Promise<Status> => {
  const controller = new AbortController();
  const id = setTimeout(() => controller.abort(), timeout);

  const res = await fetch(apiPath('status'), { signal: controller.signal })
  clearTimeout(id);

  await assertOk(res, 'get modem status')
  try {
    const status = await res.json()
    StatusStore.set(status)

    return status
  } catch (err) {
    throw new ServiceError("get modem status", { action: "decode", error: err.message }, 0, "")
  }
}

const modemReboot = async (before: Info, timeout: number = 10000) => {
  const limit = millis() + timeout

  while (millis() < limit) {
    try {
      const now = await info(1000);
      if (now.uptime < before.uptime) return;
    } catch (err) {
      console.log("await reboot", err);
    }
    await sleep(500);
  }

  throw new ServiceError('await-reboot', { error: 'timeout' }, 0, '')
}

const assertOk = async (res: Response, action: string) => {
  if (res.ok) return;

  const contentType = res.headers.get('content-type')
  const isJson = !!contentType && contentType.startsWith('application/json')
  let details: ErrorDetails
  if (isJson) {
    details = await res.json()
  } else {
    const text = await res.text()
    details = { action, error: text }
  }

  throw new ServiceError(action, details, res.status, res.statusText)
}

const millis = (): number => new Date().getTime()

const sleep = async (ms: number) => new Promise((resolve) => setTimeout(resolve, ms));

const secretFiller = (): string => {
  return '(unchanged)'
}

const keyAt = (o: any, path: string[]) => path.length === 0 ? o : keyAt(o[path[0]], path.slice(1))

const strip = (a: any, b: any, path: string[]): boolean => keyAt(a, path) !== keyAt(b, path)

const stripFillers = (s: Settings, original: Settings) => {
  s.general.has_password = s.general.password !== original.general.password
  s.web.has_password = s.web.password !== original.web.password
  s.wifi.has_sta_psk = s.wifi.sta_psk !== original.wifi.sta_psk
  s.wifi.has_ap_psk = s.wifi.ap_psk !== original.wifi.ap_psk
  s.bluetooth.has_pin = s.bluetooth.pin !== original.bluetooth.pin
  s.mqtt.has_password = s.mqtt.password !== original.mqtt.password
}

const fillSecrets = (s: Settings) => {
  if (s.general.has_password) s.general.password = 999999
  else s.general.password = 0

  if (s.web.has_password) s.web.password = secretFiller()
  else s.web.password = ''

  if (s.wifi.has_sta_psk) s.wifi.sta_psk = secretFiller()
  else s.wifi.sta_psk = ''

  if (s.wifi.has_ap_psk) s.wifi.ap_psk = secretFiller()
  else s.wifi.ap_psk = ''

  if (s.bluetooth.has_pin) s.bluetooth.pin = 999999
  else s.bluetooth.pin = 0

  if (s.mqtt.has_password) s.mqtt.password = secretFiller()
  else s.mqtt.password = ''
}

export interface ErrorDetails {
  action?: string
  error: string
}

export class ServiceError extends Error {
  constructor(public readonly action: string,
    public readonly details: ErrorDetails,
    public readonly statusCode: number,
    public readonly statusText: string) {
    super(action)
  }
}

export const InfoService = { info, status }

export const SettingsService = new SettingsServiceClass();

export const BluetoothService = new BluetoothServiceClass();
