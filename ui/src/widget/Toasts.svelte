<script lang="ts">
  import { Errors } from "../stores/error";
  import { SuccessfulSaves } from "../stores/success";
  import { Popover, ToastNotification } from "carbon-components-svelte";
  import { ErrorDetails, ServiceError } from "../service";
  import { TextService } from "../text"

  interface ToastContent {
    title: string;
    subtitle: string;
  }

  const errorToToast = (err: Error | ServiceError): ToastContent => {
    if (err instanceof ServiceError) {
      return fromServiceError(err);
    }

    return fromError(err);
  };

  const fromServiceError = (err: ServiceError): ToastContent => ({
    title: `Failed to ${err.action}`,
    subtitle: fromErrorDetails(err.action, err.details),
  });

  const fromErrorDetails = (action: string, d: ErrorDetails): string => {
    if (action === "save settings" && d.action === "invalid")
      return d.error + " is bad";

    if (action === "load settings" && d.action === "decode")
      return "Unable to understand the modem response";

    if (action === "get modem info" && d.action === "decode")
      return "Unable to understand the modem response";

    if (d.action) return d.action + " / " + d.error;

    return d.error;
  };

  const fromError = (err: Error): ToastContent => ({
    title: err.message,
    subtitle: "Please try again or reload the page",
  });

  let errs: ToastContent[] = [];
  $: errs = $Errors.filter((err) => !!err).map(errorToToast);
</script>

<Popover open align="bottom-right">
  {#each errs as error}
    <ToastNotification
      title={error.title}
      subtitle={error.subtitle}
      caption={new Date().toLocaleString()}
      timeout={5000}
    />
  {/each}
  {#each $SuccessfulSaves as success}
    <ToastNotification
      title={TextService.actionResultFor(success.action, 'success')}
      kind="success"
      caption={success.date.toLocaleString()}
      timeout={5000}
    />
  {/each}
</Popover>
