import { baseURL } from './utils';

export async function fetchUser(userID) {
  return fetch(`${baseURL}/user/${userID}`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function register(userID, password) {
  const res = await fetch(`${baseURL}/user`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      id: userID,
      password,
    }),
  });

  const result = res.status;
  return result;
}

export async function hidePost(userID, postID, groupID, groupType) {
  const res = await fetch(`${baseURL}/user/${userID}/hidden`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      postID,
      groupID,
      groupType,
    }),
  });

  const result = res.status;
  return result;
}

export async function invite(hostID, guestID, groupID, groupType) {
  const res = await fetch(`${baseURL}/notification/${guestID}/invite`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      hostID,
      groupID,
      groupType,
    }),
  });

  const result = res.status;
  return result;
}

export async function fetchUserPost(userID) {
  return fetch(`${baseURL}/user/${userID}/post`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function fetchUserNotificationNumber(userID) {
  return fetch(`${baseURL}/user/${userID}/notificationNumber`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function deleteNotification(userID, id) {
  const res = await fetch(`${baseURL}/user/${userID}/notification`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      notificationID: id,
    }),
  });

  const result = res.status;
  return result;
}

export async function deletePost(postID, postType) {
  const res = await fetch(`${baseURL}/${postType}/${postID}`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      type: 'self',
    }),
  });

  const result = res.status;
  return result;
}

export async function rejectInvitation(guestID, hostID, groupID) {
  const res = await fetch(`${baseURL}/notification/${hostID}/inviteRes`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      guestID,
      groupID,
      status: 'rejected',
    }),
  });
  const result = res.status;
  return result;
}

export async function acceptInvitation(guestID, hostID, groupID) {
  const res = await fetch(`${baseURL}/notification/${hostID}/inviteRes`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      guestID,
      groupID,
      status: 'accepted',
    }),
  });
  console.log(res);
  const result = res.status;
  return result;
}

export async function fetchChat(userID) {
  return fetch(`${baseURL}/user/${userID}/conversations`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}
