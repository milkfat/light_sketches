function sf(sketch,flag) {
    var http = new XMLHttpRequest();
    http.open('POST', 'setsketch', true);
    http.setRequestHeader('Content-type','application/x-www-form-urlencoded');
    var params = 'sketch=' + sketch + '&flag=' + flag;
    http.send(params);
}

document.onkeydown = checkKeyDown;
document.onkeyup = checkKeyUp;
keyList = {};

function checkKeyDown(e) {
  if(keyList[e.keyCode]) {
    return;
  } else {
    keyList[e.keyCode] = true;
  }
  checkKey(e, "k");
}

function checkKeyUp(e) {
  keyList[e.keyCode] = false;
  checkKey(e, "K");
}

function checkKey(e, name) {
    e = e || window.event;
    
    amInline(name,e.keyCode)
}







connected = document.getElementById("connected");
log = document.getElementById("log");
form = document.getElementById("myForm");
state = document.getElementById("status");

if (window.WebSocket === undefined) {
    state.innerHTML = "sockets not supported";
    state.className = "fail";
}else {
    if (typeof String.prototype.startsWith != "function") {
        String.prototype.startsWith = function (str) {
        return this.indexOf(str) == 0;
        };
    }

window.addEventListener("load", onLoad, false);
}
webshocketattempt = 0;
function onLoad() {
    if (webshocketattempt > 1) {
    return;
    }
    if (webshocketattempt == 0) {
    websocket = new WebSocket(((window.location.protocol === "https:") ? "wss://" : "ws://") + window.location.hostname + "/socket");
    } else {
    websocket = new WebSocket(((window.location.protocol === "https:") ? "wss://" : "ws://") + window.location.hostname + ":8081/socket");
    }
    webshocketattempt++;
    websocket.onopen = function(evt) { onOpen(evt) };
    websocket.onclose = function(evt) { onClose(evt) };
    websocket.onmessage = function(evt) { onMessage(evt) };
    websocket.onerror = function(evt) { onError(evt) };
    setInterval(wsSendTimer, 33);
}

function onOpen(evt) {
    state.className = "success";
    state.innerHTML = "Connected to server";
    websocket.send('{"i":"1"}')
}

function onClose(evt) {
    state.className = "fail";
    state.innerHTML = "Not connected. Attempting to reconnect...";
    //connected.innerHTML = "0";
    // Try to reconnect in 5 seconds
    websocket = null;
    setTimeout(function(){onLoad()}, 500);
}


function onMessage(evt) {
    var message = evt.data;

    log.innerHTML = '<li class = "message">' + 
message + "</li>" + log.innerHTML;
var elem = document.querySelector('li:nth-child(1)');
if (elem) {
    elem.parentNode.removeChild(elem);
}
try {
    var json_stuff = JSON.parse(evt.data);
    for (x in json_stuff) {
        x2 = document.getElementById(x);
        if (typeof json_stuff[x] == typeof true){
        // variable is a boolean
        x2.checked = json_stuff[x];
        } else if (x == 'clients') {
        x2.innerHTML = '';
        for (y in json_stuff[x]) {
        x2.innerHTML += '<br />' + json_stuff[x][y].name + ': <a href="http://' + json_stuff[x][y].address + '">' + json_stuff[x][y].address + '</a>';
        }
        } else {
        // variable is not a boolean
        x2.value = json_stuff[x];
        }
    }
    }
    catch {
        //contents is HTML text, insert it into the page
        document.getElementById('s').innerHTML = evt.data;
        let range = document.querySelectorAll('input[type=range]');
        let par = document.getElementById('s');
        for (i of range) {
            let node = document.createElement('output');
            node.setAttribute('id', 'o' + i.getAttribute('id') );
            node.innerHTML = i.value;
            par.insertBefore(node, i);
            i.setAttribute('oninput', 'am("' + i.getAttribute('id') + '","' + i.getAttribute('id') + '")');
        }
    }
}

function onError(evt) {
    websocket = null;
    state.className = "fail";
    state.innerHTML = "Communication error";
}

var message_buffer = {};
var ws_send_now = 1;
function wsSendTimer() {
ws_send_now = 1;
wsSend();
}
function wsSend() {
if(!ws_send_now) return;
var cnt = 0;
var comma = '';
var ws_message = '{';
for (jkey in message_buffer) {
    ws_message += comma + '"' + jkey + '":"' + message_buffer[jkey] + '"';
    cnt++;
    if (cnt > 0) {
    comma = ',';
    }
}
if (cnt > 0) {
    ws_message += '}';
    websocket.send(ws_message);
    message_buffer = {};
    ws_send_now = 0;
}
}

function am(jkey,jid) {
    message_buffer[jkey] = document.getElementById(jid).value;
    try {
        document.getElementById('o'+jid).value = document.getElementById(jid).value;
    }
    catch (err) {
        
    }
    wsSend();
}

function amInline(jkey,jval) {
    message_buffer[jkey] = jval;
    wsSend();
}

function amBoolean(jkey,jid) {
    var val = document.getElementById(jid).checked+0;
    message_buffer[jkey] = val;
    wsSend();
}