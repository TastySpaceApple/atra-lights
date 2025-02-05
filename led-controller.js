const { SerialPort } = require('serialport');

const productId = '7523';
const baudRate = 115200;

const _debug_ = true;

class LedController {
  constructor() {
    this.isConnected = false;
  }

  async open() {
    const ports = await SerialPort.list();
    console.log(ports);

    const port = ports.find((port) => port.productId === productId);

    if(!port) {
      console.error('No LED controller found');
      return;
    }

    this.port = new SerialPort({
      baudRate,
      path: port.path
    });

    if(_debug_) {
      this.port.on('data', (data) => {
        console.log(data.toString());
      });
    }

    return new Promise((resolve, reject) => {
      this.port.on('open', () => {
        this.isConnected = true;
        resolve();
      });
    });
  }

  send(message) {
    if(!this.isConnected) return;
    if(this.isSending) return;

    console.log('Sending message');
    this.isSending = true;

    const byteArray = message.toByteArray();
    this.port.write(Buffer.from(byteArray));
    
    this.isSending = false;
  }
}

module.exports = LedController;