import type { Point, Edge } from "./model"

export const pointsForPolygon = (p: Point[]): string => p.map(({ x, y }) => `${x},${y}`).join(' ')

export const rotatePointsAroundOrigin = (p: Point[], radians: number): Point[] =>
  ((s: number, c: number) =>
    p.map(({ x, y }) =>
      ({ x: x * c - y * s, y: x * s + y * c })))
    (Math.sin(radians), Math.cos(radians))

export const pointsToEdges = (p: Point[]): Edge[] =>
  p.length < 2
    ? []
    : [...p, p[0]]
      .map((_, i, a) => (i === 0 ? undefined : [a[i - 1], a[i]]))
      .filter((x) => x !== undefined)
      .map((p) => ({ begin: p[0], end: p[1] }));
