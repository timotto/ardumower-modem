<script lang="ts">
  import type { Point } from "./model";

  export let value: Point;
  export let mapItemId: string = ""
  export let editItemId: null | string = null;
  export let stroke = "yellow";
  export let strokeChoose = "black";
  export let strokeActive = "black";
  export let strokePassive = "grey";
  export let fill = "transparent";
  export let fillActive = "transparent";
  export let fillPassive = "transparent";
  export let strokeWidth = 0.1;
  export let r = 0.25;

  $: stroke = editItemId === null ? strokeChoose : mapItemId === editItemId ? strokeActive : strokePassive
  $: fill = editItemId === null ? fillActive : mapItemId === editItemId ? fillActive : fillPassive

  function click() {
    editItemId = mapItemId
  }
</script>

<circle
  cx={value.x}
  cy={value.y}
  {r}
  {stroke}
  {fill}
  stroke-width={strokeWidth}
  on:click={click}
/>
