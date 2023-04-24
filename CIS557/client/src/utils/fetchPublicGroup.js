import { baseURL } from './utils';

export async function createPublicGroup(groupID, creator, tag) {
  let res = await fetch(`${baseURL}/publicgroup`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      id: groupID,
      creator,
      tag,
    }),
  });

  if (res.status === 201) {
    res = await fetch(`${baseURL}/user/${creator}/publicgroup`, {
      method: 'POST',
      headers: { 'Content-type': 'application/json' },
      body: JSON.stringify({
        groupID,
      }),
    });
  }
  const result = res.status;
  return result;
}

export async function fetchAllPublicGroups() {
  return fetch(`${baseURL}/publicgroup/`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function fetchGroup(groupID) {
  return fetch(`${baseURL}/publicgroup/${groupID}`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function fetchSuggestedGroup(userID) {
  return fetch(`${baseURL}/suggestedgroup/${userID}`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function requestJoin(groupID, userID) {
  const res = await fetch(`${baseURL}/publicgroup/${groupID}/request`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });

  const result = res.status;
  return result;
}

export async function removeRequestJoin(groupID, userID) {
  let res = await fetch(`${baseURL}/publicgroup/${groupID}/request`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });
  res = await fetch(`${baseURL}/notification/${userID}/joinReq`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      groupID,
      status: 'rejected',
    }),
  });
  const result = res.status;
  return result;
}

export async function approveRequestJoin(groupID, userID) {
  let res = await fetch(`${baseURL}/publicgroup/${groupID}/member`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });
  res = await fetch(`${baseURL}/notification/${userID}/joinReq`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      groupID,
      status: 'approved',
    }),
  });

  const result = res.status;
  return result;
}

export async function flagForDeletion(groupID, userID, postID) {
  const res = await fetch(`${baseURL}/publicgroup/${groupID}/deletion`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
      postID,
    }),
  });
  const result = res.status;
  return result;
}

export async function unflagForDeletion(groupID, userID, postID) {
  let res = await fetch(`${baseURL}/publicgroup/${groupID}/deletion`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
      postID,
    }),
  });

  res = await fetch(`${baseURL}/notification/${userID}/delReqRejected`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      groupID,
    }),
  });
  const result = res.status;
  return result;
}

export async function fetchGroupPost(groupID, userID) {
  return fetch(`${baseURL}/publicgroup/${groupID}/post/${userID}`)
    .then(async (res) => {
      const data = await res.json();
      if (res.status === 404) {
        return null;
      }
      return data;
    });
}

export async function exitGroup(groupID, userID) {
  let res = await fetch(`${baseURL}/publicgroup/${groupID}/member`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });
  if (res.status !== 404) {
    res = await fetch(`${baseURL}/notification/${userID}/exit`, {
      method: 'POST',
      headers: { 'Content-type': 'application/json' },
      body: JSON.stringify({
        groupType: 'public',
        groupID,
      }),
    });
  }
  return res.status;
}

export async function promoteAdmin(groupID, userID) {
  let res = await fetch(`${baseURL}/publicgroup/${groupID}/admin`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });

  res = await fetch(`${baseURL}/notification/${userID}/admin`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      groupID,
      status: 'promoted',
    }),
  });

  const result = res.status;
  return result;
}

export async function revokeAdmin(groupID, userID) {
  let res = await fetch(`${baseURL}/publicgroup/${groupID}/admin`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      userID,
    }),
  });
  res = await fetch(`${baseURL}/notification/${userID}/admin`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      groupID,
      status: 'revoked',
    }),
  });

  const result = res.status;
  return result;
}

export async function deletePost(postID, groupID) {
  let res = await fetch(`${baseURL}/notification/delReqApproved`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      groupID,
      groupType: 'public',
      postID,
    }),
  });
  console.log(res);
  res = await fetch(`${baseURL}/publicpost/${postID}`, {
    method: 'DELETE',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      type: 'admin',
    }),
  });
  const result = res.status;
  return result;
}
