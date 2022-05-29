import type { Settings } from '../model'

export const different = (a: Settings, b: Settings): boolean =>
  JSON.stringify(a) !== JSON.stringify(b)

export const clone = (a: Settings): Settings =>
  JSON.parse(JSON.stringify(a))

