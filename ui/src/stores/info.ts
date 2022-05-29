import { writable } from "svelte/store"
import type { Info } from "../model"

export const InfoStore = writable<Info>()
