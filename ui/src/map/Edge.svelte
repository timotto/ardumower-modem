<script lang="ts">
  import type { Edge } from "./model";

  export let value: Edge;
  export let mapItemId: string = "";
  export let editItemId: null | string = null;

  export let stroke = "yellow";
  export let strokeChoose = "black";
  export let strokeActive = "black";
  export let strokePassive = "grey";
  export let strokeWidth = 0.2;

  $: stroke = editItemId === null ? strokeChoose : mapItemId === editItemId ? strokeActive : strokePassive

  function click() {
    editItemId = mapItemId
  }
</script>

<line
  on:click={click}
  x1={value.begin.x}
  y1={value.begin.y}
  x2={value.end.x}
  y2={value.end.y}
  {stroke}
  stroke-width={strokeWidth}
/>
