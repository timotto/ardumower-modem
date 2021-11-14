import { writable } from "svelte/store";
import type { Map, MapPresentation, Point } from "./model"
import { mockMapData } from "./mock"
import { rotatePointsAroundOrigin } from "./geometry";

export interface StoredMap {
  map: Map,
  presentation: MapPresentation,
}
export let MapStore = writable<StoredMap>()

interface PresentationTuningParameters {
  padding: number
}

const presentationParameterTuning: PresentationTuningParameters = {
  padding: 1,
}

const deg2rad = (deg: number): number => deg * Math.PI / 180

const calculatePresentation = (m: Map, p: PresentationTuningParameters = presentationParameterTuning): MapPresentation => {
  const rotation = calculateRotation(m)
  const pointsRotated: Point[] = rotatePointsAroundOrigin(m.perimeter.points, deg2rad(rotation))
  const boundary = calculateBoundary(pointsRotated)
  const center = boundary.center
  boundary.a.x -= p.padding
  boundary.a.y -= p.padding
  boundary.b.x += 2 * p.padding
  boundary.b.y += 2 * p.padding
  const viewBox = `${boundary.a.x} ${boundary.a.y} ${boundary.b.x - boundary.a.x} ${boundary.b.y - boundary.a.y}`

  // const allX: number[] = m.perimeter.points.map(({ x }) => x)
  // const allY: number[] = m.perimeter.points.map(({ y }) => y)
  // const extremes = { x: [smallest(allX), largest(allX)], y: [smallest(allY), largest(allY)] }
  // const center: Point = { x: middle(extremes.x), y: middle(extremes.y) }
  // const boundary = {
  //   a: { x: extremes.x[0], y: extremes.y[0] },
  //   b: { x: extremes.x[1], y: extremes.y[1] },
  // }

  // const viewBox = `${extremes.x[0]} ${extremes.y[0]} ${extremes.x[1] - extremes.x[0]} ${extremes.y[1] - extremes.y[0]}`;

  return { center, boundary, rotation, viewBox }
}

const calculateBoundary = (value: Point[]): { a: Point, b: Point, center: Point } => {
  const allX: number[] = value.map(({ x }) => x)
  const allY: number[] = value.map(({ y }) => y)
  const extremes = { x: [smallest(allX), largest(allX)], y: [smallest(allY), largest(allY)] }
  const center: Point = { x: middle(extremes.x), y: middle(extremes.y) }

  return {
    a: { x: extremes.x[0], y: extremes.y[0] },
    b: { x: extremes.x[1], y: extremes.y[1] },
    center,
  }
}

const calculateRotation = (m: Map): number => {
  if (m.perimeter.points.length < 3) return 0;

  interface edge {
    edge: Point[],
    length: number,
    orientation: number,
  }

  const dist = (a: Point[]): number => Math.sqrt(Math.pow(a[0].x - a[1].x, 2) + Math.pow(a[0].y - a[1].y, 2))

  const rad2deg = (rad: number): number => rad * 180 / Math.PI

  const angle = (a: Point[]): number => (rad2deg(Math.atan2(a[0].x - a[1].x, a[0].y - a[1].y)) + 90) % 360

  const raw: edge[] = [...m.perimeter.points, m.perimeter.points[0]]
    .map((_, i, a) => i === 0 ? undefined : [a[i - 1], a[i]])
    .filter(x => x !== undefined)
    .map(edge => ({ edge, length: dist(edge), orientation: angle(edge) }))

  const sorted = raw.sort((a, b) => b.length - a.length)

  // console.log({ sorted, })

  return sorted[0].orientation;
}

const smallest = (a: number[]): number =>
  a.reduce((a, b) => (a < b ? a : b), Number.MAX_VALUE);
const largest = (a: number[]): number =>
  a.reduce((a, b) => (a > b ? a : b), Number.MIN_VALUE);
const middle = (a: number[]): number =>
  Math.min(a[0], a[1]) + Math.abs(a[0] - a[1]) / 2;

const p2p = (a: { X: number, Y: number }): { x: number, y: number } => ({ x: a.X, y: -a.Y })

let mockMap: Map = {
  perimeter: { points: mockMapData[0].perimeter.map(p2p) },
  exclusions: mockMapData[0].exclusions.map(e => ({ points: e.map(p2p) })),
}

let presentation: MapPresentation = calculatePresentation(mockMap)

MapStore.set({ map: mockMap, presentation })
