<script lang="ts">
  import type { Settings } from "../model";
  import TextSetting from "../widget/TextSetting.svelte";
  import CheckboxSetting from "../widget/CheckboxSetting.svelte";
  import Group from "./Group.svelte";
  import RelayStatus from "../relay/RelayStatus.svelte";

  export let allSettings: Settings;
  export let settings: Settings.Relay;
  export let original: Settings.Relay;
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
    <RelayStatus />
  </svelte:fragment>
</Group>
