<script lang="ts">
  import { Button, Modal } from "carbon-components-svelte";
  import IconTrash from "carbon-icons-svelte/lib/TrashCan16";
  import type { Settings } from "../model";
  import { BluetoothService } from "../service";
  import CheckboxSetting from "../widget/CheckboxSetting.svelte";
  import TextSetting from "../widget/TextSetting.svelte";
  import Group from "./Group.svelte";

  export let settings: Settings.Bluetooth;
  export let original: Settings.Bluetooth;

  export let ps4ControllerSettings: Settings.PS4Controller;

  let confirmClearPairing = false;
  const clearPairings = () => {
    confirmClearPairing = true;
  };
  let clearing = false;
  const confirmed = async () => {
    clearing = true;
    try {
      await BluetoothService.resetPairings();
      confirmClearPairing = false;
    } finally {
      clearing = false;
    }
  };

  const enableChange = (e) => {
    ps4ControllerSettings.enabled = false;
  }
</script>

<Group title="Bluetooth" {settings} {original}>
  <CheckboxSetting
    label="Control and monitor your ArduMower with Bluetooth"
    key="bluetooth.enabled"
    bind:value={settings.enabled}
    bind:original={original.enabled}
    on:change={enableChange}
  />
  <svelte:fragment slot="enabled">
    <CheckboxSetting
      label="Bluetooth pairings require a PIN code"
      key="bluetooth.pin_enabled"
      bind:value={settings.pin_enabled}
      bind:original={original.pin_enabled}
    />

    <TextSetting
      label="PIN"
      key="bluetooth.pin"
      placeholder="Bluetooth pairing PIN"
      kind="password"
      disabled={!settings.pin_enabled}
      bind:value={settings.pin}
      bind:original={original.pin}
    />

    <Button
      on:click={clearPairings}
      icon={IconTrash}
      kind="danger-tertiary"
      disabled={!settings.pin_enabled}>Delete all Bluetooth Pairings</Button
    >
  </svelte:fragment>
</Group>

<Modal
  danger
  bind:open={confirmClearPairing}
  primaryButtonDisabled={clearing}
  modalHeading="Delete all Bluetooth Pairings"
  primaryButtonText="Delete"
  secondaryButtonText="Cancel"
  on:click:button--secondary={() => (confirmClearPairing = false)}
  on:open
  on:close
  on:submit={confirmed}
>
  <p>Previously connected Bluetooth devices will be asked for a PIN again.</p>
</Modal>
