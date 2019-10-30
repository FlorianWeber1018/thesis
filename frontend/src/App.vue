<template>
  <v-app id="inspire" dark>
    <v-navigation-drawer v-if="pageStruct != null" v-model="drawer" app clipped>
      <v-list dense>
        <v-list-item
          v-if="parentPageID != null"
          v-on:click="ws_send_wsEvent_pageChange(parentPageID)"
        >
          <v-list-item-action>
            <v-icon>navigate_before</v-icon>
          </v-list-item-action>
          <v-list-item-content>
            <v-list-item-title>Zurück</v-list-item-title>
          </v-list-item-content>
        </v-list-item>
        <v-list-item
          v-for="childPage in childPages"
          v-bind:key="'navButtonTo:' + childPage.id"
          v-on:click="ws_send_wsEvent_pageChange(childPage.id)"
        >
          <v-list-item-action>
            <v-icon>navigate_next</v-icon>
          </v-list-item-action>
          <v-list-item-content>
            <v-list-item-title>{{ childPage.title }} (ID: {{ childPage.id }})</v-list-item-title>
          </v-list-item-content>
        </v-list-item>
      </v-list>
    </v-navigation-drawer>

    <v-app-bar v-if="pageStruct != null" app clipped-left>
      <v-app-bar-nav-icon @click.stop="drawer = !drawer"></v-app-bar-nav-icon>
      <v-toolbar-title>{{ pageTitle }} (ID: {{ pageID }})</v-toolbar-title>
    </v-app-bar>

    <v-content>
      <v-container class="fill-height">
        <router-view />
      </v-container>
    </v-content>
  </v-app>
</template>

<script>
import { mapActions, mapState } from "vuex";
export default {
  name: "App",
  components: {},
  data: () => ({
    drawer: false
  }),
  methods: {
    ...mapActions(["ws_send_wsEvent_pageChange"])
  },
  computed: {
    ...mapState({ pageStruct: state => state.pageStruct }),
    pageID: function() {
      return this.pageStruct.pageID;
    },
    pageTitle: function() {
      return this.pageStruct.title;
    },
    parentPageID: function() {
      return this.pageStruct.parentId;
    },
    childPages: function() {
      return this.pageStruct.subPages;
    }
  },
  created: function() {
    this.$vuetify.theme.dark = true;
  }
};
</script>
