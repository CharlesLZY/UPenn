const express = require('express');
const { mentionExtraction, hashTagsExtraction } = require('./utils');

// router
const router = express.Router();

// database models
const Post = require('../models/PrivatePost');
const Group = require('../models/PrivateGroup');
const User = require('../models/User');
const FileSystem = require('../models/FileSystem');

router.get('/', async (req, res) => {
  const result = await Post.find();
  if (result === null) {
    res.status(404).json({ msg: 'Error' });
  } else {
    res.status(200).json(result);
  }
});

router.post('/', async (req, res) => {
  const {
    group, author, title, content, attachment,
  } = req.body;
  const hashtags = hashTagsExtraction(content);
  const data = {
    group,
    author,
    date: Date.now(),
    title,
    content,
    attachment,
    flags: [],
    type: 'privatepost',
    hashtags,
  };

  const newPost = new Post(data);
  const objectID = newPost._id;
  await newPost.save();

  const mentionedUser = mentionExtraction(content);
  if (mentionedUser.length > 0) {
    const groupData = await Group.findOne({ id: group });
    const data = {
      id: Date.now(),
      type: 'mention',
      from: author,
      status: 'post',
      to: `/privatepost/${objectID}`,
    };
    for (let i = 0; i < mentionedUser.length; i += 1) {
      if (groupData.members.indexOf(mentionedUser[i]) >= 0) {
        const userInfo = await User.findOne({ id: mentionedUser[i] });
        if (userInfo) {
          let { notifications } = userInfo;
          notifications = notifications.concat([data]);
          await User.updateOne(
            {
              id: mentionedUser[i],
            },
            {
              $set: { notifications },
            },
          );
        }
      }
    }
  }

  res.status(201).json(objectID);
});

router.get('/:id', async (req, res) => {
  const { id } = req.params;
  // check valid objectID
  if (id.match(/^[0-9a-fA-F]{24}$/)) {
    const result = await Post.findOne({ _id: id });
    if (result === null) {
      res.status(404).json(null);
    } else {
      res.status(200).json(result);
    }
  } else {
    res.status(404).json(null);
  }
});

router.post('/:id/comment', async (req, res) => {
  const { id } = req.params;
  const { userID, text, groupID } = req.body;
  const hashtags = hashTagsExtraction(text);
  const result = await Post.findOne({ _id: id });
  if (result === null) {
    res.status(404).json({ msg: 'Error' });
  } else {
    let { comments } = result;
    comments = comments.concat([{
      author: userID, date: Date.now(), text, hashtags,
    }]);
    await Post.updateOne(
      {
        _id: id,
      },
      {
        $set: { comments },
      },
    );

    const mentionedUser = mentionExtraction(text);
    if (mentionedUser.length > 0) {
      const group = await Group.findOne({ id: groupID });
      const data = {
        id: Date.now(),
        type: 'mention',
        from: userID,
        status: 'comment',
        to: `/privatepost/${id}`,
      };
      for (let i = 0; i < mentionedUser.length; i += 1) {
        if (group.members.indexOf(mentionedUser[i]) >= 0) {
          const userInfo = await User.findOne({ id: mentionedUser[i] });
          if (userInfo) {
            let { notifications } = userInfo;
            notifications = notifications.concat([data]);
            await User.updateOne(
              {
                id: mentionedUser[i],
              },
              {
                $set: { notifications },
              },
            );
          }
        }
      }
    }

    res.status(200).json({ msg: 'Success.' });
  }
});

router.delete('/:id/comment', async (req, res) => {
  const { id } = req.params;
  const { userID, date } = req.body;
  const result = await Post.findOne({ _id: id });
  if (result === null) {
    res.status(404).json({ msg: 'Error' });
  } else {
    let { comments } = result;

    comments = comments.filter((comment) => (comment.author !== userID) || (comment.date !== date));

    await Post.updateOne(
      {
        _id: id,
      },
      {
        $set: { comments },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.post('/:id/flag', async (req, res) => {
  const { id } = req.params;
  const { userID } = req.body;
  const result = await Post.findOne({ _id: id });
  if (result === null) {
    res.status(404).json({ msg: 'Error' });
  } else {
    let { flags } = result;
    flags = flags.concat([userID]);
    await Post.updateOne(
      {
        _id: id,
      },
      {
        $set: { flags },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.delete('/:id/flag', async (req, res) => {
  const { id } = req.params;
  const { userID } = req.body;
  const result = await Post.findOne({ _id: id });
  if (result === null) {
    res.status(404).json({ msg: 'Error' });
  } else {
    let { flags } = result;
    flags = flags.filter((user) => user != userID);
    await Post.updateOne(
      {
        _id: id,
      },
      {
        $set: { flags },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.delete('/:id', async (req, res) => {
  const { id } = req.params;
  const { type } = req.body;
  if (type === 'self') {
    const post = await Post.findOne({ _id: id });
    if (post === null) {
      res.status(404).json({ msg: 'Post not found.' });
    } else {
      const { fileID } = post.attachment;
      const { author } = post;
      const groupID = post.group;

      if (fileID) {
        await FileSystem.deleteFileById(fileID);
      }
      await Post.deleteOne({ _id: id });

      const userInfo = await User.findOne({ id: author });
      let userPosts = userInfo.posts;
      userPosts = userPosts.filter((post) => post.postID !== id);
      await User.updateOne(
        {
          id: author,
        },
        {
          $set: { posts: userPosts },
        },
      );

      const group = await Group.findOne({ id: groupID });
      let groupPosts = group.posts;
      let { deleted_number } = group;
      groupPosts = groupPosts.filter((post) => post !== id);
      deleted_number += 1;
      await Group.updateOne(
        {
          id: groupID,
        },
        {
          $set: { posts: groupPosts, deleted_number },
        },
      );

      res.status(200).json({ msg: 'Success.' });
    }
  } else if (type === 'admin') {
    const post = await Post.findOne({ _id: id });
    if (post === null) {
      res.status(404).json({ msg: 'Post not found.' });
    } else {
      const { fileID } = post.attachment;
      const { author } = post;
      const groupID = post.group;

      if (fileID) {
        await FileSystem.deleteFileById(fileID);
      }
      await Post.deleteOne({ _id: id });

      const userInfo = await User.findOne({ id: author });
      let userPosts = userInfo.posts;
      userPosts = userPosts.filter((post) => post.postID !== id);
      await User.updateOne(
        {
          id: author,
        },
        {
          $set: { posts: userPosts },
        },
      );

      const group = await Group.findOne({ id: groupID });
      let groupPosts = group.posts;
      let { deleted_number } = group;
      let { deletionRequest } = group;
      groupPosts = groupPosts.filter((post) => post !== id);
      deleted_number += 1;
      deletionRequest = deletionRequest.filter((req) => req.postID !== id);
      await Group.updateOne(
        {
          id: groupID,
        },
        {
          $set: { posts: groupPosts, deletionRequest, deleted_number },
        },
      );

      res.status(200).json({ msg: 'Success.' });
    }
  } else {
    res.status(400).json({ msg: 'Wrong Type' });
  }
});

module.exports = router;
