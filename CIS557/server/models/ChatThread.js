const mongoose = require('mongoose');

const ConversationSchema = new mongoose.Schema({
  users: Array,
  messages: Array,
}, { versionKey: false });

const Conversation = mongoose.model('Conversation', ConversationSchema);

module.exports = Conversation;
