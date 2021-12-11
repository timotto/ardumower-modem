import { writable } from "svelte/store"
import type { Status } from "../model"

export const StatusStore = writable<Status>()
