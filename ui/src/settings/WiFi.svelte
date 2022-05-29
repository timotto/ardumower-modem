<script lang="ts">
  import type { Settings } from "../model";
  import type { Option } from "../model/ui";
  import SelectSetting from "../widget/SelectSetting.svelte";
  import TextSetting from "../widget/TextSetting.svelte";
  import Group from "./Group.svelte";

  export let settings: Settings.WiFi;
  export let original: Settings.WiFi;

  const wifiModes: Option<"sta" | "ap" | "off">[] = [
    { label: "WiFi Client", value: "sta" },
    { label: "Access Point", value: "ap" },
    { label: "Off / setup only", value: "off" },
  ];
</script>

<Group title="WiFi" {settings} {original}>
  <SelectSetting
    label="Mode"
    key="wifi.mode"
    bind:value={settings.mode}
    bind:original={original.mode}
    options={wifiModes}
  />
  {#if settings.mode === "sta"}
    <p>The modem connects to your WiFi network.</p>
    <TextSetting
      label="SSID"
      key="wifi.sta_ssid"
      helpText="Name of your WiFi network"
      bind:value={settings.sta_ssid}
      bind:original={original.sta_ssid}
    />
    <TextSetting
      label="PSK"
      key="wifi.sta_psk"
      helpText="Password for your WiFi network"
      kind="password"
      bind:value={settings.sta_psk}
      bind:original={original.sta_psk}
    />
  {:else if settings.mode === "ap"}
    <p>The modem creates a WiFi network you can connect to.</p>
    <TextSetting
      label="SSID"
      key="wifi.ap_ssid"
      helpText="Name of the WiFi network"
      bind:value={settings.ap_ssid}
      bind:original={original.ap_ssid}
    />
    <TextSetting
      label="PSK"
      key="wifi.ap_psk"
      helpText="Password for the WiFi network"
      kind="password"
      bind:value={settings.ap_psk}
      bind:original={original.ap_psk}
    />
  {:else}
    <p>
      The modem enables a WiFi Access Point for a few minutes after power on you
      can connect to.
    </p>
    <TextSetting
      label="SSID"
      key="wifi.ap_ssid"
      helpText="Name of the WiFi network"
      bind:value={settings.ap_ssid}
      bind:original={original.ap_ssid}
    />
    <TextSetting
      label="PSK"
      key="wifi.ap_psk"
      helpText="Password for the WiFi network"
      kind="password"
      bind:value={settings.ap_psk}
      bind:original={original.ap_psk}
    />
  {/if}
</Group>
