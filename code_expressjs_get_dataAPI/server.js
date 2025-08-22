const express = require("express");
const app = express();
const port = 3000;

const apiKey = "YGrvClJOSTqVt4YEuA9gIQ==";

const defaultStopCode = "01029";

app.get("/bus-arrival/:stopCode", async (req, res) => {
  try {
    const stopCode = req.params.stopCode || defaultStopCode;
    const response = await fetch(
      `https://datamall2.mytransport.sg/ltaodataservice/v3/BusArrival?BusStopCode=${stopCode}`,
      {
        headers: { accountKey: apiKey, accept: "application/json" },
      }
    );
    if (!response.ok) {
      throw new Error(`Datamall API error: ${response.status}`);
    }

    const data = await response.json();

    // Return raw JSON (you can clean later)
    res.json(data);
  } catch (error) {
    console.error(error.message);
    res.status(500).json({ error: "failed to fetch bus arrival data" });
  }
});

// app.get("/bus-arrival", (req, res) => {
//   res.send("Hello World!");
// });

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`);
});
