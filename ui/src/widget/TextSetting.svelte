<script lang="ts">
  import { Button, PasswordInput, TextInput } from "carbon-components-svelte";
  import IconClear from "carbon-icons-svelte/lib/CloseOutline16";
  import { Busy } from "../stores/busy";
  import { Invalid } from "../stores/invalid";
  import { TextService } from "../text";

  export let label: string;
  export let key: string;
  export let helpText: string = "";
  export let helpTextEmpty: string = "";
  export let helpTextNotEmpty: string = "";
  export let value: any;
  export let original = value;
  export let disabled: boolean = false;
  export let readonly: boolean = false;

  export let placeholder: string = undefined;
  export let required: boolean = true;

  export let kind: "text" | "password" = "text";

  let dirty = false;
  $: dirty = value !== original;

  let labelMod = label;
  $: labelMod = dirty ? `${label} (*)` : label;

  let help: string;
  $: help =
    !!value && value.toString().length > 0
      ? !!helpTextNotEmpty && helpTextNotEmpty.length > 0
        ? helpTextNotEmpty
        : helpText
      : !!helpTextEmpty && helpTextEmpty.length > 0
      ? helpTextEmpty
      : helpText;

  let invalid = false;
  let invalidText = undefined;
  $: invalid = $Invalid === key;
  $: invalidText = !invalid ? undefined : TextService.invalidTextFor(key);

  function revert() {
    value = original;
    Invalid.set("");
  }
</script>

<main>
  {#if kind === "text"}
    <TextInput
      bind:value
      labelText={labelMod}
      helperText={help}
      disabled={disabled || $Busy}
      {readonly}
      {required}
      {invalid}
      {invalidText}
    />
  {:else if kind === "password"}
    <PasswordInput
      bind:value
      labelText={labelMod}
      {placeholder}
      {required}
      {invalid}
      {invalidText}
      helperText={help}
      disabled={disabled || $Busy}
    />
  {/if}
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
