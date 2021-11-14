import { writable } from 'svelte/store';
import type { Settings } from '../model'
import { SettingsService } from '../service'

import { clone } from './util'
import { FrontendSettings } from './frontend'
import { Busy } from './busy'
import { Error } from './error'
import { SaveSuccess } from './success'

function createBackendSettings() {
  const { subscribe, set } = writable<Settings>();

  let original: Settings
  const maybeSet = async (fn: Promise<Settings>) => {
    try {
      Busy.set(true)
      const state = await fn
      set(clone(state))
      FrontendSettings.set(clone(state))
      Error.set(undefined)
      original = clone(state)
    } catch (error) {
      Error.set(error)
    } finally {
      Busy.set(false)
    }
  }

  const loadFromBackend = async () => maybeSet(SettingsService.load())

  const writeToBackend = async (it: Settings) => maybeSet(SettingsService.save(it, original))

  loadFromBackend()

  return {
    subscribe,
    set,
    commit: (it: Settings) => writeToBackend(it),
  };
}

export const BackendSettings = createBackendSettings()
