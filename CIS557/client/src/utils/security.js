import { baseURL } from './utils';

export async function login(userID, password) {
  const res = await fetch(`${baseURL}/login`, {
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

export async function changepassword(userID, oldPassword, password) {
  const res = await fetch(`${baseURL}/user/${userID}/password`, {
    method: 'PUT',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      oldPassword,
      password,
    }),
  });

  const result = res.status;
  return result;
}

export async function deactivate(userID, password) {
  const res = await fetch(`${baseURL}/deactivate`, {
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
