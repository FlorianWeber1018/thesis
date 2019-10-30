<template>
  <v-form v-model="valid">
    <v-container>
      <v-row align="center">
        <v-col cols="12" md="4">
          <v-text-field
            v-model="userName"
            :counter="10"
            label="User Name"
            required
          ></v-text-field>
        </v-col>

        <v-col cols="12" md="4">
          <v-text-field
            v-model="password"
            :counter="10"
            label="Password"
            required
            type="password"
          ></v-text-field>
        </v-col>

        <v-col cols="12" md="4">
          <v-btn
            class="ma-2"
            block
            color="indigo"
            v-on:click="ws_send_wsEvent_authentification([userName, password])"
          >
            LOGIN!
          </v-btn>
        </v-col>
      </v-row>
    </v-container>
  </v-form>
</template>
<script>
import { mapActions } from "vuex";

export default {
  components: null,
  data: () => ({
    valid: true,
    userName: "user",
    password: "testPW"
  }),
  methods: {
    ...mapActions(["ws_send_wsEvent_authentification"])
  },
  created: function() {}
};
</script>
