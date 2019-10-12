import Vue from "vue";
import Router from "vue-router";
import loginPage from "./views/loginPage.vue";
import webVisuPages from "./views/webVisuPages.vue";
Vue.use(Router);

export default new Router({
  mode: "history",
  base: process.env.BASE_URL,
  routes: [
    {
      path: "/",
      name: "Login Page",
      component: loginPage
    },
    {
      path: "/webVisu",
      name: "WebVisu",
      component: webVisuPages
    }
  ]
});
