const express = require('express');

// router
const router = express.Router();

// database models
const PublicGroup = require('../models/PublicGroup');
const PublicPost = require('../models/PublicPost');
const PrivateGroup = require('../models/PrivateGroup');
const PrivatePost = require('../models/PrivatePost');
const User = require('../models/User');

router.delete('/:userID', async (req, res) => {
  const { userID } = req.params;
  const { id } = req.body;
  const result = await User.findOne({ id: userID });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { notifications } = result;
    notifications = notifications.filter((notice) => notice.id !== id);
    await User.updateOne(
      {
        id: userID,
      },
      {
        $set: { notifications },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.post('/:userID/exit', async (req, res) => {
  const { userID } = req.params;
  const { groupType, groupID } = req.body;
  const type = 'exit'; // if there is a member exit the group, admins should be notified
  const data = {
    id: Date.now(),
    type,
    from: userID,
    status: 'null',
    to: groupID,
  };
  if (groupType === 'public') {
    const result = await PublicGroup.findOne({ id: groupID });
    const { admins } = result;
    for (let i = 0; i < admins.length; i += 1) {
      const admin = await User.findOne({ id: admins[i] });
      if (admin) {
        let { notifications } = admin;
        notifications = notifications.concat([data]);
        await User.updateOne(
          {
            id: admins[i],
          },
          {
            $set: { notifications },
          },
        );
      }
    }
    res.status(200).json({ msg: 'Success.' });
  } else if (groupType === 'private') {
    const result = await PrivateGroup.findOne({ id: groupID });
    const { admins } = result;
    for (let i = 0; i < admins.length; i += 1) {
      const admin = await User.findOne({ id: admins[i] });
      if (admin) {
        let { notifications } = admin;
        notifications = notifications.concat([data]);
        await User.updateOne(
          {
            id: admins[i],
          },
          {
            $set: { notifications },
          },
        );
      }
    }
    res.status(200).json({ msg: 'Success.' });
  } else {
    res.status(400).json({ msg: 'Wrong Status' });
  }
});

router.post('/:userID/joinReq', async (req, res) => {
  const { userID } = req.params;
  const { groupID, status } = req.body;
  const type = 'joinReq'; // if the join request is handled by the admin, the user should be notified
  const data = {
    id: Date.now(),
    type,
    from: groupID,
    status,
    to: 'null',
  };
  if (status !== 'approved' && status !== 'rejected') {
    res.status(400).json({ msg: 'Wrong Status' });
  } else {
    const result = await User.findOne({ id: userID });
    if (result === null) {
      res.status(404).json({ msg: 'User not found.' });
    } else {
      let { notifications } = result;
      notifications = notifications.concat([data]);
      await User.updateOne(
        {
          id: userID,
        },
        {
          $set: { notifications },
        },
      );
      res.status(200).json({ msg: 'Success.' });
    }
  }
});

router.post('/:userID/invite', async (req, res) => {
  const { userID } = req.params;
  const { hostID, groupID, groupType } = req.body;
  const type = 'invite'; // if host invite guest to join a group, the guest should be notified
  const data = {
    id: Date.now(),
    type,
    from: hostID,
    status: groupType,
    to: groupID,
  };
  if (groupType !== 'public' && groupType !== 'private') {
    res.status(400).json({ msg: 'Wrong Status' });
  } else {
    const result = await User.findOne({ id: userID });
    if (result === null) {
      res.status(404).json({ msg: 'User not found.' });
    } else {
      let { notifications } = result;
      notifications = notifications.concat([data]);
      await User.updateOne(
        {
          id: userID,
        },
        {
          $set: { notifications },
        },
      );
      res.status(200).json({ msg: 'Success.' });
    }
  }
});

router.post('/:userID/inviteRes', async (req, res) => {
  const { userID } = req.params;
  const { guestID, groupID, status } = req.body;
  const type = 'inviteRes'; // if guest accepts or rejects the invitation, the host should be notified
  const data = {
    id: Date.now(),
    type,
    from: guestID,
    status,
    to: groupID,
  };
  if (status !== 'accepted' && status !== 'rejected') {
    res.status(400).json({ msg: 'Wrong Status' });
  } else {
    const result = await User.findOne({ id: userID });
    if (result === null) {
      res.status(404).json({ msg: 'User not found.' });
    } else {
      let { notifications } = result;
      notifications = notifications.concat([data]);
      await User.updateOne(
        {
          id: userID,
        },
        {
          $set: { notifications },
        },
      );
      res.status(200).json({ msg: 'Success.' });
    }
  }
});

router.post('/:userID/admin', async (req, res) => {
  const { userID } = req.params;
  const { groupID, status } = req.body;
  const type = 'admin'; // if user is promoted or revoked the admin, the user should be notified
  const data = {
    id: Date.now(),
    type,
    from: groupID,
    status,
    to: 'null',
  };
  if (status !== 'promoted' && status !== 'revoked') {
    res.status(400).json({ msg: 'Wrong Status' });
  } else {
    const result = await User.findOne({ id: userID });
    if (result === null) {
      res.status(404).json({ msg: 'User not found.' });
    } else {
      let { notifications } = result;
      notifications = notifications.concat([data]);
      await User.updateOne(
        {
          id: userID,
        },
        {
          $set: { notifications },
        },
      );
      res.status(200).json({ msg: 'Success.' });
    }
  }
});

router.post('/:userID/delReqRejected', async (req, res) => {
  const { userID } = req.params;
  const { groupID } = req.body;
  const type = 'delReq'; // if the deletion request is rejected, the user should be notified
  const data = {
    id: Date.now(),
    type,
    from: groupID,
    status: 'rejected',
    to: 'null',
  };
  const result = await User.findOne({ id: userID });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { notifications } = result;
    notifications = notifications.concat([data]);
    await User.updateOne(
      {
        id: userID,
      },
      {
        $set: { notifications },
      },
    );
    res.status(200).json({ msg: 'Success.' });
  }
});

router.post('/delReqApproved', async (req, res) => {
  const { groupID, groupType, postID } = req.body;
  const type1 = 'delReq'; // if the deletion request is approved, all users should be notified
  const data1 = {
    id: Date.now(),
    type: type1,
    from: groupID,
    status: 'approved',
    to: 'null',
  };

  if (groupType === 'public') {
    const post = await PublicPost.findOne({ _id: postID });
    if (post) {
      for (let i = 0; i < post.flags.length; i += 1) {
        const user = await User.findOne({ id: post.flags[i] });
        if (user) {
          let userNotification = user.notifications;
          userNotification = userNotification.concat([data1]);
          await User.updateOne(
            {
              id: post.flags[i],
            },
            {
              $set: { notifications: userNotification },
            },
          );
        }
      }
      const type2 = 'delByAdmin'; // the author of the post should be notified
      const data2 = {
        id: Date.now(),
        type: type2,
        from: groupID,
        status: 'null',
        to: 'null',
      };
      const author = await User.findOne({ id: post.author });
      if (author) {
        let authorNotification = author.notifications;
        authorNotification = authorNotification.concat([data2]);
        await User.updateOne(
          {
            id: post.author,
          },
          {
            $set: { notifications: authorNotification },
          },
        );
      }
      res.status(200).json({ msg: 'Success.' });
    } else {
      res.status(404).json({ msg: 'Post not found.' });
    }
  } else if (groupType === 'private') {
    const post = await PrivatePost.findOne({ _id: postID });
    if (post) {
      for (let i = 0; i < post.flags.length; i += 1) {
        const user = await User.findOne({ id: post.flags[i] });
        if (user) {
          let userNotification = user.notifications;
          userNotification = userNotification.concat([data1]);
          await User.updateOne(
            {
              id: post.flags[i],
            },
            {
              $set: { notifications: userNotification },
            },
          );
        }
      }
      const type2 = 'delByAdmin'; // the author of the post should be notified
      const data2 = {
        id: Date.now(),
        type: type2,
        from: groupID,
        status: 'null',
        to: 'null',
      };
      const author = await User.findOne({ id: post.author });
      if (author) {
        let authorNotification = author.notifications;
        authorNotification = authorNotification.concat([data2]);
        await User.updateOne(
          {
            id: post.author,
          },
          {
            $set: { notifications: authorNotification },
          },
        );
      }
      res.status(200).json({ msg: 'Success.' });
    } else {
      res.status(404).json({ msg: 'Post not found.' });
    }
  } else {
    res.status(400).json({ msg: 'Wrong Type' });
  }
});

module.exports = router;
