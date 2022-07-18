<script lang="ts">
  import { Button, Modal } from "carbon-components-svelte";
  import IconTrash from "carbon-icons-svelte/lib/TrashCan16";
  import type { Settings } from "../model";
  import { BluetoothService } from "../service";

  export let bluetoothSettings: Settings.Bluetooth;
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
</script>

<p>
  <Button
    on:click={clearPairings}
    icon={IconTrash}
    kind="danger-tertiary"
    disabled={!bluetoothSettings.pin_enabled && !ps4ControllerSettings.enabled}>Delete all Bluetooth Pairings</Button
  >
</p>

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
