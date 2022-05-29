<script lang="ts">
  import type { Settings } from "../model";
  import CheckboxSetting from "../widget/CheckboxSetting.svelte";
  import TextSetting from "../widget/TextSetting.svelte";
  import Group from "./Group.svelte";

  export let settings: Settings.Prometheus
  export let original: Settings.Prometheus
</script>

<Group title="Prometheus" settings={settings} {original}>
  <CheckboxSetting 
  label="Enabled" 
  key="prometheus.enabled"
  bind:value={settings.enabled} 
  bind:original={original.enabled} 
  />
</Group>
