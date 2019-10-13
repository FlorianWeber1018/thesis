<template>
    <div v-if="pageStruct != null">
    {{pageStruct}}
    </div>
</template>
<script>
import { mapActions, mapState } from 'vuex'
export default{
    components: null,
    data: () => (
        {
            
        }),
    methods: {
    ...mapActions([
      "ws_send_wsEvent_pageChange"
    ])
        
  },
  computed:{
    ...mapState({pageStruct: state => state.pageStruct}),
    pageID: function(){
      return this.pageStruct.pageID;
    },
    pageTitle: function(){
      return this.pageStruct.title;
    },
    parentPageID: function(){
      return this.pageStruct.parentId;
    },
    childPages: function(){
      return this.pageStruct.pages;
    }

    },
    created: function() {
      this.$vuetify.theme.dark = true
    }
};
</script>