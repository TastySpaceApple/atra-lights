const express = require('express');
const EventEmitter = require('events');

class AtraConfigServer extends EventEmitter {
  constructor() {
    super();
    this.app = express();
    this.app.use(express.json());

    this.app.use(express.static('frontend'));

    this.app.post('/setLedNumber', (req, res) => {
      const { ledNumber, stripIndex } = req.body;
      this.emit('/setLedNumber', {
        ledNumber: parseInt(ledNumber),
        stripIndex: parseInt(stripIndex)
      });
      res.status(200).send('Event received');
    });
  }

  start(port) {
    this.app.listen(port, () => {
      console.log(`Atra Config Server is running on http://localhost:${port}`);
    });
  }
}

module.exports = AtraConfigServer;