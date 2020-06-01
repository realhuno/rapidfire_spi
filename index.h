const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<div id="demo">
<h1>Rapidfire SPI Wifi Bridge</h1>

  <button type="button" onclick="sendesp32('B=1')">ImmersionRC/Fatshark</button>
  <button type="button" onclick="sendesp32('B=2')">Raceband</button><BR>
  <button type="button" onclick="sendesp32('B=3')">Boscam E</button>
  <button type="button" onclick="sendesp32('B=4')">Boscam B</button><BR>
  <button type="button" onclick="sendesp32('B=5')">Boscam A</button>
  <button type="button" onclick="sendesp32('B=6')">LowRace</button><BR>
  <button type="button" onclick="sendesp32('B=7')">Band X</button><BR>
  <br>
  <button type="button" onclick="sendesp32('C=1')">CH1</button>
  <button type="button" onclick="sendesp32('C=2')">CH2</button>
  <button type="button" onclick="sendesp32('C=3')">CH3</button>
  <button type="button" onclick="sendesp32('C=4')">CH4</button>
  <button type="button" onclick="sendesp32('C=5')">CH5</button>
  <button type="button" onclick="sendesp32('C=6')">CH6</button>
  <button type="button" onclick="sendesp32('C=7')">CH7</button>
  <button type="button" onclick="sendesp32('C=8')">CH8</button>
  <br>
   <button type="button" onclick="sendesp32('S>')">Beep</button>
   <button type="button" onclick="sendesp32('T=Hello')">Hello</button>
   <button type="button" onclick="reconnect('true')">MQTT Connect!!</button>
   <button type="button" onclick="led('on')">LED ON!</button>
   <button type="button" onclick="led('off')">LED OFF!</button>
    <br>
   <button type="button" onclick="sendesp32('O=0')">No OSD</button>
   <button type="button" onclick="sendesp32('O=1')">OSD Mode1</button>
   <button type="button" onclick="sendesp32('O=2')">OSD Mode2</button>
   <button type="button" onclick="sendesp32('O=3')">OSD Mode3</button>
   <button type="button" onclick="sendesp32('O=4')">OSD Mode4</button>
   <button type="button" onclick="sendesp32('O=5')">OSD Mode5</button>
   <button type="button" onclick="sendesp32('O=6')">OSD Mode6</button>
   <button type="button" onclick="sendesp32('O=7')">OSD Mode7</button>
   <button type="button" onclick="sendesp32('O=8')">OSD Mode8</button>
   <button type="button" onclick="sendesp32('O=9')">OSD Mode9</button>
  <!--<input type="text" id="valuein">-->
   <!-- <button type="button" onclick="sendesp32(valuein.value)">Send</button><BR>-->
</div>

<div>

  Send : <span id="ADCValue">0</span><br>
  
</div>
<script>

function dec2hex(n){
    n = parseInt(n); var c = 'ABCDEF';
    var b = n / 16; var r = n % 16; b = b-(r/16); 
    b = ((b>=0) && (b<=9)) ? b : c.charAt(b-10);    
    return ((r>=0) && (r<=9)) ? b+''+r : b+''+c.charAt(r-10);
}

function sendData(led) {
 

  led=Number(led).toString(16);
  console.log(led);
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LEDState").innerHTML =
      this.responseText;
    }
  };

  xhttp.open("GET", "setLED?LEDstate=S1L01000"+led, true);
  xhttp.send();
}

function sendesp32(led) {
 

  
  console.log(led);
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LEDState").innerHTML =
      this.responseText;
    }
  };

  xhttp.open("GET", "setLED?LEDstate="+led, true);
  xhttp.send();
}


function reconnect(led) {
 

  
  console.log(led);
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LEDState").innerHTML =
      this.responseText;
    }
  };

  xhttp.open("GET", "reconnect", true);
  xhttp.send();
}


function led(led) {
 

  
  console.log(led);
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LEDState").innerHTML =
      this.responseText;
    }
  };

  xhttp.open("GET", "led?led=" + led, true);
  xhttp.send();
}

setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
}, 200); //2000mSeconds update rate

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ADCValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readADC", true);
  xhttp.send();
}
</script>

</body>
</html>
)=====";
