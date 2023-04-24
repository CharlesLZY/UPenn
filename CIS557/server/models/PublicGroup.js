const mongoose = require('mongoose');

const PublicGroupSchema = new mongoose.Schema({
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

const PublicGroup = mongoose.model('PublicGroup', PublicGroupSchema);

module.exports = PublicGroup;
