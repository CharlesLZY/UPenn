const express = require('express');
const { encrypt, checkPassword } = require('./utils');

// router
const router = express.Router();

// database models
const User = require('../models/User');
const PublicGroup = require('../models/PublicGroup');
const PublicPost = require('../models/PublicPost');
const PrivateGroup = require('../models/PrivateGroup');
const PrivatePost = require('../models/PrivatePost');

// endpoints
router.get('/', async (req, res) => {
  const result = await User.find();
  if (result === null) {
    res.status(404).json({ msg: 'Error' });
  } else {
    res.status(200).json(result);
  }
});

router.post('/', async (req, res) => {
  const { id, password } = req.body;
  const result = await User.findOne({ id });
  if (result !== null) {
    res.status(409).json({ msg: 'Username existed.' });
  } else {
    const data = {
      id,
      password: encrypt(password),
      register_date: Date.now(),
      attempts: 0,
      last_attempt: Date.now(),
      posts: [],
      publicgroups: [],
      privategroups: [],
      notifications: [],
      hidden: [],
      chats: [],
    };

    const newUser = new User(data);
    // const objectID = newUser._id;
    await newUser.save();
    res.status(201).json({ msg: 'Success.' });
  }
});

router.get('/:id', async (req, res) => {
  const { id } = req.params;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    res.status(200).json(result);
  }
});

router.put('/:id/password', async (req, res) => {
  const { id } = req.params;
  const { oldPassword, password } = req.body;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else if (checkPassword(oldPassword, result.password)) {
    await User.updateOne(
      {
        id,
      },
      {
        $set: { password: encrypt(password) },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  } else {
    res.status(400).json({ msg: 'Wrong Password' });
  }
});

router.post('/:id/post', async (req, res) => {
  const { id } = req.params;
  const { postID, postType } = req.body;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { posts } = result;
    posts = posts.concat([{ postID, postType }]);
    await User.updateOne(
      {
        id,
      },
      {
        $set: { posts },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.post('/:id/publicgroup', async (req, res) => {
  const { id } = req.params;
  const { groupID } = req.body;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { publicgroups } = result;
    publicgroups = publicgroups.concat([groupID]);
    await User.updateOne(
      {
        id,
      },
      {
        $set: { publicgroups },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.post('/:id/privategroup', async (req, res) => {
  const { id } = req.params;
  const { groupID } = req.body;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { privategroups } = result;
    privategroups = privategroups.concat([groupID]);
    await User.updateOne(
      {
        id,
      },
      {
        $set: { privategroups },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.post('/:id/hidden', async (req, res) => {
  const { id } = req.params;
  const { postID, groupID, groupType } = req.body;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { hidden } = result;
    hidden = hidden.concat([postID]);
    await User.updateOne(
      {
        id,
      },
      {
        $set: { hidden },
      },
    );
    if (groupType === 'public') {
      const group = await PublicGroup.findOne({ id: groupID });
      let { hidden_number } = group;
      hidden_number += 1;
      await PublicGroup.updateOne(
        {
          id: groupID,
        },
        {
          $set: { hidden_number },
        },
      );
    } else if (groupType === 'private') {
      const group = await PrivateGroup.findOne({ id: groupID });
      let { hidden_number } = group;
      hidden_number += 1;
      await PrivateGroup.updateOne(
        {
          id: groupID,
        },
        {
          $set: { hidden_number },
        },
      );
    }
    res.status(200).json({ msg: 'Success.' });
  }
});

router.get('/:id/post', async (req, res) => {
  const { id } = req.params;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    const { posts } = result;
    const allPosts = [];
    for (let i = 0; i < posts.length; i += 1) {
      if (posts[i].postType === 'public') {
        const post = await PublicPost.findOne({ _id: posts[i].postID });
        if (post !== null) {
          post.postType = 'publicpost';
          allPosts.push(post);
        }
      } else if (posts[i].postType === 'private') {
        const post = await PrivatePost.findOne({ _id: posts[i].postID });
        if (post !== null) {
          post.postType = 'privatepost';
          allPosts.push(post);
        }
      }
    }
    allPosts.sort((a, b) => b.date - a.date);
    res.status(200).json(allPosts);
  }
});

router.get('/:id/notificationNumber', async (req, res) => {
  const { id } = req.params;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    const { notifications } = result;
    res.status(200).json(notifications.length);
  }
});

router.delete('/:id/notification', async (req, res) => {
  const { id } = req.params;
  const { notificationID } = req.body;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { notifications } = result;
    notifications = notifications.filter((notification) => notification.id !== notificationID);
    await User.updateOne(
      {
        id,
      },
      {
        $set: { notifications },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.get('/:id/conversations', async (req, res) => {
  const { id } = req.params;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    const { conversations } = result;
    res.status(200).json(conversations);
  }
});

// export
module.exports = router;
