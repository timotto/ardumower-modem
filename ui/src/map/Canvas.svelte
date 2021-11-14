<script lang="ts">
  import ZoomSvg from "@svelte-parts/zoom/svg";

  import type { MapPresentation } from "./model";
  import { MapStore } from "./service";

  function transform(p: MapPresentation): string {
    return `rotate(${p.rotation})`;
  }
</script>

<main>
  <ZoomSvg viewBox={$MapStore.presentation.viewBox}>
    <g transform={transform($MapStore.presentation)}>
      <slot />
    </g>
  </ZoomSvg>
</main>
