<script lang="ts">
  import { Button } from "carbon-components-svelte";
  import IconReload from "carbon-icons-svelte/lib/UpdateNow32";
  import { StatusStore } from "../stores/status";
  import { InfoService } from "../service";
  import ConnectionStatus from "./ConnectionStatus.svelte";
  import { onDestroy } from "svelte";

  function refresh() {
    InfoService.status();
  }

  function refreshFast() {
    if ($StatusStore.relay_connected) return;
    refresh();
  }

  let pollerTimerSlowId = setInterval(refresh, 20000);
  let pollerTimerFastId = setInterval(refreshFast, 3000);

  onDestroy(() => {
    clearInterval(pollerTimerSlowId);
    clearInterval(pollerTimerFastId);
  });
</script>

<main>
  <div class="details">
    <ConnectionStatus status={$StatusStore} />
  </div>
  <div class="action">
    <Button
      on:click={refresh}
      iconDescription="Check relay connection status"
      kind="ghost"
      icon={IconReload}
      hasIconOnly={true}
    />
  </div>
</main>

<style>
  main {
    display: flex;
    flex-direction: row;
    padding-top: 0.5em;
  }
  .details {
    flex: 1;
  }
</style>
