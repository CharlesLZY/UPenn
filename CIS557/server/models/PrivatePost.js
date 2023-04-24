const mongoose = require('mongoose');

const PrivatePostSchema = new mongoose.Schema({
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

const PrivatePost = mongoose.model('PrivatePost', PrivatePostSchema);

module.exports = PrivatePost;
