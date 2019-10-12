import Vue from "vue";
import Vuex from "vuex";

Vue.use(Vuex);

var store = new Vuex.Store({
  state: {
    socket: {
      isConnected: false,
      message: "",
      reconnectError: false
    },
    page: 0, 
    authenticated: false,
    pageStruct: null
  },
  mutations: {
    SOCKET_ONOPEN(state, event) {
      Vue.prototype.$socket = event.currentTarget;
      state.socket.isConnected = true;
    },
    SOCKET_ONCLOSE(state, event) {
      state.socket.isConnected = false;
    },
    SOCKET_ONERROR(state, event) {
      console.error(state, eval);
    },
    // default handler called for all methods
    SOCKET_ONMESSAGE(state, message) {
      let msgArray = message.data.split(";");
      if(msgArray.length > 0){
        let event = msgArray.shift();
        let payload = msgArray;
        switch (event) {
          case "0":{
            //wsEvent_invalid this shoul be never received by client --> no hadler
          }break;
          case "1":{
            //wsEvent_dataNodeChange

          }break;
          case "2":{
            //wsEvent_paramNodeChange

          }break;
          case "3":{
            //wsEvent_pageChange
            if(payload.length == 1){
              let newPage = parseInt(payload[0], 10);
              if(newPage != NaN){
                state.page = newPage;
                store.dispatch('ws_send_wsEvent_structure', newPage);
                console.log("page changed to: " + newPage);
              }
            }
          }break;
          case "4":{
            //wsEvent_structure
            if(payload.length == 1){
              let pageObj = JSON.parse(payload[0]);
              if(typeof pageObj === 'object' && pageObj !== null){
                state.pageStruct = pageObj;
                store.dispatch('wsEvent_reqSendParams');
                store.dispatch('wsEvent_reqSendDataNodes');
              }
            }
          }break;
          case "5":{
            //wsEvent_authentification
            if(payload.length == 0){
              state.authenticated = true;
              store.dispatch('ws_send_wsEvent_pageChange', 1);
            }
          }break;
          case "6":{
            //wsEvent_reqSendParams this should be never received by client --> no hadler
          }break;
          case "7":{
            //wsEvent_reqSendDataNodes this should be never received by client --> no hadler
          }break;
        }
      }
    }
  },
  actions: {
    wsEvent_reqSendParams: function(context){
      context.dispatch('ws_sendEvent', "6");
    },
    wsEvent_reqSendDataNodes: function(context){
      context.dispatch('ws_sendEvent', "7");
    },
    ws_send_wsEvent_structure: function(context, page){
      let sendstr = "4;";
      sendstr += page;
      context.dispatch('ws_sendEvent', sendstr);
    },
    ws_send_wsEvent_pageChange: function(context, newPage){
      let sendstr = "3;";
      sendstr += newPage;
      context.dispatch('ws_sendEvent', sendstr);
    },
    ws_send_wsEvent_authentification: function(context, credentials) {
      let sendstr = "5;";
      sendstr += credentials.shift();
      sendstr += ";";
      sendstr += credentials.shift();
      context.dispatch('ws_sendEvent', sendstr);
    },
    ws_sendEvent: function(context, message) {
      if(context.state.socket.isConnected){
        Vue.prototype.$socket.send(message);
      }
    }
  }
});

export default store;