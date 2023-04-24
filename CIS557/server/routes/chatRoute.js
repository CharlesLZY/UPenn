const express = require('express');

const router = express.Router();

const User = require('../models/User');
const FileSystem = require('../models/FileSystem');
const Message = require('../models/Message');
const ChatThread = require('../models/ChatThread');

router.post('/check', async (req, res) => {
  const { initiator, recipient } = req.body;
  const initiatorInfo = await User.findOne({ id: initiator });
  const recipientInfo = await User.findOne({ id: recipient });
  if (initiatorInfo && recipientInfo) {
    const public1 = initiatorInfo.publicgroups;
    const public2 = recipientInfo.publicgroups;
    const private1 = initiatorInfo.privategroups;
    const private2 = recipientInfo.privategroups;
    let valid = false;
    for (let i = 0; i < public1.length; i += 1) {
      if (public2.indexOf(public1[i]) >= 0) {
        valid = true;
        break;
      }
    }
    if (!valid) {
      for (let i = 0; i < private1.length; i += 1) {
        if (private2.indexOf(private1[i]) >= 0) {
          valid = true;
          break;
        }
      }
    }

    if (valid) {
      res.status(200).json({ msg: 'Valid.' });
    } else {
      res.status(401).json({ msg: 'Invalid.' });
    }
  } else {
    res.status(404).json({ msg: 'User Not Found.' });
  }
});

router.post('/', async (req, res) => {
  const { initiator, recipient } = req.body;
  const data = {
    users: [initiator, recipient],
    messages: [],
  };

  const initiatorInfo = await User.findOne({ id: initiator });
  const recipientInfo = await User.findOne({ id: recipient });
  if (initiatorInfo && recipientInfo) {
    const result = await ChatThread.findOne({ users: { $all: [initiator, recipient] } });

    if (result) {
      let { conversations } = initiatorInfo;
      const index = conversations.findIndex((chat) => chat.chatID.toString() === result._id.toString());
      if (index < 0) {
        conversations = conversations.concat([{ chatID: result._id.toString(), chatTo: recipient, last_date: Date.now() }]);
        await User.updateOne(
          {
            id: initiator,
          },
          {
            $set: { conversations },
          },
        );
      } else {
        conversations[index].last_date = Date.now();
        await User.updateOne(
          {
            id: initiator,
          },
          {
            $set: { conversations },
          },
        );
      }
      res.status(201).json(result._id);
    } else {
      const newChatThread = new ChatThread(data);
      const objectID = newChatThread._id;
      await newChatThread.save();
      let { conversations } = initiatorInfo;
      conversations = conversations.concat([{ chatID: objectID.toString(), chatTo: recipient, last_date: Date.now() }]);
      await User.updateOne(
        {
          id: initiator,
        },
        {
          $set: { conversations },
        },
      );
      res.status(201).json(objectID);
    }
  } else {
    res.status(404).json({ msg: 'User Not Found.' });
  }
});

router.post('/:id', async (req, res) => {
  const { id } = req.params;
  const { messageID, from, to } = req.body;
  const result = await ChatThread.findOne({ _id: id });
  if (result) {
    let { messages } = result;
    messages = messages.concat([messageID]);
    await ChatThread.updateOne(
      {
        _id: id,
      },
      {
        $set: { messages },
      },
    );

    const user1 = await User.findOne({ id: from });
    const user2 = await User.findOne({ id: to });
    // the user who sends the message mush have the chat thread
    const conversationUser1 = user1.conversations;
    conversationUser1[conversationUser1.findIndex((chat) => chat.chatID === id)].last_date = Date.now();
    await User.updateOne(
      {
        id: from,
      },
      {
        $set: { conversations: conversationUser1 },
      },
    );
    if (user2) {
      let conversationUser2 = user2.conversations;
      const index = conversationUser2.findIndex((chat) => chat.chatID === id);
      if (index < 0) {
        conversationUser2 = conversationUser2.concat([{ chatID: id, chatTo: from, last_date: Date.now() }]);
      } else {
        conversationUser2[index].last_date = Date.now();
      }
      await User.updateOne(
        {
          id: to,
        },
        {
          $set: { conversations: conversationUser2 },
        },
      );
      res.status(200).json({ msg: 'Success' });
    } else {
      res.status(404).json({ msg: 'User Not Found.' });
    }
  } else {
    res.status(404).json({ msg: 'Chat Thread Not Found.' });
  }
});

router.delete('/:id', async (req, res) => {
  const { id } = req.params;
  const { userID } = req.body;
  const result = await ChatThread.findOne({ _id: id });
  if (result) {
    const { users } = result;
    const otherID = users[users.findIndex((user) => user !== userID)];
    const userInfo = await User.findOne({ id: userID });
    const otherInfo = await User.findOne({ id: otherID });
    if (userInfo) {
      let { conversations } = userInfo;
      conversations = conversations.filter((chat) => chat.chatID !== id);
      await User.updateOne(
        {
          id: userID,
        },
        {
          $set: { conversations },
        },
      );
      if (otherInfo === null || otherInfo.conversations.findIndex((chat) => chat.chatID === id) < 0) {
        const messagesToDel = result.messages;
        for (let i = 0; i < messagesToDel.length; i += 1) {
          const messageID = messagesToDel[i];
          const message = await Message.findOne({ _id: messageID });
          if (message) {
            if (message.type !== 'text') {
              const fileID = message.content;
              await FileSystem.deleteFileById(fileID);
            }
            await Message.deleteOne({ _id: messageID });
          }
        }
        await ChatThread.deleteOne({ _id: id });
      }
      res.status(200).json({ msg: 'Success.' });
    } else {
      res.status(404).json({ msg: 'User Not Found.' });
    }
  } else {
    res.status(404).json({ msg: 'Chat Thread Not Found.' });
  }
});

router.get('/:id', async (req, res) => {
  const { id } = req.params;
  const chat = await ChatThread.findOne({ _id: id });
  if (chat) {
    const { messages } = chat;
    const result = [];
    for (let i = 0; i < messages.length; i += 1) {
      const message = await Message.findOne({ _id: messages[i] });
      if (message) {
        result.push(message);
      }
    }
    res.status(200).json(result);
  } else {
    res.status(404).json({ msg: 'Chat Thread Not Found.' });
  }
});

module.exports = router;
