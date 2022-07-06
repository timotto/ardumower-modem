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
      label="Bluetooth pairings require a PIN code"
      key="ps4Controller.usePS4MAC"
      bind:value={settings.usePS4MAC}
      bind:original={original.usePS4MAC}
    />

    <TextSetting
      label="Use MAC address from PS4. (You can use the sixaxispairer tool to read the MAC address from the cotroller)"
      key="ps4Controller.ps4MAC"
      placeholder="PS4 Bluetooth MAC Address"
      disabled={!settings.usePS4MAC}
      bind:value={settings.ps4MAC}
      bind:original={original.ps4MAC}
    />
  </svelte:fragment>
</Group>
