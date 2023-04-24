const mongoose = require('mongoose');

const MessageSchema = new mongoose.Schema({
  chatID: String,
  from: String,
  to: String,
  date: Number,
  content: String,
  type: String, // 'text' & 'image' & 'video' & 'audio'
}, { versionKey: false });

const Message = mongoose.model('Message', MessageSchema);

module.exports = Message;
