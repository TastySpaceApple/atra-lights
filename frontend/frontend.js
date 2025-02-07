import { config } from "./frontend-config.js"

const defaultState = {
  brightness: 50,
  position: 50,
  width: 50
}

let states = []

let currentDeviceId = 1;

function postRequest(url, data) {
  return fetch(url, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(data)
  })
}

function setLedNumber(stripIndex, ledNumber) {
  postRequest('/setLedNumber', { ledNumber, stripIndex })
}

function setPositionBrightnessWidth(stripIndex, position, brightness, width) {
  postRequest('/setPositionBrightnessWidth', { position, brightness, width, stripIndex })
}

function setColor(r, g, b) {
  postRequest('/setColor', { r, g, b })
}

const renderDevicesList = () => {
  const devicesList = document.getElementById('devicesList')
  devicesList.innerHTML = ''
  config.devices.forEach(device => {
    const deviceElement = document.createElement('label')
    deviceElement.className = 'device'
    deviceElement.innerHTML = `
      <input type="radio" name="device" value="${device.id}" />
      <div class="device-info">
        <div class="device-id">${device.id}</div>
        <div class="device-name">${device.name}</div>
        <div class="mac">(${device.mac})</div>
      </div>
    `
    devicesList.appendChild(deviceElement)
  })

}

renderDevicesList();

// binds
document.getElementById('devicesList').addEventListener('change', (event) => {
  if (event.target.name === 'device') {
    const selectedDeviceId = event.target.value;
    currentDeviceId = selectedDeviceId;
    const state = states[selectedDeviceId] || defaultState;
    document.getElementById('brightness').value = state.brightness;
    document.getElementById('position').value = state.position;
    document.getElementById('width').value = state.width;
    document.getElementById('led-number').value = state.ledNumber || 0;
  }
});


const changeState = (stripIndex, update) => {
  if(!states[stripIndex]){
    states[stripIndex] = defaultState;
  }
  states[stripIndex] = { ...states[stripIndex], ...update }
}


document.getElementById('btn-set-led-number').addEventListener('click', () => {
  const ledNumber = document.getElementById('led-number').value;
  setLedNumber(currentDeviceId, ledNumber);
  changeState(currentDeviceId, {ledNumber});
})

//width
document.getElementById('width').addEventListener('input', (event) => {
  const width = event.target.value;
  
  changeState(currentDeviceId, {width});
  setPositionBrightnessWidth(currentDeviceId, states[currentDeviceId].brightness, states[currentDeviceId].position, width)

})

//position
document.getElementById('position').addEventListener('input', (event) => {
  const position = event.target.value;
  changeState(currentDeviceId, {position});

  setPositionBrightnessWidth(currentDeviceId, states[currentDeviceId].brightness, position, states[currentDeviceId].width)

})

//brightness
document.getElementById('brightness').addEventListener('input', (event) => {
  const brightness = event.target.value;
  changeState(currentDeviceId, {brightness});

  setPositionBrightnessWidth(currentDeviceId, brightness, states[currentDeviceId].position, states[currentDeviceId].width)

})

// color
document.getElementById('color').addEventListener('input', (event) => {
  const color = event.target.value;
  const r = parseInt(color.substring(1, 3), 16);
  const g = parseInt(color.substring(3, 5), 16);
  const b = parseInt(color.substring(5, 7), 16);
  setColor(r, g, b);
})


// select the first
devicesList.querySelector('input').checked = true;