import Vue from "vue";
import Router from "vue-router";
import loginPage from "./views/loginPage.vue";
import webVisuPage from "./views/webVisuPage.vue";
Vue.use(Router);

var router = new Router({
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
      component: webVisuPage
    }
  ]
});
export default router;
