const express = require('express');

// router
const router = express.Router();

// database models
const Post = require('../models/PublicPost');

router.get('/', async (req, res) => {
  const { id_only } = req.query;
  try {
    // find all posts sorted by date and return only id if id_only is true
    const result = await Post.find().sort({ date: -1 }).select(id_only ? { _id: 1 } : {});
    if (result === null) {
      res.status(404).json({ msg: 'Error' });
    } else {
      res.status(200).json(result);
    }
  } catch (err) {
    res.status(500).json({ msg: 'Error' });
  }
});

module.exports = router;
