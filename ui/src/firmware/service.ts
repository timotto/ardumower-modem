import { Readable, writable } from "svelte/store"

export enum FirmwareUploadStatus {
  clear = 0,
  fileSelected,
  uploading,
  expectReboot,
  error,
  success,
}

export class FirmwareUploader {
  private _req: XMLHttpRequest
  private _file: null | File = null
  private _rebootAwaiter: null | (() => Promise<void>) = null

  private _status = writable<FirmwareUploadStatus>(FirmwareUploadStatus.clear)
  public get status(): Readable<FirmwareUploadStatus> { return this._status }

  private _progress = writable<number>(0)
  public get progress(): Readable<number> { return this._progress }

  private _error: null | string = null
  public get error(): null | string { return this._error }

  public set file(f: null | File) {
    this._file = f
    const s = f !== null
      ? FirmwareUploadStatus.fileSelected
      : FirmwareUploadStatus.clear
    this._status.set(s)
  }

  public async upload() {
    const data = uploadRequest(this._file)

    this._progress.set(0)
    this._status.set(FirmwareUploadStatus.uploading)

    this._rebootAwaiter = await makeRebootAwaiter()

    this._req = new XMLHttpRequest()
    this._req.open('POST', '/api/modem/ota/upload')
    this._req.upload.addEventListener('progress', this.onUploadProgress.bind(this))
    this._req.addEventListener('load', this.onLoad.bind(this))
    this._req.addEventListener('error', this.onError.bind(this))
    try {
      this._req.send(data)
    } catch (err) {
      this._error = `Upload failed (${err.message})`
      this._status.set(FirmwareUploadStatus.error)
    }
  }

  private onUploadProgress(e: ProgressEvent<XMLHttpRequestEventTarget>) {
    const progress = e.loaded / e.total * 100
    this._progress.set(progress)
  }

  private onLoad(_: ProgressEvent<XMLHttpRequestEventTarget>) {
    if (this._req.status >= 400) this.onBadResponseStatus()
    else this.onGoodResponseStatus()
  }

  private onError(e: ProgressEvent<XMLHttpRequestEventTarget>) {
    this._error = `Upload request failed`
    this._status.set(FirmwareUploadStatus.error)
  }

  private async onGoodResponseStatus() {
    try {
      const res: FirmwareUploadResponse = JSON.parse(this._req.responseText)
      if (!res.success) {
        this._error = res.result
        this._status.set(FirmwareUploadStatus.error)
        return
      }

      this._status.set(FirmwareUploadStatus.expectReboot)
      await this._rebootAwaiter()
      this._status.set(FirmwareUploadStatus.success)
    } catch (err) {
      this._error = err.message
      this._status.set(FirmwareUploadStatus.error)
    }
  }

  private onBadResponseStatus() {
    this._error = `bad response result: ${this._req.status}`
    this._status.set(FirmwareUploadStatus.error)
  }
};

interface FirmwareUploadResponse {
  result: string
  success: boolean
  md5?: string
}

const uploadRequest = (file: File): FormData => {
  const result = new FormData()
  result.append('upload', file)
  return result
}

const makeRebootAwaiter = async (timeout: number = 10000): Promise<() => Promise<void>> => {
  const before = await getModemInfo()

  const isAfter = (i: ApiModemInfoResponse): boolean => i.uptime < before.uptime

  return async (): Promise<void> => {
    const limit: number = millis() + timeout

    const isTimeout = (): boolean => millis() > limit

    while (!isTimeout()) {
      try {
        const now = await getModemInfo(2000)
        if (isAfter(now)) return
      } catch (_) {}

      await delay(500)
    }
    throw new Error('timeout waiting for reboot')
  }
}

const getModemInfo = async (timeout: number = 5000): Promise<ApiModemInfoResponse> => {
  const controller = new AbortController();
  const id = setTimeout(() => controller.abort(), timeout);
  const res = await fetch('/api/modem/info', {
    signal: controller.signal,
  })
  clearTimeout(id)
  if (res.status !== 200) throw new Error(`bad response status: ${res.statusText}`)

  return await res.json()
}

const millis = (): number => new Date().getTime()

const delay = async (ms: number) => new Promise(resolve => setTimeout(resolve, ms))

interface ApiModemInfoResponse {
  git_hash: string
  git_time: string
  git_tag: string
  uptime: number
}
