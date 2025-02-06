const OSC = require('osc-js')
const LedController = require('./led-controller.js');
const { AtraBrightnessMessage, AtraColorMessage, AtraSetLedNumberMessage } = require('./atra-messages.js');
const AtraConfigServer = require('./atra-config-server.js');

const osc = new OSC({
  plugin: new OSC.DatagramPlugin({
    open: { host: '192.168.1.32', port: 6000 },
  })
})

osc.open()

let state = [
]

let chunksToUpdate = [
]

const updateState = (chunk, position, brightness, width) => {
  const index = state.findIndex((s) => s.chunk === chunk);
  if (index === -1) {
    state.push({ chunk, position, brightness, width });
  } else {
    // find the update and update it
    if (state[index].position === position && state[index].brightness === brightness && state[index].width === width) {
      return;
    }

    if (!chunksToUpdate.includes(chunk)) {
      chunksToUpdate.push(chunk);
    }

    state[index] = { chunk, position, brightness, width };
  }
}

osc.on('/send', (msg) => {
  const [chunk, position, brightness, width] = msg.args;
  updateState(chunk, position, brightness, width);
})

osc.on('/rgb', (msg) => {
  const [r, g, b] = msg.args;
  clearTimeout(sendStateUpdate);
  const atraColorMessage = new AtraColorMessage(1, r, g, b);
  ledController.send(atraColorMessage);
  setTimeout(sendStateUpdate, 18);
})

const ledController = new LedController();
ledController.open();

const sendStateUpdate = async () => {
  const chunkToUpdate = chunksToUpdate.shift();
  if (chunkToUpdate !== undefined) {
    const { chunk, position, brightness, width } = state.find((s) => s.chunk === chunkToUpdate);
    const atraMessage = new AtraBrightnessMessage(chunk, position, brightness, width);
    console.log('Sending message', atraMessage);
    await ledController.send(atraMessage);
  }
}

setInterval(sendStateUpdate, 18);

// (function () {

//   let direction = 1;
//   let width = 0;

//   const send = async () => {
//     width += direction;
//     if (width === 99 || width === 0) {
//       direction *= -1;
//     }

//     const atraMessage = new AtraBridgeMessage(1, 50, 50, width);
//     await ledController.send(atraMessage);

//     setTimeout(send, 10);
//   }
//   send();
// })();


// setTimeout(() => {
//   // RBG
//   const msg = new AtraSetLedNumberMessage(1, 20)
//   ledController.send(msg);
// }, 1000);

const configServer = new AtraConfigServer();
configServer.start(3000);

configServer.on('/setLedNumber', (data) => {
  const { stripIndex, ledNumber } = data;
  const msg = new AtraSetLedNumberMessage(stripIndex, ledNumber);
  ledController.send(msg);
})