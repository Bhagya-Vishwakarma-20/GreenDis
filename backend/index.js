const express = require('express');
const app = express();
const GreenDisClient = require("./greendisClient")
app.use(express.json());

const cache = new GreenDisClient();

(async () => {
  await cache.connect();
  console.log("Connected to GreenDis");
})();

app.get("/user/:id", async (req, res) => {

  const key = "user:" + req.params.id;

  const cached = await cache.get(key);

  if (cached !== "ERROR key_not_found") {
    return res.json({ source: "cache", data: JSON.parse(cached) });
  }

  // simulate DB
  const user = { id: req.params.id, name: "Bhagya" };

  await cache.set(key, JSON.stringify(user));

  res.json({ source: "db", data: user });

});
app.get('/',(req,res)=>{
    res.send("hhe");
})
app.listen(3000)