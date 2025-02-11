const MESSAGE_TYPE_BRIGHTNESS = 0;
const MESSAGE_TYPE_COLOR = 1;
const MESSAGE_TYPE_LED_NUMBER = 2;

const MESSAGE_TYPE_SOUND_START = 8;
const MESSAGE_TYPE_SOUND_STOP = 9;
const MESSAGE_TYPE_SOUND_VOLUME = 10;

const wrapAround100 = (value) => {
  return value > 100 ? value % 100 : value;
}

class AtraBrightnessMessage {
  constructor(chunk, position, brightness, width) {
    this.chunk = chunk;
    this.position = wrapAround100(position);
    this.brightness = wrapAround100(brightness);
    this.width = wrapAround100(width);
  }

  toByteArray() {
    return [this.chunk, MESSAGE_TYPE_BRIGHTNESS, this.position, this.brightness, this.width];
  }
}

class AtraColorMessage {
  constructor(chunk, r, g, b){
    this.chunk = chunk;
    this.r = r;
    this.g = g;
    this.b = b;
  }

  toByteArray(){
    return [this.chunk, MESSAGE_TYPE_COLOR, this.r, this.g, this.b];
  }
}

class AtraSetLedNumberMessage {
  constructor(chunk, ledNumber){
    this.chunk = chunk;
    this.ledNumber = ledNumber;
  }

  toByteArray(){
    return [this.chunk, MESSAGE_TYPE_LED_NUMBER, this.ledNumber, 0, 0];
  }
}

class AtraSoundMessage {
  constructor(chunk){
    this.chunk = chunk;
    this.messageType = MESSAGE_TYPE_SOUND_START;
    this.volume = 0;
    this.track = 0;
  }

  start(track){
    this.messageType = MESSAGE_TYPE_SOUND_START;
    this.track = track ?? 0;
  }

  stop(){
    this.messageType = MESSAGE_TYPE_SOUND_STOP;
  }

  setVolume(volume){
    this.messageType = MESSAGE_TYPE_SOUND_VOLUME;
    this.volume = volume;
  }

  toByteArray(){
    return [this.chunk, this.messageType, this.volume, 0, 0];
  }
}

module.exports = {
  AtraBrightnessMessage,
  AtraColorMessage,
  AtraSetLedNumberMessage,
  AtraSoundMessage
}