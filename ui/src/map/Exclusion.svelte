<script lang="ts">
  import { pointsForPolygon, pointsToEdges } from "./geometry";
  import type { Exclusion as ExclusionModel } from "./model";
  import Point from "./Point.svelte";
  import Edge from "./Edge.svelte";

  export let value: ExclusionModel;
  export let exclusionId: string;
  export let edit: boolean = false;
  export let editItemId: null | string = null;
</script>

<polygon
  fill="red"
  stroke="black"
  stroke-width="0.1"
  points={pointsForPolygon(value.points)}
/>

{#if edit}
  {#each value.points as point, index}
    <Point
      value={point}
      mapItemId={exclusionId + "-point-" + index}
      bind:editItemId
    />
  {/each}
  {#each pointsToEdges(value.points) as edge, index}
    <Edge
      value={edge}
      mapItemId={exclusionId + "-edge-" + index}
      bind:editItemId
    />
  {/each}
{/if}
