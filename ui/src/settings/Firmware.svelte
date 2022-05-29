<script lang="ts">
  import {
    Button,
    StructuredList,
    StructuredListBody,
    StructuredListRow,
    StructuredListCell,
  } from "carbon-components-svelte";
  import IconUpload from "carbon-icons-svelte/lib/CloudUpload16";
  import Group from "./Group.svelte";
  import {InfoStore} from "../stores/info"
  import FirmwareUpload from '../firmware/FirmwareUpload.svelte'

  let dialogOpen = false;

  function upload() {
    dialogOpen = true;
  }
</script>

<FirmwareUpload bind:open={dialogOpen} />
<Group title="Firmware" open={true}>
  {#if $InfoStore}
    <StructuredList>
      <StructuredListBody>
        <StructuredListRow>
          <StructuredListCell>Tag</StructuredListCell>
          <StructuredListCell>{$InfoStore.git_tag}</StructuredListCell>
        </StructuredListRow>
        <StructuredListRow>
          <StructuredListCell>Timestamp</StructuredListCell>
          <StructuredListCell>{$InfoStore.git_time}</StructuredListCell>
        </StructuredListRow>
        <StructuredListRow>
          <StructuredListCell>Source Version</StructuredListCell>
          <StructuredListCell>{$InfoStore.git_hash}</StructuredListCell>
        </StructuredListRow>
      </StructuredListBody>
    </StructuredList>
  {/if}
  <Button on:click={upload} icon={IconUpload}>Upload modem firmware</Button>
</Group>
