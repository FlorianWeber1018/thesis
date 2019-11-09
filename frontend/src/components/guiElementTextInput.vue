<template>
  <div
    class="absolutePos"
    v-bind:style="{
        left: posX,
        top: posY,
        width: sizeX,
        height: sizeY
      }"
  >
    <v-text-field
      v-bind:hint="name"
      v-bind:label="text"
      v-bind:loading="highlight"
      v-bind:color="color"
      v-model="tempText"
      v-on:blur="blured"
      v-on:keydown.enter="enter"
      v-on:keydown.escape="esc"
      v-on:focus="focused"
      filled
      outlined
      persistent-hint
    ></v-text-field>
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
  name: "guiElementTextInput",
  components: null,
  props: {
    guiElementStruct: {
      type: Object,
      required: true
    }
  },
  data: function() {
    return {
      tempText: "",
      highlight: false
    };
  },
  methods: {
    ...mapActions(["ws_send_wsEvent_dataNodeChange"]),
    sendNewText: function(newText) {
      let dataNode = [];
      dataNode.push(String(this.textID));
      dataNode.push(newText);
      this.ws_send_wsEvent_dataNodeChange(dataNode);
    },      
    sendTempText: function() {
      this.sendNewText(this.tempText);
      event.target.blur();
    },
    esc: function() {
      event.target.blur();
    },
    enter: function() {
      this.sendTempText();
      event.target.blur();
    },
    blured: function() {
      this.tempText = "";
      this.highlight = false;
    },
    focused: function() {
      this.highlight = true;
    }
  },
  computed: {
    paramNodes: function() {
      return this.guiElementStruct.paramNodes;
    },
    dataNodes: function() {
      return this.guiElementStruct.dataNodes;
    },
    name: function() {
      return this.guiElementStruct.name;
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
    textID: function() {
      return this.dataNodes["text"].id;
    },
    text: function() {
      return this.dataNodes["text"].value;
    },
    posX: function() {
      return String(this.paramNodes["posX"].value + 20) + "px";
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
