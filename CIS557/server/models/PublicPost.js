const mongoose = require('mongoose');

const PublicPostSchema = new mongoose.Schema({
  group: String,
  author: String,
  date: Number,
  title: String,
  content: String,
  attachment: Object,
  comments: Array,
  flags: Array,
  type: String,
  hashtags: Array,
}, { versionKey: false });

const PublicPost = mongoose.model('PublicPost', PublicPostSchema);

module.exports = PublicPost;
