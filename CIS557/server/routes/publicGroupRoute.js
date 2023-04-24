const express = require('express');

// router
const router = express.Router();

// database models
const Group = require('../models/PublicGroup');
const User = require('../models/User');
const Post = require('../models/PublicPost');

router.get('/', async (req, res) => {
  const result = await Group.find();
  if (result === null) {
    res.status(404).json({ msg: 'Error' });
  } else {
    res.status(200).json(result);
  }
});

router.post('/', async (req, res) => {
  const { id, creator, tag } = req.body;
  const result = await Group.findOne({ id });
  if (result !== null) {
    res.status(409).json({ msg: 'Groupname existed.' });
  } else {
    const data = {
      id,
      tag,
      creator,
      admins: [creator],
      members: [creator],
      posts: [],
      joinRequest: [],
      deletionRequest: [],
      deleted_number: 0,
      hidden_number: 0,
    };

    const newGroup = new Group(data);
    await newGroup.save();
    res.status(201).json({ msg: 'Success.' });
  }
});

router.get('/:id', async (req, res) => {
  const { id } = req.params;
  const result = await Group.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else {
    res.status(200).json(result);
  }
});

router.post('/:id/member', async (req, res) => {
  const { id } = req.params;
  const { userID } = req.body;
  const result = await Group.findOne({ id });
  const user = await User.findOne({ id: userID });
  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else if (user === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { members, joinRequest } = result;
    let { publicgroups } = user;
    publicgroups = publicgroups.concat([id]);
    members = members.concat([userID]);
    joinRequest = joinRequest.filter((curUser) => curUser !== userID);
    await Group.updateOne(
      {
        id,
      },
      {
        $set: { members, joinRequest },
      },
    );
    await User.updateOne(
      {
        id: userID,
      },
      {
        $set: { publicgroups },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.delete('/:id/member', async (req, res) => {
  const { id } = req.params;
  const { userID } = req.body;
  const result = await Group.findOne({ id });
  const user = await User.findOne({ id: userID });
  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else if (user === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { members, admins } = result;
    let { publicgroups } = user;
    publicgroups = publicgroups.filter((group) => group !== id);
    members = members.filter((curUser) => curUser !== userID);
    admins = admins.filter((curUser) => curUser !== userID);
    await Group.updateOne(
      {
        id,
      },
      {
        $set: { members, admins },
      },
    );
    await User.updateOne(
      {
        id: userID,
      },
      {
        $set: { publicgroups },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.post('/:id/post', async (req, res) => {
  const { id } = req.params;
  const { postID } = req.body;
  const result = await Group.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else {
    let { posts } = result;
    posts = posts.concat([postID]);
    await Group.updateOne(
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

router.post('/:id/request', async (req, res) => {
  const { id } = req.params;
  const { userID } = req.body;
  const result = await Group.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else {
    let { joinRequest } = result;
    joinRequest = joinRequest.concat([userID]);
    await Group.updateOne(
      {
        id,
      },
      {
        $set: { joinRequest },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.delete('/:id/request', async (req, res) => {
  const { id } = req.params;
  const { userID } = req.body;
  const result = await Group.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else {
    let { joinRequest } = result;
    joinRequest = joinRequest.filter((curUser) => curUser !== userID);
    await Group.updateOne(
      {
        id,
      },
      {
        $set: { joinRequest },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.post('/:id/deletion', async (req, res) => {
  const { id } = req.params;
  const { userID, postID } = req.body;
  const result = await Group.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else {
    let { deletionRequest } = result;
    deletionRequest = deletionRequest.concat([{ from: userID, postID }]);
    await Group.updateOne(
      {
        id,
      },
      {
        $set: { deletionRequest },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.delete('/:id/deletion', async (req, res) => {
  const { id } = req.params;
  const { userID, postID } = req.body;
  const result = await Group.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else {
    let { deletionRequest } = result;
    deletionRequest = deletionRequest.filter(
      (request) => (request.from !== userID) || (request.postID !== postID),
    );
    await Group.updateOne(
      {
        id,
      },
      {
        $set: { deletionRequest },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.get('/:id/post/:userID', async (req, res) => {
  const { id, userID } = req.params;
  const userInfo = await User.findOne({ id: userID });
  const group = await Group.findOne({ id });
  if (group === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else {
    let postIDs = group.posts;
    if (userInfo !== null) {
      postIDs = postIDs.filter((postID) => userInfo.hidden.indexOf(postID) < 0);
    }
    const result = await Post.find({ _id: { $in: postIDs } }).sort({ date: -1 });
    res.status(200).json(result);
  }
});

router.post('/:id/admin', async (req, res) => {
  const { id } = req.params;
  const { userID } = req.body;
  const result = await Group.findOne({ id });

  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else {
    let { admins } = result;
    admins = admins.concat([userID]);
    await Group.updateOne(
      {
        id,
      },
      {
        $set: { admins },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.delete('/:id/admin', async (req, res) => {
  const { id } = req.params;
  const { userID } = req.body;
  const result = await Group.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'Group not found.' });
  } else {
    let { admins } = result;
    admins = admins.filter((user) => user !== userID);
    await Group.updateOne(
      {
        id,
      },
      {
        $set: { admins },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

module.exports = router;
