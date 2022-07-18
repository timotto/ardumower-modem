<script lang="ts">
  import { Button, Select, SelectItem } from "carbon-components-svelte";
  import IconClear from "carbon-icons-svelte/lib/CloseOutline16";
import { createEventDispatcher } from "svelte";
import type { ChangeEventValue } from "../model";
  import type { Option } from "../model/ui";
  import { Busy } from "../stores/busy";

  export let label: string;
  export let key: string;
  export let value: any;
  export let original: any;
  export let options: Option<any>[];
  export let disabled: boolean = false;

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
  <Select
    labelText={labelMod}
    bind:selected={value}
    disabled={disabled || $Busy}
    on:change="{(e) => change(e, value)}"
  >
    {#each options as option}
      <SelectItem text={option.label} value={option.value} />
    {/each}
  </Select>
  {#if dirty}
    <Button
      on:click={revert}
      disabled={disabled || $Busy}
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
