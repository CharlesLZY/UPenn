const express = require('express');
const { checkPassword } = require('./utils');

// router
const router = express.Router();

// database models
const User = require('../models/User');

// middleware

// endpoints
router.post('/', async (req, res) => {
  const { id } = req.body;
  const { password } = req.body;
  const result = await User.findOne({ id });
  if (result === null) {
    res.status(404).json({ msg: 'User not found.' });
  } else {
    let { attempts } = result;
    let { last_attempt } = result;
    const cipher = result.password;
    const now = Date.now();
    if (attempts >= 3) {
      const timeDiff = now - last_attempt;
      if (timeDiff / 1000 > 1 * 60) { // 1 min
        attempts = 0;
        last_attempt = now;
      } else {
        res.status(401).json({ msg: 'Too many unsuccessful attempts. The acount is locked.' });
      }
    }

    if (attempts < 3) {
      if (checkPassword(password, cipher)) {
        attempts = 0;
        last_attempt = now;
        res.status(200).json({ msg: 'Success.' });
      } else {
        attempts += 1;
        last_attempt = now;
        res.status(400).json({ msg: 'Wrong password.' });
      }
    }

    await User.updateOne(
      {
        id,
      },
      {
        $set: { attempts, last_attempt },
      },
    );
  }
});

// export
module.exports = router;
