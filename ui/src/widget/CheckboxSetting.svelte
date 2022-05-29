<script lang="ts">
  import { Button, Toggle } from "carbon-components-svelte";
  import IconClear from "carbon-icons-svelte/lib/CloseOutline16";
  import { Busy } from "../stores/busy";

  export let label: string;
  export let key: string;
  export let value: boolean;
  export let original: boolean;

  let dirty = false;
  $: dirty = value !== original;

  let labelMod = label;
  $: labelMod = dirty ? `${label} (*)` : label;

  function revert() {
    value = original;
  }
</script>

<main>
  <Toggle labelText={labelMod} bind:toggled={value} disabled={$Busy} />
  {#if dirty}
    <Button
      on:click={revert}
      disabled={$Busy}
      iconDescription="Revert changes"
      kind="ghost"
      icon={IconClear}
      hasIconOnly={true}
    />
  {/if}
</main>

<style>
  main {
    display: flex;
    flex-direction: row;
  }
</style>
