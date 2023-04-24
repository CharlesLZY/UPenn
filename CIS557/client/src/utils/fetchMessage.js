import { baseURL } from './utils';

async function uploadFile(file) {
  const form = new FormData();
  form.append('file', file, file.name);
  const res = await fetch(`${baseURL}/file`, {
    method: 'POST',
    body: form,
  });

  const result = await res.json(); // file id
  return result;
}

export async function sendMessage(from, to, text, file, type, chatID) {
  if (file !== null && text === null) {
    const fileID = await uploadFile(file);

    let res = await fetch(`${baseURL}/message`, {
      method: 'POST',
      headers: { 'Content-type': 'application/json' },
      body: JSON.stringify({
        chatID,
        from,
        to,
        content: fileID,
        type,
      }),
    });
    const messageID = await res.json();
    if (res.status !== 404) {
      res = await fetch(`${baseURL}/chat/${chatID}`, {
        method: 'POST',
        headers: { 'Content-type': 'application/json' },
        body: JSON.stringify({
          messageID,
          from,
          to,
        }),
      });
      return messageID;
    }
    return res.status;
  }

  let res = await fetch(`${baseURL}/message`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      chatID,
      from,
      to,
      content: text,
      type,
    }),
  });
  const messageID = await res.json();
  if (res.status !== 404) {
    res = await fetch(`${baseURL}/chat/${chatID}`, {
      method: 'POST',
      headers: { 'Content-type': 'application/json' },
      body: JSON.stringify({
        messageID,
        from,
        to,
      }),
    });
    return messageID;
  }
  return res.status;
}

export async function fetchChat(chatID) {
  return fetch(`${baseURL}/chat/${chatID}`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function fetchMessage(id) {
  return fetch(`${baseURL}/message/${id}`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function createChatThread(initiator, recipient) {
  const res = await fetch(`${baseURL}/chat`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      initiator,
      recipient,
    }),
  });
  return res.status;
}

export async function check(initiator, recipient) {
  const res = await fetch(`${baseURL}/chat/check`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      initiator,
      recipient,
    }),
  });
  return res.status;
}

export async function deleteChat(userID, chatID) {
  const res = await fetch(`${baseURL}/chat/${chatID}`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });
  return res.status;
}
