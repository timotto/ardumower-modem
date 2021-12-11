<script lang="ts">
  import type { Settings } from "../model";
  import TextSetting from "../widget/TextSetting.svelte";
  import CheckboxSetting from "../widget/CheckboxSetting.svelte";
  import Group from "./Group.svelte";
  import { Button } from "carbon-components-svelte";
  import ConnectionSignal16 from "carbon-icons-svelte/lib/ConnectionSignal16";
  import ConnectionSignalOff16 from "carbon-icons-svelte/lib/ConnectionSignalOff16";
  import IconReload from "carbon-icons-svelte/lib/UpdateNow32";
  import { StatusStore } from "../stores/status";
  import { InfoService } from "../service";

  export let allSettings: Settings;
  export let settings: Settings.Relay;
  export let original: Settings.Relay;

  function refresh() {
    InfoService.status()
  }
</script>

<Group title="Relay" {settings} {original}>
  <CheckboxSetting
    label="Enabled"
    key="relay.enabled"
    bind:value={settings.enabled}
    bind:original={original.enabled}
  />
  <svelte:fragment slot="enabled">
    <TextSetting
      label="URL"
      key="relay.url"
      bind:value={settings.url}
      bind:original={original.url}
    />

    <div class="details">
    {#if $StatusStore?.relay_connected}
      <p>
        <ConnectionSignal16 />
        The modem is connected to the relay server.
      </p>
    {:else}
      <p>
        <ConnectionSignalOff16 />
        The modem is not connected to the relay server.
      </p>
    {/if}
    <Button
      on:click={refresh}
      iconDescription="Check relay connection status"
      kind="ghost"
      icon={IconReload}
      hasIconOnly={true}
    />
  </div>
  </svelte:fragment>
</Group>

<style>
  p {
    padding: 1em;
    flex: 1;
  }

  .details {
    display: flex;
    flex-direction: row;
  }
</style>
