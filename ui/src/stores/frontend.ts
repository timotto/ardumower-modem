import { writable } from 'svelte/store';
import type { Settings } from '../model'

function createFrontendSettings() {
  const { subscribe, set } = writable<Settings>();

  return {
    subscribe,
    set,
    commit: (it: Settings) => set(it),
  };
}

export const FrontendSettings = createFrontendSettings()
