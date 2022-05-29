<script lang="ts">
  import { Button, Modal } from "carbon-components-svelte";
  import IconReload from "carbon-icons-svelte/lib/UpdateNow32";

  import { BackendSettings } from "../stores/backend";
  import { Error } from "../stores/error";

  let open: boolean = false;
  $: open = !$BackendSettings && $Error && $Error.message === "load settings";

  const reload = () => {
    document.location.reload()
  };
</script>

{#if open}
  <div class="container">
    <div class="content">
      <p>Unable to load Modem settings.</p>
      <Button
        on:click={reload}
        kind="primary"
        size="xl"
        expressive={true}
        isSelected={true}
        icon={IconReload}
        iconDescription="Try again">Try again</Button
      >
      <p>
        If this error persists you may have to restart the Modem by power
        cycling your ArduMower.
      </p>
    </div>
  </div>
{/if}

<style>
  p {
    padding: 1em;
  }

  .container {
    display: flex;
    justify-content: center;
    align-items: center;
  }

  .content {
    display: flex;
    flex-direction: column;
    align-items: center;
  }
</style>
