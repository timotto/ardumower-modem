<script lang="ts">
  import { Button, Modal } from "carbon-components-svelte";
  import IconTrash from "carbon-icons-svelte/lib/TrashCan16";
  import type { Settings } from "../model";
  import { BluetoothService } from "../service";
  import CheckboxSetting from "../widget/CheckboxSetting.svelte";
  import TextSetting from "../widget/TextSetting.svelte";
  import Group from "./Group.svelte";

  export let settings: Settings.PS4Controller;
  export let original: Settings.PS4Controller;
</script>

<Group title="PS4 Controller" {settings} {original}>
  <CheckboxSetting
    label="Use PS4 controller to control the mower"
    key="ps4Controller.enabled"
    bind:value={settings.enabled}
    bind:original={original.enabled}
  />
  <svelte:fragment slot="enabled">
    <CheckboxSetting
      label="Use MAC address from PS4. (You can use the sixaxispairer tool to read the MAC address from the cotroller)"
      key="ps4Controller.use_ps4_mac"
      bind:value={settings.use_ps4_mac}
      bind:original={original.use_ps4_mac}
    />

    <TextSetting
      label=""
      key="ps4Controller.ps4MAC"
      placeholder="PS4 Bluetooth MAC Address"
      disabled={!settings.use_ps4_mac}
      bind:value={settings.ps4_mac}
      bind:original={original.ps4_mac}
    />
  </svelte:fragment>
</Group>
