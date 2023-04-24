const express = require('express');

// router
const router = express.Router();

// database models
const Group = require('../models/PublicGroup');
const User = require('../models/User');

router.get('/:id', async (req, res) => {
  const { id } = req.params;
  const user = await User.findOne({ id });
  if (user === null) {
    res.status(404).json({ msg: 'User not found.' });
  }

  const groups = await Group.find();
  if (groups === null) {
    res.status(404).json({ msg: 'Error' });
  }
  const groupUserNotIn = groups.filter((group) => user.publicgroups.indexOf(group.id) < 0);
  if (groupUserNotIn.length === 0) {
    res.status(200).json([]);
  } else {
    const suggestion = groupUserNotIn[Math.floor(Math.random() * groupUserNotIn.length)].id;
    res.status(200).json([suggestion]);
  }
});

module.exports = router;
