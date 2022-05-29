<script lang="ts">
  import {
    ComposedModal,
    ModalHeader,
    ModalFooter,
    ModalBody,
    ProgressBar,
    FileUploaderButton,
  } from "carbon-components-svelte";
  import type { Readable } from "svelte/store";
  import { FirmwareUploader, FirmwareUploadStatus } from "./service";

  export let open: boolean = false;

  let ref: null | HTMLInputElement;

  let fileSize = 0;

  let uploader = new FirmwareUploader();

  function uploadChange(e) {
    if (!(ref && ref.files && ref.files.length > 0)) {
      uploader.file = null;
      return;
    }
    const file = ref.files[0];
    uploader.file = file;
    fileSize = file.size;

    if (file !== null) uploader.upload();
  }

  async function primary() {
    open = false;
  }

  function close() {
    if ($uploaderStatus === FirmwareUploadStatus.success)
      document.location.reload()
  }

  let uploaderStatus: Readable<FirmwareUploadStatus> = uploader.status;
  let uploaderProgress: Readable<number> = uploader.progress;
</script>

<ComposedModal on:click:button--primary={primary} bind:open on:close={close}>
  <ModalHeader title="Upload modem firmware" />
  <ModalBody hasForm={true}>
    <ProgressBar
      value={$uploaderProgress}
      max={100}
      disabled={$uploaderStatus < FirmwareUploadStatus.uploading}
    />
    <p>Select the firmware update file on your computer.</p>
    <p>
      You can download the latest firmware updates on the <a
        href="https://github.com/timotto/ardumower-modem/releases"
        target="_blank">GitHub Releases</a
      > page.
    </p>
    <FileUploaderButton
      bind:ref
      on:change={uploadChange}
      accept={[".bin"]}
      labelText="Select..."
    />
    {#if $uploaderStatus >= FirmwareUploadStatus.fileSelected}
      <p>Size: {fileSize} bytes</p>
    {/if}

    {#if $uploaderStatus === FirmwareUploadStatus.uploading}
      <p>Uploading the firmware update...</p>
    {/if}
    {#if $uploaderStatus === FirmwareUploadStatus.expectReboot}
      <p>The firmware has been uploaded successfully.</p>
      <p>Waiting for the modem to restart...</p>
    {/if}

    {#if $uploaderStatus === FirmwareUploadStatus.error}
      <p>The update failed!</p>
      <p>The error message is <i>{uploader.error}</i></p>
    {/if}

    {#if $uploaderStatus === FirmwareUploadStatus.success}
      <p>The firmware update has been installed successfully.</p>
    {/if}
  </ModalBody>
  <ModalFooter
    secondaryButtonText="Cancel"
    primaryButtonText="Close"
    primaryButtonDisabled={$uploaderStatus !== FirmwareUploadStatus.success}
  />
</ComposedModal>
