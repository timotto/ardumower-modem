<script lang="ts">
  import { Button, Toggle } from "carbon-components-svelte";
  import IconClear from "carbon-icons-svelte/lib/CloseOutline16";
  import { createEventDispatcher } from "svelte";
import type { ChangeEventValue } from "../model";
  import { Busy } from "../stores/busy";

  export let label: string;
  export let key: string;
  export let value: boolean;
  export let original: boolean;
  
  let dispatch = createEventDispatcher<{ change: ChangeEventValue }>();

  const change = (event, value) => {
      dispatch('change', { event: event, value: value });
  }

  let dirty = false;
  $: dirty = value !== original;

  let labelMod = label;
  $: labelMod = dirty ? `${label} (*)` : label;

  function revert() {
    value = original;
  }
</script>

<main>
  <Toggle labelText={labelMod} bind:toggled={value} disabled={$Busy} on:change="{(e) => change(e, value)}" />
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
    margin-bottom: 1rem;
  }
</style>
