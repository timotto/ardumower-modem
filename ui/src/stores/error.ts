import { derived, writable } from 'svelte/store';

const errors = []

export const Error = writable(undefined)

export const Errors = derived(Error, $Error => {
  if (!$Error) return [...errors]

  errors.push($Error)

  return [...errors]
})
