<script lang="ts">
  import ConnectionSignal16 from "carbon-icons-svelte/lib/ConnectionSignal16";
  import ConnectionSignalOff16 from "carbon-icons-svelte/lib/ConnectionSignalOff16";
  import Time16 from "carbon-icons-svelte/lib/Time16";
  import Tools16 from "carbon-icons-svelte/lib/Tools16";
  import type { Status } from "../model";

  export let status: Status;

  function rtt(s: Status) {
    return `${(s.relay_rtt * 1000).toFixed(0)} ms`;
  }

  function connectedSince(s: Status): string {
    const now = new Date();
    const t = now.getTime() - s.uptime + s.relay_connect_time;
    const d = new Date(t);

    const dt = now.getTime() - d.getTime();

    if (dt <= 24 * 3600 * 1000) return d.toLocaleTimeString();

    return d.toLocaleString();
  }
</script>

<main>
  {#if status && status.relay_connected}
    <p>
      <ConnectionSignal16 />
      The modem is connected to the relay server since {connectedSince(status)}.
    </p>
    <p>
      <Time16 />
      Ping: {rtt(status)}
    </p>
    <p>
      <Tools16 />
      Re-connects: {status.relay_connect_count - 1}
    </p>
  {:else}
    <p>
      <ConnectionSignalOff16 />
      The modem is not connected to the relay server.
    </p>
  {/if}
</main>

<style>
  p {
    padding-left: 1em;
    padding-right: 1em;
    flex: 1;
  }
</style>
