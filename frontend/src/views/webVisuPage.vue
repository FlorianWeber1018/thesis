<template>
  <div v-if="pageStruct != null">
    <guiElement v-for="guiElementObj in pageStruct.guiElements" :key="'gE_' + guiElementObj.id" :id="guiElementObj.id" /> 
  </div>
</template>
<script>
import { mapActions, mapState } from 'vuex'
import guiElement from "../components/guiElement"
export default{
    components: {
      guiElement
    },
    data: () => (
        {
            
        }),
    methods: {
    ...mapActions([
      
    ])
        
  },
  computed:{
    ...mapState({pageStruct: state => state.pageStruct})
  },
  created: function() {
    this.$vuetify.theme.dark = true
  }
};
</script>