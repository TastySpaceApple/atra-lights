const osc = require('osc');
const LedController = require('./led-controller.js');
const AtraBridgeMessage = require('./atra-bridge-message.js');

var udp = new osc.UDPPort({
  localAddress: "192.168.1.32", // shouldn't matter here
  localPort: 6000, // not receiving, but here's a port anyway
});

udp.open();

udp.on("message", function (msg) {
  console.log(msg);
  const [channel, chunk, position, brightness, width] = msg.args;

  const atraMessage = new AtraBridgeMessage(0, chunk, position, brightness, width);
  ledController.send(atraMessage);
});

const ledController = new LedController();
ledController.open();


let direction = 1;
let width = 0;

const send = async () => {
  width += direction;
  if (width === 99 || width === 0) {
    direction *= -1;
  }

  const atraMessage = new AtraBridgeMessage(1, 1, 50, 50, width);
  await ledController.send(atraMessage);

  // await ledController.send(new AtraBridgeMessage(0, 3, 50, 10, width));

  setTimeout(send, 10);
}

// send();