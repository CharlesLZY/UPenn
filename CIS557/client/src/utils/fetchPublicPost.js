import { baseURL } from './utils';

export async function fetchPopularPost() {
  return fetch(`${baseURL}/popularpost/`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

// infinite scroll
export async function fetchPopularPostId() {
  return fetch(`${baseURL}/popularpost/?id_only=true`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function fetchPost(postID) {
  return fetch(`${baseURL}/publicpost/${postID}`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function commentPost(postID, userID, text, groupID) {
  const res = await fetch(`${baseURL}/publicpost/${postID}/comment`, {
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
  const res = await fetch(`${baseURL}/publicpost/${postID}/comment`, {
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
  const res = await fetch(`${baseURL}/publicpost/${postID}/flag`, {
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
  const res = await fetch(`${baseURL}/publicpost/${postID}/flag`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });

  const result = res.status;
  return result;
}
