<template>
  <div class="absolutePos" v-bind:style="{
        left: posX,
        top: posY,
        width: sizeX,
        height: sizeY
      }">
    <v-btn
      v-bind:color="color"
      v-bind:outlined="btnStateValue"
      v-on:click="toggleState()"
    >{{ text }}</v-btn>
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
  name: "guiElementButton",
  components: null,
  props: {
    guiElementStruct: {
      type: Object,
      required: true
    }
  },
  data: () => ({}),
  methods: {
    ...mapActions(["ws_send_wsEvent_dataNodeChange"]),
    toggleState: function() {
      let dataNode = [];
      dataNode.push(String(this.btnStateID));
      if (this.btnStateValue === true) {
        dataNode.push("0");
      } else {
        dataNode.push("1");
      }
      this.ws_send_wsEvent_dataNodeChange(dataNode);
    }
  },
  computed: {
    paramNodes: function() {
      return this.guiElementStruct.paramNodes;
    },
    dataNodes: function() {
      return this.guiElementStruct.dataNodes;
    },
    color: function() {
      if (this.paramNodes["colorEnum"].value !== null) {
        return JSON.parse(this.paramNodes["colorEnum"].value)[
          this.dataNodes["colorSelector"].value
        ];
      } else {
        return "";
      }
    },
    btnStateID: function() {
      return this.dataNodes["buttonState"].id;
    },
    btnStateValue: function() {
      return this.dataNodes["buttonState"].value;
    },
    text: function() {
      return this.dataNodes["text"].value;
    },
    posX: function() {
      return String(this.paramNodes["posX"].value+200) + "px";
    },
    posY: function() {
      return String(this.paramNodes["posY"].value+20) + "px";
    },
    sizeX: function() {
      return String(this.paramNodes["sizeX"].value) + "px";
    },
    sizeY: function() {
      return String(this.paramNodes["sizeY"].value) + "px";
    }
  },
  created: function() {
    //this.$vuetify.theme.dark = true;
  }
};
</script>
