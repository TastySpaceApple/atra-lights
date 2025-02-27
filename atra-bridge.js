const OSC = require('osc-js')
const LedController = require('./led-controller.js');
const { AtraBrightnessMessage, AtraColorMessage, AtraSetLedNumberMessage, AtraSoundMessage } = require('./atra-messages.js');
const AtraConfigServer = require('./atra-config-server.js');

const osc = new OSC({
  plugin: new OSC.DatagramPlugin({
    open: { port: 6000 },
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
    chunksToUpdate.push(chunk);
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

const sendOneTimeMessage = (message) => {
  // pause the state update
  clearTimeout(sendStateUpdate);
  // send the message
  ledController.send(message);
  // resume the state update
  setTimeout(sendStateUpdate, 18);
}

osc.on('/send', (msg) => {
  const [chunk, position, brightness, width] = msg.args;
  updateState(chunk, position, brightness, width);
})

osc.on('/rgb', (msg) => {
  const [r, g, b] = msg.args;
  clearTimeout(sendStateUpdate);
  const atraColorMessage = new AtraColorMessage(0, r, g, b);
  ledController.send(atraColorMessage);
  setTimeout(sendStateUpdate, 20);
})

osc.on('/sound/start', (msg) => {
  const [chunk, track] = msg.args;
  const atraSoundMessage = new AtraSoundMessage(chunk);
  atraSoundMessage.start(track || 0);
  sendOneTimeMessage(atraSoundMessage);
})

osc.on('/sound/stop', (msg) => {
  const [chunk] = msg.args;
  const atraSoundMessage = new AtraSoundMessage(chunk);
  atraSoundMessage.stop();
  sendOneTimeMessage(atraSoundMessage);
});

osc.on('/sound/volume', (msg) => {
  const [chunk, volume] = msg.args;
  const atraSoundMessage = new AtraSoundMessage(chunk);
  atraSoundMessage.setVolume(volume);
  sendOneTimeMessage(atraSoundMessage);
});

const ledController = new LedController();
ledController.open();

const sendStateUpdate = async () => {
  const chunkToUpdate = chunksToUpdate.shift();
  if (chunkToUpdate !== undefined) {
    const { chunk, position, brightness, width } = state.find((s) => s.chunk === chunkToUpdate);
    const atraMessage = new AtraBrightnessMessage(chunk, position, brightness, width);
    await ledController.send(atraMessage);
  }
}

setInterval(sendStateUpdate, 18);

const sendAtraMessage = async (msg) => {
  clearInterval(sendStateUpdate);
  ledController.send(msg);
  setTimeout(sendStateUpdate, 20);
}

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
  if(stripIndex === 0){
    console.log('not setting all led number!');
    return;
  }
  sendAtraMessage(
    new AtraSetLedNumberMessage(stripIndex, ledNumber)
  );
})

configServer.on('/setPositionBrightnessWidth', (data) => {
  const { stripIndex, position, brightness, width } = data;
  updateState(stripIndex, position, brightness, width);
})

configServer.on('/setColor', (data) => {
  const { r, g, b } = data;
  const atraColorMessage = new AtraColorMessage(0, r, g, b);
  ledController.send(atraColorMessage);
})

configServer.on('/play-horns', () => {
  const atraSoundMessage = new AtraSoundMessage(0);
  atraSoundMessage.start(0);
  sendOneTimeMessage(atraSoundMessage);
});

configServer.on('/stop-horns', () => {
  const atraSoundMessage = new AtraSoundMessage(0);
  atraSoundMessage.stop();
  sendOneTimeMessage(atraSoundMessage);
})

//
configServer.on('/set-horns-volume', (data) => {
  const { volume } = data;
  const atraSoundMessage = new AtraSoundMessage(0);
  atraSoundMessage.setVolume(volume);
  sendOneTimeMessage(atraSoundMessage);
});

// demo set volume
setTimeout(() => {
  const msg = new AtraSoundMessage(18);
  msg.stop(0);
  sendAtraMessage(
    msg
  );
}, 2000);