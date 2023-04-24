function encrypt(password) {
  const cipher = password;
  return cipher;
}

function decrypt(cipher) {
  const password = cipher;
  return password;
}

function checkPassword(password, cipher) {
  if (decrypt(cipher) === password) {
    return true;
  }
  return false;
}

function mentionExtraction(text) {
  const mentionRegex = /@((?!\d)[\w]{3,20}(?!\w))/g;
  return [...text.matchAll(mentionRegex)].map((match) => match[1]);
}

function hashTagsExtraction(text) {
  const mentionRegex = /#([\w]+(?!\w))/g;
  return [...text.matchAll(mentionRegex)].map((match) => match[1]);
}

module.exports = {
  encrypt,
  checkPassword,
  mentionExtraction,
  hashTagsExtraction,
};
