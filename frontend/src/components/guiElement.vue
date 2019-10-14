<template>

  <div v-if="myStruct != null">
    <!--{{myStruct}}-->
    <guiElementButton v-if="myStruct.type === 'button'" v-bind:guiElementStruct="myStruct" />

  </div>
</template>
<script>
import guiElementButton from "../components/guiElementButton"
import { mapActions, mapState } from 'vuex'
export default{
  name: "guiElement",
  components: {
    guiElementButton
  },
  props: {
    id: {
      type: Number,
      required: true
    }
  },
  data: () => (
    {
            
    }),
  methods: {
    ...mapActions([
      
    ])  
  },
  computed:{
    ...mapState({guiElements: state => state.pageStruct.guiElements}),
    myStruct: function(){
      return this.guiElements[String(this.id)];
    }
  },
  created: function() {
    this.$vuetify.theme.dark = true
  }
};
</script>