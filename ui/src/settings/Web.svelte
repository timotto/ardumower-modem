<script lang="ts">
  import TextSetting from "../widget/TextSetting.svelte";
  import CheckboxSetting from "../widget/CheckboxSetting.svelte";
  import Group from "./Group.svelte";
  import type { Settings } from "../model";

  export let settings: Settings.Web;
  export let original: Settings.Web;
</script>

<Group title="Web interface" {settings} {original} enabledKey="protected">
  <CheckboxSetting
    label="Protect this web interface"
    key="web.protected"
    bind:value={settings.protected}
    bind:original={original.protected}
  />

  <svelte:fragment slot="enabled">
    <TextSetting
      label="Username"
      key="web.username"
      bind:value={settings.username}
      bind:original={original.username}
    />
    <TextSetting
      label="Password"
      key="web.password"
      kind="password"
      bind:value={settings.password}
      bind:original={original.password}
    />
  </svelte:fragment>
</Group>
