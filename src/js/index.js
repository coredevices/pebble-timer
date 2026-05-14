// ********** Timeline ********** //
// "Exersized" pin
var timerPIN = {
  "id": "",
  "time": 0,
  "layout": {
    "type": "weatherPin",
    "title": "Timer Complete",
    "subtitle": "50:00",
    "tinyIcon": "system://images/ALARM_CLOCK",
    "largeIcon": "system://images/ALARM_CLOCK",
    "locationName": " ",
    "backgroundColor": "#55AAFF"
  },
  "actions": [
    {
      "title": "Open Timer",
      "type": "openWatchApp",
      "launchCode": 10
    }
    // {
    //   "title": "Restart Timer",
    //   "type": "openWatchApp",
    //   "launchCode": 11
    // },
    // {
    //   "title": "Delete Timer",
    //   "type": "openWatchApp",
    //   "launchCode": 12
    // }
  ]
};

// ***** Timeline Lib ***** //
// The Timeline public URL root
var API_URL_ROOT = 'https://timeline-api.getpebble.com/';
/**
 * Send a request to the Pebble public web timeline API.
 * @param pin The JSON pin to insert. Must contain 'id' field.
 * @param type The type of request, either PUT or DELETE.
 * @param callback The callback to receive the responseText after the request has completed.
 */
function timelineRequest(pin, type, callback) {
    // User or shared?
    var url = API_URL_ROOT + 'v1/user/pins/' + pin.id;

    // Create XHR
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        console.log('timeline: response received: ' + this.responseText);
        callback(this.responseText);
    };
    xhr.open(type, url);

    // Get token
    Pebble.getTimelineToken(function (token) {
        // Add headers
        xhr.setRequestHeader('Content-Type', 'application/json');
        xhr.setRequestHeader('X-User-Token', '' + token);

        // Send
        xhr.send(JSON.stringify(pin));
        console.log('timeline: request sent.');
    }, function (error) { console.log('timeline: error getting timeline token: ' + error); });
}
/**
 * Insert a pin into the timeline for this user.
 * @param pin The JSON pin to insert.
 * @param callback The callback to receive the responseText after the request has completed.
 */
function insertUserPin(pin, callback) {
    timelineRequest(pin, 'PUT', callback);
}
/**
 * Delete a pin from the timeline for this user.
 * @param pin The JSON pin to delete.
 * @param callback The callback to receive the responseText after the request has completed.
 */
function deleteUserPin(pin, callback) {
    timelineRequest(pin, 'DELETE', callback);
}



// ********** AppMessage ********** //
// send message to phone
function send_to_phone(){
  // create dictionary
  var dict = {
    'KEY_DURATION':0
  };

  // send to pebble
  Pebble.sendAppMessage(dict,
    function(e) {
      console.log('Send successful.');
    },
    function(e) {
      console.log('Send failed!');
    }
  );
}

// message received
Pebble.addEventListener('appmessage', function(e) {
  // check for key
  if (e.payload.hasOwnProperty('KEY_DURATION')){
    // check that it is valid to send pins i.e. its SDK 3.0 or greater
    if (typeof Pebble.getTimelineToken == 'function') {
      // update pin time
      timerPIN.id = e.payload.KEY_UNIQUEID.toString();
      // show total time
      var tot = e.payload.KEY_TOTAL_TIME / 60;
      var hr = Math.floor(tot / 60);
      var min = Math.floor(tot % 60);
      if (hr < 10) hr = "0" + hr;
      if (min < 10) min = "0" + min;
      timerPIN.layout.subtitle = hr + ":" + min;
      // zero two least significant digits
      timerPIN.actions[0].launchCode = timerPIN.id * 100 + 10;
      // timerPIN.actions[1].launchCode = timerPIN.id * 100 + 11;
      // timerPIN.actions[2].launchCode = timerPIN.id * 100 + 12;
      // check if deleting
      if (e.payload.KEY_DURATION > 0){
        // update date
        var tDate = new Date();
        tDate.setSeconds(tDate.getSeconds() + e.payload.KEY_DURATION);
        timerPIN.time = tDate.toISOString();
        // insert pin
        insertUserPin(timerPIN, function (responseText) {
          console.log('Pin Sent Result (' + timerPIN.id + '): ' + responseText);
        });
      }
      else{
        deleteUserPin(timerPIN, function (responseText) {
          console.log('Pin Deleted Result (' + timerPIN.id + '): ' + responseText);
        });
      }
    }
  }
});

// loaded and ready
Pebble.addEventListener('ready', function(e) {
  console.log("JS ready!");
});