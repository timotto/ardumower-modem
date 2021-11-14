export interface Point {
  x: number;
  y: number;
}

export interface Edge {
  begin: Point;
  end: Point;
}

export interface Area {
  points: Point[]
}

export interface Perimeter extends Area {
}

export interface Exclusion extends Area {
}

export interface Map {
  perimeter: Perimeter
  exclusions: Exclusion[]
}

export interface MapPresentation {
  boundary: {
    a: Point,
    b: Point,
  }
  center: Point
  rotation: number
  viewBox: string
}
