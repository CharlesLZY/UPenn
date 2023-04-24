const express = require('express');
const { checkPassword } = require('./utils');

// router
const router = express.Router();

// database models
const User = require('../models/User');
const PublicGroup = require('../models/PublicGroup');
const PublicPost = require('../models/PublicPost');
const PrivateGroup = require('../models/PrivateGroup');
const PrivatePost = require('../models/PrivatePost');
const FileSystem = require('../models/FileSystem');
const Message = require('../models/Message');
const ChatThread = require('../models/ChatThread');

async function notify(groupID, userID, groupType) {
  const data = {
    id: Date.now(),
    type: 'owner',
    from: groupID,
    status: groupType,
    to: 'null',
  };

  // if the user deleted his account, then fxxk me
  const userInfo = await User.findOne({ id: userID });
  let { notifications } = userInfo;
  notifications = notifications.concat([data]);
  await User.updateOne(
    {
      id: userID,
    },
    {
      $set: { notifications },
    },
  );
}

async function deletePosts(posts) {
  for (let i = 0; i < posts.length; i += 1) {
    const { postID, postType } = posts[i];
    if (postType === 'public') {
      const post = await PublicPost.findOne({ _id: postID });
      if (post !== null) {
        const { fileID } = post.attachment;
        const groupID = post.group;

        if (fileID) {
          await FileSystem.deleteFileById(fileID);
        }

        const group = await PublicGroup.findOne({ id: groupID });
        let groupPosts = group.posts;
        let { deleted_number } = group;
        groupPosts = groupPosts.filter((post) => post !== postID);
        deleted_number += 1;
        await PublicGroup.updateOne(
          {
            id: groupID,
          },
          {
            $set: { posts: groupPosts, deleted_number },
          },
        );

        await PublicPost.deleteOne({ _id: postID });
      }
    } else if (postType === 'private') {
      const post = await PrivatePost.findOne({ _id: postID });
      if (post !== null) {
        const { fileID } = post.attachment;
        const groupID = post.group;

        if (fileID) {
          await FileSystem.deleteFileById(fileID);
        }

        const group = await PrivateGroup.findOne({ id: groupID });
        let groupPosts = group.posts;
        let { deleted_number } = group;
        groupPosts = groupPosts.filter((post) => post !== postID);
        deleted_number += 1;
        await PrivateGroup.updateOne(
          {
            id: groupID,
          },
          {
            $set: { posts: groupPosts, deleted_number },
          },
        );

        await PrivatePost.deleteOne({ _id: postID });
      }
    }
  }
}

async function quitPublicGroups(groups, userID) {
  for (let i = 0; i < groups.length; i = i + 1) {
    
    const group = await PublicGroup.findOne({ id: groups[i] });
    if (group.creator === userID) {
      if (group.admins.length > 1) { // assign the ownership to an admin
        const candidate = group.admins.filter((admin) => admin !== userID)[0];
        await PublicGroup.updateOne(
          {
            id: groups[i],
          },
          {
            $set: { admins: group.admins.filter((admin) => admin !== userID), creator: candidate, members: group.members.filter((member) => member !== userID) },
          },
        );
        await notify(userID, groups[i], 'public');
      } else if (group.members.length > 1) { // promote a member to admin and assign the ownership
        const candidate = group.members.filter((member) => member !== userID)[0];
        await PublicGroup.updateOne(
          {
            id: groups[i],
          },
          {
            $set: { admins: [candidate], creator: candidate, members: group.members.filter((member) => member !== userID) },
          },
        );
        await notify(userID, groups[i], 'public');
      } else { // delete the group
        for (let j = 0; j < group.posts.length; j = j + 1) { // delete posts first
          const postID = group.posts[j];
          const post = await PublicPost.findOne({ _id: postID });
          if (post !== null) {
            const { fileID } = post.attachment;

            if (fileID) {
              await FileSystem.deleteFileById(fileID);
            }
            await PublicPost.deleteOne({ _id: postID });
          }
        }
        await PublicGroup.deleteOne({ id: groups[i] });
        
      }
    } else {
      await PublicGroup.updateOne(
        {
          id: groups[i],
        },
        {
          $set: { admins: group.admins.filter((admin) => admin !== userID), members: group.members.filter((member) => member !== userID) },
        },
      );
    }
  }
}

async function quitPrivateGroups(groups, userID) {
  for (let i = 0; i < groups.length; i = i + 1) {
    const group = await PrivateGroup.findOne({ id: groups[i] });
    if (group.creator === userID) {
      if (group.admins.length > 1) { // assign the ownership to an admin
        const candidate = group.admins.filter((admin) => admin !== userID)[0];
        await PrivateGroup.updateOne(
          {
            id: groups[i],
          },
          {
            $set: { admins: group.admins.filter((admin) => admin !== userID), creator: candidate, members: group.members.filter((member) => member !== userID) },
          },
        );
        await notify(userID, groups[i], 'private');
      } else if (group.members.length > 1) { // promote a member to admin and assign the ownership
        const candidate = group.members.filter((member) => member !== userID)[0];
        await PrivateGroup.updateOne(
          {
            id: groups[i],
          },
          {
            $set: { admins: [candidate], creator: candidate, members: group.members.filter((member) => member !== userID) },
          },
        );
        await notify(userID, groups[i], 'private');
      } else { // delete the group
        for (let j = 0; j < group.posts.length; j = j + 1) { // delete posts first
          const postID = group.posts[j];
          const post = await PrivatePost.findOne({ _id: postID });
          if (post !== null) {
            const { fileID } = post.attachment;

            if (fileID) {
              await FileSystem.deleteFileById(fileID);
            }
            await PrivatePost.deleteOne({ _id: postID });
          }
        }

        await PrivateGroup.deleteOne({ id: groups[i] });
      }
    } else {
      await PrivateGroup.updateOne(
        {
          id: groups[i],
        },
        {
          $set: { admins: group.admins.filter((admin) => admin !== userID), members: group.members.filter((member) => member !== userID) },
        },
      );
    }
  }
}

async function deleteMessages(conversations) {
  for (let i = 0; i < conversations.length; i += 1) {
    const { chatID } = conversations[i];
    const otherID = conversations[i].chatTo;
    const result = await ChatThread.findOne({ _id: chatID });

    const messagesToDel = result.messages;
    for (let j = 0; j < messagesToDel.length; j += 1) {
      const messageID = messagesToDel[j];
      const message = await Message.findOne({ _id: messageID });
      if (message) {
        if (message.type !== 'text') {
          const fileID = message.content;
          await FileSystem.deleteFileById(fileID);
        }
        await Message.deleteOne({ _id: messageID });
      }
    }
    await ChatThread.deleteOne({ _id: chatID });

    const otherInfo = await User.findOne({ id: otherID });
    if (otherInfo) {
      let { conversations } = otherInfo;
      conversations = conversations.filter((chat) => chat.chatID !== chatID);
      await User.updateOne(
        {
          id: otherID,
        },
        {
          $set: { conversations },
        },
      );
    }
  }
}

// endpoints
router.post('/', async (req, res) => {
  const { id, password } = req.body;
  const userInfo = await User.findOne({ id });
  if (userInfo === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else if (checkPassword(password, userInfo.password)) {
    await deletePosts(userInfo.posts);
    await quitPublicGroups(userInfo.publicgroups, id);
    await quitPrivateGroups(userInfo.privategroups, id);
    await deleteMessages(userInfo.conversations);
    await User.deleteOne({ id });

    res.status(200).json({ msg: 'Success.' });
  } else {
    res.status(400).json({ msg: 'Wrong Password' });
  }
});

// export
module.exports = router;
