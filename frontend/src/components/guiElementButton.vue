<template>
<div>

  <v-btn fixed
    v-bind:color="color"
    v-bind:style="{
      left: posX,
      top:  posY,
      maxWidth: maxSizeX,
      maxHeight: maxSizeY
    }"
    v-bind:outlined="btnStateValue"
    v-on:click="toggleState()"
  >{{text}}</v-btn>
  

</div>
</template>
<script>
import { mapActions} from 'vuex'
export default{
  name: "guiElement",
  components: null,
  props: {
    guiElementStruct: {
      type: Object,
      required: true
    }
  },
  data: () => (
    {
            
    }),
  methods: {
    ...mapActions([
      "ws_send_wsEvent_dataNodeChange"
    ]),
    toggleState: function(){
      let dataNode = [];
      dataNode.push(String(this.btnStateID));
      if(this.btnStateValue === true){
        dataNode.push("0");
      }else{
        dataNode.push("1");
      }
      console.log(this.btnStateValue)
      this.ws_send_wsEvent_dataNodeChange(dataNode);
    }
  },
  computed:{
    params: function(){
      return this.guiElementStruct.params;
    },
    dataNodes: function(){
      return this.guiElementStruct.dataNodes;
    },
    color: function(){
      return JSON.parse(this.params["colorEnum"].value)[this.dataNodes["colorSelector"].value];
    },
    btnStateID: function(){
      return this.dataNodes["buttonState"].id;
    },
    btnStateValue: function(){
      return this.dataNodes["buttonState"].value;
    },
    text: function(){
      return this.dataNodes["text"].value;
    },
    posX: function(){
      return (String(this.params["posX"].value) + "%");
    },
    posY: function(){
      return (String(this.params["posY"].value + 60) + "px");
    },
    maxSizeX: function(){
      return (String(this.params["maxSizeX"].value) + "%");
    },
    maxSizeY: function(){
      return (String(this.params["maxSizeY"].value) + "%");
    }
  },
  created: function() {
    this.$vuetify.theme.dark = true;
  }
};
</script>