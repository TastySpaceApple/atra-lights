const wrapAround100 = (value) => {
  return value > 100 ? value % 100 : value;
}

class AtraBridgeMessage {
  constructor(chunk, position, brightness, width) {
    this.chunk = chunk;
    this.position = wrapAround100(position);
    this.brightness = wrapAround100(brightness);
    this.width = wrapAround100(width);
  }

  toByteArray() {
    return [this.chunk, false, this.position, this.brightness, this.width];
  }
}

module.exports = AtraBridgeMessage;