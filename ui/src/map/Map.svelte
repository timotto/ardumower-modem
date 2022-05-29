<script lang="ts">
  import {
    Button,
    ButtonSet,
    Checkbox,
    ComboBox,
    Slider,
    Row,
    Column,
    Grid,
  } from "carbon-components-svelte";
  import Canvas from "./Canvas.svelte";
  import Exclusion from "./Exclusion.svelte";
  import { pointsToEdges } from "./geometry";
  import type { Point, Edge } from "./model";
  import Perimeter from "./Perimeter.svelte";
  import { MapStore } from "./service";

  interface EditItem {
    id: string;
    text: string;
  }

  let edit = false;
  let selectedIndex = -1;
  let editItemId: null | string = null;
  let editItems: EditItem[] = [];

  let editPoint = false;
  let editEdge = false;

  const pointsToEditItem =
    (idPrefix: string, textPrefix: string) => (p: Point, index: number) => ({
      id: idPrefix + index,
      text: textPrefix + index,
    });

  const edgesToEditItem =
    (idPrefix: string, textPrefix: string) =>
    (e: Edge, index: number): EditItem => ({
      id: idPrefix + index,
      text: textPrefix + index,
    });

  $: editItems = [
    $MapStore.map.perimeter.points.map(
      pointsToEditItem("map-0-perimeter-point-", "Perimeter point #")
    ),
    pointsToEdges($MapStore.map.perimeter.points).map(
      edgesToEditItem("map-0-perimeter-edge-", "Perimeter edge #")
    ),
    ...$MapStore.map.exclusions.map((e, i) => [
      ...e.points.map(
        pointsToEditItem(
          `map-0-exclusion-${i}-point-`,
          `Exclution #${i} point #`
        )
      ),
      ...pointsToEdges(e.points).map(
        edgesToEditItem(`map-0-exclusion-${i}-edge-`, `Exclusion #${i} edge #`)
      ),
    ]),
  ].reduce((a, b) => [...a, ...b], []);

  function shouldFilterItem(item, value) {
    if (!value) return true;
    return item.text.toLowerCase().includes(value.toLowerCase());
  }

  let uiEditId: null | string = null;

  function updateButtonAvailability() {
    editPoint = edit && !!editItemId && editItemId.indexOf("-point-") !== -1;
    editEdge = edit && !!editItemId && editItemId.indexOf("-edge-") !== -1;
  }

  function selectEditItem(e: any) {
    uiEditId = editItemId = e["detail"]["selectedId"];
    updateButtonAvailability();
  }

  function clearEditItem() {
    editItemId = null;
  }

  function omg(item: null | string) {
    if (item == uiEditId) return;

    if (item === null) return;

    let editItemIndex = editItems
      .map((item, index) => (item.id === editItemId ? index : -1))
      .reduce((a, b) => (a > b ? a : b), -1);

    selectedIndex = editItemIndex;
    updateButtonAvailability();
  }

  $: omg(editItemId);
</script>

<main>
  <Grid>
    <Row>
      <Column sm={1} md={1} lg={1}>
        <Checkbox bind:checked={edit} labelText="Editor" />
      </Column>
      <Column>
        <ComboBox
          disabled={!edit}
          placeholder="Select item to edit"
          items={editItems}
          bind:selectedIndex
          on:select={selectEditItem}
          on:clear={clearEditItem}
          {shouldFilterItem}
        />
      </Column>
      <Column>
        <!-- <ButtonSet> -->
        <Button kind="tertiary" size="small" disabled={!editPoint}
          >Duplicate</Button
        >
        <Button kind="tertiary" size="small" disabled={!editEdge}>Split</Button>
        <Button kind="tertiary" size="small" disabled={!editEdge}>Cut</Button>
        <Button
          kind="danger"
          size="small"
          disabled={!edit || selectedIndex === -1}>Delete</Button
        >
        <!-- </ButtonSet> -->
      </Column>
    </Row>
  </Grid>
  <Canvas>
    <Perimeter
      value={$MapStore.map.perimeter}
      perimiterId="map-0-perimeter"
      {edit}
      bind:editItemId
    />
    {#each $MapStore.map.exclusions as exclusion, index}
      <Exclusion
        value={exclusion}
        exclusionId={"map-0-exclusion-" + index}
        {edit}
        bind:editItemId
      />
    {/each}
  </Canvas>
</main>
