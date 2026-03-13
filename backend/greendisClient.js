const net = require("net");

class GreenDisClient {
  constructor(host = "127.0.0.1", port = 6379) {
    this.host = host;
    this.port = port;
    this.socket = null;
    this.buffer = "";
    this.queue = [];
  }

  connect() {
    return new Promise((resolve, reject) => {
      this.socket = net.createConnection(
        { host: this.host, port: this.port },
        resolve
      );

      this.socket.on("data", (data) => {
        this.buffer += data.toString();

        let idx;
        while ((idx = this.buffer.indexOf("\n")) !== -1) {
          const line = this.buffer.slice(0, idx);
          this.buffer = this.buffer.slice(idx + 1);

          const pending = this.queue.shift();
          if (pending) pending.resolve(line);
        }
      });

      this.socket.on("error", reject);
    });
  }

  send(cmd) {
    return new Promise((resolve, reject) => {
      this.queue.push({ resolve, reject });
      this.socket.write(cmd + "\n");
    });
  }

  async set(key, value) {
    return this.send(`SET ${key} ${value}`);
  }

  async get(key) {
    return this.send(`GET ${key}`);
  }

  async del(key) {
    return this.send(`DEL ${key}`);
  }
}

module.exports = GreenDisClient;