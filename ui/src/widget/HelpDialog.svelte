<script lang="ts">
  import { Modal } from "carbon-components-svelte";

  export let open: boolean = false;

  const primary = () => (open = false);
  const close = () => {};
</script>

<Modal
  passiveModal
  modalHeading="ArduMower Modem Help"
  on:click:button--primary={primary}
  bind:open
  on:close={close}
>
  <h3>About</h3>
  <p>You are viewing the settings interface of the ArduMower Modem.</p>
  <p>
    <a href="https://github.com/timotto/ardumower-modem" target="_blank"
      >ArduMower Modem</a
    > is an open source firmware for an ESP32 as a companion to the ArduMower Sunray
    App and firmware.
  </p>
  <p>
    <a href="https://www.ardumower.de/" target="_blank">ArduMower</a> is an open
    source smart lawn mower platform.
  </p>
  <p>
    <a href="https://github.com/Ardumower/Sunray" target="_blank">Sunray</a> is an
    open source firmware with RTK-GPS support for the ArduMower smart lawn mower
    platform.
  </p>
  <p>
    <a href="http://grauonline.de/am" target="_blank">Sunray App</a> is a Web and
    Smartphone App for a rich ArduMower Sunray user experience.
  </p>

  <h3>License</h3>
  <p>license text</p>
  <h3>Third party libraries</h3>
  <ul>
    <li>Arduino</li>
    <li>Arduino ESP32</li>
    <li>ArduinoJson</li>
    <li>AsyncTCP</li>
    <li>ESPAsyncWebServer</li>
    <li>NimBLE-Arduino</li>
    <li>MQTT</li>
  </ul>
</Modal>

<style>
  p {
    padding-top: 0.25em;
    padding-bottom: 0.25em;
  }
</style>
