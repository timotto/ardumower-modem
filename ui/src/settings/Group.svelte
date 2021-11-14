<script lang="ts">
  import { fade, slide } from "svelte/transition";
  import { AccordionItem } from "carbon-components-svelte";

  export let title: string;
  export let settings: any = {};
  export let original: any = {};
  export let dirty = false;
  export let open = true;
  export let enabledKey = "enabled";
  $: dirty = JSON.stringify(settings) !== JSON.stringify(original);

  let titleMod = title;

  $: titleMod = dirty ? `${title} (*)` : title;
  let more = true;
</script>

<AccordionItem title={titleMod} {open}>
  <slot />
  {#if settings[enabledKey]}
    <div transition:slide on:introend={() => (more = true)}>
      {#if more}
        <slot name="enabled" />
      {/if}
    </div>
  {/if}
</AccordionItem>
