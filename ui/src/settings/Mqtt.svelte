<script lang="ts">
  import type { Settings } from "../model";
  import type { Option } from "../model/ui";
  import TextSetting from "../widget/TextSetting.svelte";
  import CheckboxSetting from "../widget/CheckboxSetting.svelte";
  import Group from "./Group.svelte";
  import SelectSetting from "../widget/SelectSetting.svelte";
import SliderSetting from "../widget/SliderSetting.svelte";

  export let allSettings: Settings;
  export let settings: Settings.Mqtt;
  export let original: Settings.Mqtt;

  let publishModes: Option<"json" | "text" | "both">[] = [];
  $: publishModes = [
    {
      label: `JSON at ${settings.prefix}${allSettings.general.name}/{state,stats,props}`,
      value: "json",
    },
    {
      label: `text at ${settings.prefix}${allSettings.general.name}/{state,stats,props}/[property]`,
      value: "text",
    },
    { label: "Both", value: "both" },
  ];
</script>

<Group title="MQTT" {settings} {original}>
  <CheckboxSetting
    label="Enabled"
    key="mqtt.enabled"
    bind:value={settings.enabled}
    bind:original={original.enabled}
  />
  <svelte:fragment slot="enabled">
    <TextSetting
      label="Server URL"
      key="mqtt.server"
      helpText="[mqtt://]mqtt-server-name[:port]"
      bind:value={settings.server}
      bind:original={original.server}
    />
    <TextSetting
      label="Topic prefix"
      key="mqtt.prefix"
      bind:value={settings.prefix}
      bind:original={original.prefix}
    />
    <TextSetting
      label="Username"
      key="mqtt.username"
      bind:value={settings.username}
      bind:original={original.username}
    />
    <TextSetting
      label="Password"
      key="mqtt.password"
      kind="password"
      bind:value={settings.password}
      bind:original={original.password}
    />
    <CheckboxSetting
      label="Publish updates"
      key="mqtt.publish_status"
      bind:value={settings.publish_status}
      bind:original={original.publish_status}
    />
    <SelectSetting
      label="Publish format"
      key="mqtt.publish_format"
      bind:value={settings.publish_format}
      bind:original={original.publish_format}
      options={publishModes}
      disabled={!settings.publish_status}
    />
    <SliderSetting
      label="Seconds between updates"
      key="mqtt.publish_interval"
      bind:value={settings.publish_interval}
      bind:original={original.publish_interval}
      min={5} max={900}
      />
    <CheckboxSetting
      label="HomeAssistant integration"
      key="mqtt.ha"
      bind:value={settings.ha}
      bind:original={original.ha}
    />
    <CheckboxSetting
      label="IOBroker integration"
      key="mqtt.iob"
      bind:value={settings.iob}
      bind:original={original.iob}
    />
  </svelte:fragment>
</Group>
