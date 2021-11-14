<script lang="ts">
  import "carbon-components-svelte/css/g10.css";
  import { Button } from "carbon-components-svelte";
  import IconClear from "carbon-icons-svelte/lib/Close16";
  import IconSave from "carbon-icons-svelte/lib/Checkmark16";

  import { Busy } from "../stores/busy";
  import { Dirty } from "../stores/dirty";
  import { FrontendSettings } from "../stores/frontend";
  import { BackendSettings } from "../stores/backend";
  import type { Readable } from "svelte/store";
  import type { Settings } from "../model";

  interface settable {
    commit(it: Settings);
  }

  const copy = (from: Readable<Settings>, to: settable) =>
    from.subscribe((s: Settings) => {
      to.commit(JSON.parse(JSON.stringify(s)));
    })();

  function save() {
    copy(FrontendSettings, BackendSettings);
  }

  function revert() {
    copy(BackendSettings, FrontendSettings);
  }
</script>

{#if $Dirty}
  <Button
    on:click={save}
    disabled={$Busy}
    kind="primary"
    icon={IconSave}
    iconDescription="Save changes"
  />
  <Button
    on:click={revert}
    disabled={$Busy}
    kind="secondary"
    icon={IconClear}
    iconDescription="Revert changes"
  />
{/if}
