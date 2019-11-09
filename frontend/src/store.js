import Vue from "vue";
import Vuex from "vuex";
import router from "./router";
Vue.use(Vuex);

var store = new Vuex.Store({
  state: {
    socket: {
      isConnected: false,
      message: "",
      reconnectError: false
    },
    pageID: 0,
    authenticated: false,
    pageStruct: null
  },
  mutations: {
    SOCKET_ONOPEN(state, event) {
      Vue.prototype.$socket = event.currentTarget;
      state.socket.isConnected = true;
      if (router.currentRoute.path != "/") {
        router.push("/");
      }
    },
    SOCKET_ONCLOSE(state, event) {
      state.socket.isConnected = false;
      state.authenticated = false;
    },
    SOCKET_ONERROR(state, event) {
      console.error(state, eval);
    },
    SOCKET_RECONNECT(state, event) {
      router.push("/");
      state.authenticated = false;
    },
    // default handler called for all methods
    SOCKET_ONMESSAGE(state, message) {
      let msgArray = message.data.split(";");
      if (msgArray.length > 0) {
        let event = msgArray.shift();
        let payload = msgArray;
        switch (event) {
          case "0": {
            //wsEvent_invalid this shoul be never received by client --> no hadler
          }
          break;
        case "1": {
          //wsEvent_dataNodeChange
          if (payload.length == 2) {
            let key = parseInt(payload[0], 10);
            let value = payload[1];
            for (const [guiElementKey] of Object.entries(
                state.pageStruct.guiElements
              )) {
              let found = false;
              for (const [DataNodeKey] of Object.entries(
                  state.pageStruct.guiElements[guiElementKey].dataNodes
                )) {
                if (
                  state.pageStruct.guiElements[guiElementKey].dataNodes[
                    DataNodeKey
                  ].id === key
                ) {
                  const type =
                    state.pageStruct.guiElements[guiElementKey].dataNodes[
                      DataNodeKey
                    ].type;

                  if (type == "Bool") {
                    state.pageStruct.guiElements[guiElementKey].dataNodes[
                      DataNodeKey
                    ].value = value == "1";
                  } else if (type == "String") {
                    state.pageStruct.guiElements[guiElementKey].dataNodes[
                      DataNodeKey
                    ].value = value;
                  } else if (type == "Double" || type == "Float") {
                    state.pageStruct.guiElements[guiElementKey].dataNodes[
                      DataNodeKey
                    ].value = parseFloat(value);
                  } else {
                    state.pageStruct.guiElements[guiElementKey].dataNodes[
                      DataNodeKey
                    ].value = parseInt(value);
                  }
                  found = true;
                }
                if (found) {
                  break;
                }
              }
              if (found) {
                break;
              }
            }
          }
        }
        break;
        case "2": {
          //wsEvent_paramNodeChange
          if (payload.length == 2) {
            let key = parseInt(payload[0], 10);
            let value = payload[1];
            for (const [guiElementKey] of Object.entries(
                state.pageStruct.guiElements
              )) {
              let found = false;
              for (const [paramKey] of Object.entries(
                  state.pageStruct.guiElements[guiElementKey].paramNodes
                )) {
                if (
                  state.pageStruct.guiElements[guiElementKey].paramNodes[
                    paramKey
                  ].id === key
                ) {
                  const type =
                    state.pageStruct.guiElements[guiElementKey].paramNodes[
                      paramKey
                    ].type;

                  if (type == "Bool") {
                    state.pageStruct.guiElements[guiElementKey].paramNodes[
                      paramKey
                    ].value = value == "1";
                  } else if (type == "String") {
                    state.pageStruct.guiElements[guiElementKey].paramNodes[
                      paramKey
                    ].value = value;
                  } else if (type == "Double" || type == "Float") {
                    state.pageStruct.guiElements[guiElementKey].paramNodes[
                      paramKey
                    ].value = parseFloat(value);
                  } else {
                    state.pageStruct.guiElements[guiElementKey].paramNodes[
                      paramKey
                    ].value = parseInt(value);
                  }
                  found = true;
                }
                if (found) {
                  break;
                }
              }
              if (found) {
                break;
              }
            }
          }
        }
        break;
        case "3": {
          //wsEvent_pageChange
          if (payload.length == 1) {
            let newPageID = parseInt(payload[0], 10);
            if (!isNaN(newPageID)) {
              state.pageID = newPageID;
              store.dispatch("ws_send_wsEvent_structure", newPageID);
            }
          }
        }
        break;
        case "4": {
          //wsEvent_structure
          if (payload.length == 1) {
            let pageObj = JSON.parse(payload[0]);
            if (typeof pageObj === "object" && pageObj !== null) {
              state.pageStruct = pageObj;
              store.dispatch("wsEvent_reqSendParams");
              store.dispatch("wsEvent_reqSendDataNodes");
              //console.dir(JSON.stringify(state));
              if (router.currentRoute.path != "/WebVisu") {
                router.push("WebVisu");
              }
            }
          }
        }
        break;
        case "5": {
          //wsEvent_authentification
          if (payload.length == 0) {
            state.authenticated = true;
            store.dispatch("ws_send_wsEvent_pageChange", 1);
          }
        }
        break;
        case "6": {
          //wsEvent_reqSendParams this should be never received by client --> no hadler
        }
        break;
        case "7": {
          //wsEvent_reqSendDataNodes this should be never received by client --> no hadler
        }
        break;
        }
      }
    }
  },
  actions: {
    wsEvent_reqSendParams: function (context) {
      context.dispatch("ws_sendEvent", "6");
    },
    wsEvent_reqSendDataNodes: function (context) {
      context.dispatch("ws_sendEvent", "7");
    },
    ws_send_wsEvent_structure: function (context, page) {
      let sendstr = "4;";
      sendstr += page;
      context.dispatch("ws_sendEvent", sendstr);
    },
    ws_send_wsEvent_pageChange: function (context, newPage) {
      let sendstr = "3;";
      sendstr += newPage;
      context.dispatch("ws_sendEvent", sendstr);
    },
    ws_send_wsEvent_authentification: function (context, credentials) {
      let sendstr = "5;";
      sendstr += credentials.shift();
      sendstr += ";";
      sendstr += credentials.shift();
      context.dispatch("ws_sendEvent", sendstr);
    },
    ws_send_wsEvent_dataNodeChange: function (context, dataNode) {
      let sendstr = "1;";
      sendstr += dataNode.shift();
      sendstr += ";";
      sendstr += dataNode.shift();
      context.dispatch("ws_sendEvent", sendstr);
      //console.dir(JSON.stringify(context.state));
    },
    ws_sendEvent: function (context, message) {
      if (context.state.socket.isConnected) {
        Vue.prototype.$socket.send(message);
      }
    }
  }
});
export default store;