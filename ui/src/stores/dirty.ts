import { derived } from 'svelte/store';
import { BackendSettings } from './backend';
import { FrontendSettings } from './frontend';
import { different } from './util';

export const Dirty = derived(
  [BackendSettings, FrontendSettings],
  ([backend, frontend]) =>
    different(backend, frontend))
