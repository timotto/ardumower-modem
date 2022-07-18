<script lang="ts">
  import "carbon-components-svelte/css/g10.css";
  import {
    Content,
    Button,
    Header,
    HeaderUtilities,
    Loading,
    SkipToContent,
  } from "carbon-components-svelte";
  import IconHelp from "carbon-icons-svelte/lib/Help16";

  import SettingsRoute from "./routes/Settings.svelte";
  import { Busy } from "./stores/busy";
  import SaveDiscard from "./widget/SaveDiscard.svelte";
  import Toasts from "./widget/Toasts.svelte";
  import HelpDialog from "./widget/HelpDialog.svelte";

  let helpOpen = false;

  const help = () => (helpOpen = true);
</script>

{#if $Busy}
  <Loading />
{/if}
<Header company="ArduMower" platformName="Modem">
  <div slot="skip-to-content">
    <SkipToContent />
  </div>

  <HeaderUtilities>
    <SaveDiscard />
    <Button
      on:click={help}
      kind="tertiary"
      icon={IconHelp}
      iconDescription="Help"
    />
    <Toasts />
  </HeaderUtilities>
</Header>

<Content>
  <HelpDialog bind:open={helpOpen} />
  <SettingsRoute />
</Content>

<style>
  :global(.bx--toggle-input__label .bx--toggle__switch) {
    margin-top: 0.5rem !important;
  }
</style>