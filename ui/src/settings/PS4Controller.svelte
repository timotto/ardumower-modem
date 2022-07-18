<script lang="ts">
  import { Button, Modal } from "carbon-components-svelte";
  import IconTrash from "carbon-icons-svelte/lib/TrashCan16";
  import type { ChangeEventValue, Info, Settings } from "../model";
  import type { Option } from "../model/ui";
  import { BluetoothService } from "../service";
  import CheckboxSetting from "../widget/CheckboxSetting.svelte";
  import SelectSetting from "../widget/SelectSetting.svelte";
  import TextSetting from "../widget/TextSetting.svelte";
import BluetoothClean from "./BluetoothClean.svelte";
  import Group from "./Group.svelte";

  export let settings: Settings.PS4Controller;
  export let original: Settings.PS4Controller;

  export let bluetoothSettings: Settings.Bluetooth;

  export let info: Info;

  const pairingModes: Option<"ps4_mac" | "esp_mac">[] = [
    { label: "Use MAC address from PS4 (Controller can still be used with the PS4)", value: "ps4_mac" },
    { label: "Use ESP32 BT MAC address (The controller must be paired again for use with the PS4)", value: "esp_mac" },
  ];

  let use_ps4_mac = original.use_ps4_mac ? "ps4_mac" : "esp_mac";
  let use_ps4_mac_original = use_ps4_mac;
  let bt_mac = info.bt_mac;

  const usePS4MacChange = (ev: ChangeEventValue) => {
    console.log("usePS4MacChange", ev)
    settings.use_ps4_mac = ev.value == "ps4_mac"
  }
</script>

<Group title="PS4 Controller" {settings} {original}>
  <CheckboxSetting
    label="Use PS4 controller to control the mower"
    key="ps4Controller.enabled"
    bind:value={settings.enabled}
    bind:original={original.enabled}
  />

  <svelte:fragment slot="enabled">
    <SelectSetting
      label="Pairing Mode (You can use the sixaxispairer tool to read the PS4 MAC address from the cotroller or overwrite it with ESP32 BT MAC)"
      key="ps4Controller.use_ps4_mac"
      bind:value={use_ps4_mac}
      bind:original={use_ps4_mac_original}
      options={pairingModes}
      on:change={(e) => usePS4MacChange(e.detail)}
    />

    {#if settings.use_ps4_mac}
      <TextSetting
        label="PS4 BT MAC address (When switching on the Controller, the PS4 should be far enough away)"
        key="ps4Controller.ps4MAC"
        placeholder="PS4 Bluetooth MAC Address"
        bind:value={settings.ps4_mac}
        bind:original={original.ps4_mac}
      />
    {:else}
      <TextSetting
        label="ESP32 BT MAC address (Configure PS4 control with these addresses. You can use the Sixaxispairer tool to do this)"
        key="ps4Controller.ps4MAC"
        readonly={true}
        bind:value={bt_mac}
        bind:original={bt_mac}
      />
    {/if}

    <BluetoothClean
      bind:bluetoothSettings={bluetoothSettings}
      bind:ps4ControllerSettings={settings}
    />
  </svelte:fragment>
</Group>
