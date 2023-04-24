const express = require('express');

const router = express.Router();

const User = require('../models/User');
const Message = require('../models/Message');

router.post('/', async (req, res) => {
  const {
    chatID, from, to, content, type,
  } = req.body;

  const toUserInfo = await User.findOne({ id: to });
  if (toUserInfo === null) {
    res.status(404).json({ msg: 'User Not Found.' });
  } else {
    const data = {
      chatID,
      from,
      to,
      date: Date.now(),
      content,
      type, // 'text' & 'image' & 'video' & 'audio'
    };

    const newMessage = new Message(data);
    const objectID = newMessage._id;
    await newMessage.save();

    res.status(201).json(objectID);
  }
});

router.get('/:id', async (req, res) => {
  const { id } = req.params;
  const result = await Message.findOne({ _id: id });
  if (result) {
    res.status(200).json(result);
  } else {
    res.status(404).json({ msg: 'Message Not Found.' });
  }
});

module.exports = router;
