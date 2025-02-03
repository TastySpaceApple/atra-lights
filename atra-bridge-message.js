// a message is represented using the following properties:
// channel (number): 1-14
// chunk (number): 1-8
// position (number): 0-100
// brightness (number): 0-100
// width (number): 0-100

const wrapAround100 = (value) => {
  return value > 100 ? value % 100 : value;
}

class AtraBridgeMessage {
  constructor(channel, chunk, position, brightness, width) {
    this.channel = channel;
    this.chunk = chunk;
    this.position = wrapAround100(position);
    this.brightness = wrapAround100(brightness);
    this.width = wrapAround100(width);
  }

  // Method to display message details
  display() {
    console.log(`Channel: ${this.channel}, Chunk: ${this.chunk}, Position: ${this.position}, Brightness: ${this.brightness}, width: ${this.width}`);
  }

  toByteArray() {
    // the first bye is the channel and the chunk
    let byte1 = (this.channel << 4) | this.chunk;
    // the second and third bytes together are the position, brightness and width

    return [byte1, this.position, this.brightness, this.width];
  }
}

module.exports = AtraBridgeMessage;