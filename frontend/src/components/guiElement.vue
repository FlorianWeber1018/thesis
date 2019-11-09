<template>
  <div v-if="myStruct != null">
    <!--{{myStruct}}-->
    <guiElementButton v-if="myStruct.type === 'button'" v-bind:guiElementStruct="myStruct" />
    <guiElementTextInput v-if="myStruct.type === 'textInput'" v-bind:guiElementStruct="myStruct" />
    <guiElementLabel v-if="myStruct.type === 'label'" v-bind:guiElementStruct="myStruct" />
  </div>
</template>
<script>
import guiElementButton from "../components/guiElementButton";
import guiElementTextInput from "../components/guiElementTextInput";
import guiElementLabel from "../components/guiElementLabel";
import { mapActions, mapState } from "vuex";
export default {
  name: "guiElement",
  components: {
    guiElementButton,
    guiElementTextInput,
    guiElementLabel
  },
  props: {
    id: {
      type: Number,
      required: true
    }
  },
  data: () => ({}),
  methods: {
    ...mapActions([])
  },
  computed: {
    ...mapState({ guiElements: state => state.pageStruct.guiElements }),
    myStruct: function() {
      return this.guiElements[String(this.id)];
    }
  },
  created: function() {
    this.$vuetify.theme.dark = true;
  }
};
</script>
