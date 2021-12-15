<script lang="ts">
  import { Button } from "carbon-components-svelte";
  import IconReload from "carbon-icons-svelte/lib/UpdateNow32";
  import { StatusStore } from "../stores/status";
  import { InfoService } from "../service";
  import ConnectionStatus from "./ConnectionStatus.svelte";

  function refresh() {
    InfoService.status();
  }
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
