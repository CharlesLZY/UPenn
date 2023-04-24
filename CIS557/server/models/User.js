const mongoose = require('mongoose');

const UserSchema = new mongoose.Schema({
  id: String,
  password: String,
  register_date: Number,
  attempts: Number,
  last_attempt: Number,
  posts: Array,
  publicgroups: Array,
  privategroups: Array,
  notifications: Array,
  hidden: Array,
  conversations: Array,
}, { versionKey: false });

const User = mongoose.model('User', UserSchema);

module.exports = User;
