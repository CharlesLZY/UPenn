const mongoose = require('mongoose');

const PrivateGroupSchema = new mongoose.Schema({
  id: String,
  tag: String,
  creator: String,
  admins: Array,
  members: Array,
  posts: Array,
  joinRequest: Array,
  deletionRequest: Array,
  deleted_number: Number,
  hidden_number: Number,
}, { versionKey: false });

const PrivateGroup = mongoose.model('PrivateGroup', PrivateGroupSchema);

module.exports = PrivateGroup;
