// const express = require('express');
// const http = require('http');

// const app = express();
// const port = 3000;
// const server = http.createServer(app);


// app.use(express.json());

// // Endpoint to receive data from the ESP8266
// app.post('/data', (req, res) => {
//   const receivedData = req.body;
//   console.log('Received data:', receivedData);

//   res.sendStatus(200);
// });

// // Start the server
// server.listen(port, () => {
//   console.log(`Server running on port ${port}`);
// });


const express = require('express');
const http = require('http');
const cors = require('cors');

const app = express();
const port = 3000;
const server = http.createServer(app);

app.use(express.json());
app.use(cors()); // Enable CORS

let storedCloseData = null;
let storedOpenData = null;

// Endpoint to receive data arm close data from the ESP
app.post('/close', (req, res) => {
  const receivedCloseData = req.body;
  console.log('Received close data:', receivedCloseData);

  // Store the received data
  storedCloseData = receivedCloseData;

  res.sendStatus(200);
});

// Endpoint to receive arm open data from the ESP
app.post('/open', (req, res) => {
  const receivedOpenData = req.body;
  console.log('Received open data:', receivedOpenData);

  // Store the received data
  storedOpenData = receivedOpenData;

  res.sendStatus(200);
});

//Get request for Unity

//Get request for closing the arm
app.get('/close', (req, res) => {
  if (storedCloseData) {
    // Return the stored data
    res.json(storedCloseData);

    // Set storedCloseData to an object with PositionRef as 2
    storedCloseData = { ClosePositionRef: 2 };
  } else {
    res.status(404).json({ message: 'Data not found' });
  }
});


//Get request for opening the arm
app.get('/open', (req, res) => {
  if (storedOpenData) {
    // Return the stored data
    res.json(storedOpenData);

    // Set storedOpenData to an object with PositionRef as 2
    storedOpenData = { ClosePositionRef: 2 };
  } else {
    res.status(404).json({ message: 'Data not found' });
  }
});

// Start the server
server.listen(port, () => {
  console.log(`Server running on port ${port}`);
});
