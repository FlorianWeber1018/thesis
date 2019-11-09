<template>
  <div
    class="absolutePos"
    v-bind:style="{
        left: posX,
        top: posY,
        width: sizeX,
        height: sizeY,
      }"
  >
    <v-chip v-bind:color="color" v-bind:text-color="textColor" label>{{text}}</v-chip>
  </div>
</template>
<style scoped>
.absolutePos {
  position: absolute;
}
</style>
<script>
import { mapActions } from "vuex";
export default {
  name: "guiElementLabel",
  components: null,
  props: {
    guiElementStruct: {
      type: Object,
      required: true
    }
  },
  data: function() {
    return {};
  },
  methods: {},
  computed: {
    paramNodes: function() {
      return this.guiElementStruct.paramNodes;
    },
    dataNodes: function() {
      return this.guiElementStruct.dataNodes;
    },
    color: function() {
      return this.paramNodes["color"].value;
    },
    textColor: function() {
      return this.paramNodes["textColor"].value;
    },
    text: function() {
      if (this.paramNodes["textEnum"].value !== null) {
        return JSON.parse(this.paramNodes["textEnum"].value)[
          this.dataNodes["textSelector"].value
        ];
      } else {
        return "";
      }
      return this.dataNodes["text"].value;
    },
    posX: function() {
      return String(this.paramNodes["posX"].value + 400) + "px";
    },
    posY: function() {
      return String(this.paramNodes["posY"].value + 20) + "px";
    },
    sizeX: function() {
      return String(this.paramNodes["sizeX"].value) + "px";
    },
    sizeY: function() {
      return String(this.paramNodes["sizeY"].value) + "px";
    }
  },
  created: function() {
    this.$vuetify.theme.dark = true;
  }
};
</script>
