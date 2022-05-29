<script lang="ts">
  import type { Settings } from "../model";
  import CheckboxSetting from "../widget/CheckboxSetting.svelte";
  import TextSetting from "../widget/TextSetting.svelte";
  import Group from "./Group.svelte";

  export let settings: Settings.General;
  export let original: Settings.General;
</script>

<Group title="General" {settings} {original}>
  <TextSetting
    label="Device name"
    key="general.name"
    helpText="The name is used during Bluetooth discovery, as WiFi hostname, MQTT topic, etc."
    bind:value={settings.name}
    bind:original={original.name}
  />
  <CheckboxSetting
    label="ArduMower Encryption"
    key="general.encryption"
    bind:value={settings.encryption}
    bind:original={original.encryption}
  />
  <TextSetting
    label="ArduMower encryption password"
    key="general.password"
    helpText="The password configured in config.h"
    kind="password"
    disabled={!settings.encryption}
    bind:value={settings.password}
    bind:original={original.password}
  />
</Group>
