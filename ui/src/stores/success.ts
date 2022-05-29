import { derived, writable } from 'svelte/store';

const total = []

export interface SuccessNotification {
  action: string
  date: Date
}

export const SaveSuccess = writable<SuccessNotification>()

export const SuccessfulSaves = derived(SaveSuccess, val => {
  if (!val) return [...total]

  total.push(val)
  
  return [...total]
})
