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

    this.app.post('/setPositionBrightnessWidth', (req, res) => {
      const { position, brightness, width, stripIndex } = req.body;
      this.emit('/setPositionBrightnessWidth', {
        brightness: parseInt(brightness),
        position: parseInt(position),
        width: parseInt(width),
        stripIndex: parseInt(stripIndex)
      });
      res.status(200).send('Event received');
    });

    // setColor
    this.app.post('/setColor', (req, res) => {
      const { r, g, b } = req.body;
      this.emit('/setColor', {
        r: parseInt(r),
        g: parseInt(g),
        b: parseInt(b)
      });
      res.status(200).send('Event received');
    });

    //play-horns and stop-horns
    this.app.post('/play-horns', (req, res) => {
      this.emit('/play-horns');
      res.status(200).send('Command received');
    });

    this.app.post('/stop-horns', (req, res) => {
      this.emit('/stop-horns');
      res.status(200).send('Command received');
    });

    // set-horns-volume
    this.app.post('/set-horns-volume', (req, res) => {
      const { volume } = req.body;
      this.emit('/set-horns-volume', {
        volume: parseInt(volume)
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