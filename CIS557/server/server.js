const express = require('express');
const cors = require('cors');
const path = require('path');
const mongoose = require('mongoose');
require('dotenv').config();

const MONGODB_URI = 'mongodb+srv://Charles:990601lzy@cluster.uglyf.mongodb.net/CIS557Proj';

// ----------------------------- create server --------------------------------
const app = express();
const httpServer = require('http').createServer(app);
const io = require('socket.io')(httpServer, {
  serveClient: false,
  cors: {
    // dev
    origin: '*',
  },
});

// -------------------------------- routers -----------------------------------
const userRouter = require('./routes/userRoute');
const publicGroupRouter = require('./routes/publicGroupRoute');
const publicPostRouter = require('./routes/publicPostRoute');
const loginRouter = require('./routes/loginRoute');
const deactivateRouter = require('./routes/deactivateRoute');
const fileRouter = require('./routes/fileRoute');
const popularPostRouter = require('./routes/popularPublicPostRoute');
const suggestedGroupRouter = require('./routes/suggestedGroupRoute');
const notificationRouter = require('./routes/notificationRoute');

const privateGroupRouter = require('./routes/privateGroupRoute');
const privatePostRouter = require('./routes/privatePostRoute');

const messageRouter = require('./routes/messageRoute');
const chatRouter = require('./routes/chatRoute');

// ------------------------------- middleware ---------------------------------
app.use(cors());
app.use(express.json());
app.use(
  express.urlencoded({
    extended: true,
  }),
);
app.use(express.static(path.join(__dirname, '../client/build')));

// -------------------------------- endpoints ---------------------------------
app.use('/api/user', userRouter);
app.use('/api/publicgroup', publicGroupRouter);
app.use('/api/publicpost', publicPostRouter);
app.use('/api/login', loginRouter);
app.use('/api/deactivate', deactivateRouter);
app.use('/api/file', fileRouter);
app.use('/api/popularpost', popularPostRouter);
app.use('/api/suggestedgroup', suggestedGroupRouter);
app.use('/api/notification', notificationRouter);

app.use('/api/privategroup', privateGroupRouter);
app.use('/api/privatepost', privatePostRouter);

app.use('/api/message/', messageRouter);
app.use('/api/chat/', chatRouter);

// default response
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, '../client/build/index.html'));
})

// --------------------------------- socket.io --------------------------------
let clients = [];
io.on('connection', (socket) => {
  // console.log('a user connected');

  socket.on('select', (chatID) => {
    if (!clients.some((client) => client.id === socket.id)) {
      clients.push({
        id: socket.id,
        chatID,
      });
    } else {
      clients.find((client) => client.id === socket.id).chatID = chatID;
    }
  });

  // console.log(clients);

  socket.on('newMessage', (message) => {
    const receivers = clients.filter((client) => client.chatID === message.chatID);
    for (let i = 0; i < receivers.length; i += 1) {
      const data = { chatID: message.chatID, messageID: message.id };
      io.to(receivers[i].id).emit('getMessage', data);
    }
  });

  socket.on('disconnect', () => {
    clients = clients.filter((client) => client.id !== socket.id);
    // console.log("A user disconnected.")
  });
});

// --------------------------------- start server -----------------------------
const startServer = async () => {
  try {
    mongoose.connect(
      MONGODB_URI,
      { useNewUrlParser: true, useUnifiedTopology: true },
    );
    console.log('Connected to MongoDB');
    const port = process.env.PORT || 8080;
    httpServer.listen(port, () => {
      console.log(`Server running on port: ${port}`);
    });
  } catch (err) {
    console.log(err);
  }
};

// if (require.main === module)
startServer();

// for testing
module.exports = app;
