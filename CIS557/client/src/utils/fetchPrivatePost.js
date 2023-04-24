import { baseURL } from './utils';

export async function fetchPost(postID) {
  return fetch(`${baseURL}/privatepost/${postID}`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function commentPost(postID, userID, text, groupID) {
  const res = await fetch(`${baseURL}/privatepost/${postID}/comment`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
      text,
      groupID,
    }),
  });

  const result = res.status;
  return result;
}

export async function deleteComment(postID, userID, date) {
  const res = await fetch(`${baseURL}/privatepost/${postID}/comment`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
      date,
    }),
  });

  const result = res.status;
  return result;
}

export async function flagForDeletion(postID, userID) {
  const res = await fetch(`${baseURL}/privatepost/${postID}/flag`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });

  const result = res.status;
  return result;
}

export async function unflagForDeletion(postID, userID) {
  const res = await fetch(`${baseURL}/privatepost/${postID}/flag`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });

  const result = res.status;
  return result;
}
