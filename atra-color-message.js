class AtraColorMessage {
  constructor(chunk, r, g, b){
    this.chunk = chunk;
    this.r = r;
    this.g = g;
    this.b = b;
  }

  toByteArray(){
    return [this.chunk, true, this.r, this.g, this.b];
  }
}

module.exports = AtraColorMessage;