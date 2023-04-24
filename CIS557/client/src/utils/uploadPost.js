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

export async function uploadPublicPost(data) {
  const {
    group, author, title, content, file,
  } = data;
  let fileID = null;
  let fileType = null;
  if (file) {
    fileID = await uploadFile(file);
    fileType = file.type;
  }
  const res = await fetch(`${baseURL}/publicpost`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      group,
      author,
      title,
      content,
      attachment: { fileID, fileType },
    }),
  });

  const postID = await res.json();

  await fetch(`${baseURL}/user/${author}/post`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      postID,
      postType: 'public',
    }),
  });

  await fetch(`${baseURL}/publicgroup/${group}/post`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      postID,
    }),
  });
}

export async function uploadPrivatePost(data) {
  const {
    group, author, title, content, file,
  } = data;
  let fileID = null;
  let fileType = null;
  if (file) {
    fileID = await uploadFile(file);
    fileType = file.type;
  }
  const res = await fetch(`${baseURL}/privatepost`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      group,
      author,
      title,
      content,
      attachment: { fileID, fileType },
    }),
  });

  const postID = await res.json();

  await fetch(`${baseURL}/user/${author}/post`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      postID,
      postType: 'private',
    }),
  });

  await fetch(`${baseURL}/privategroup/${group}/post`, {
    method: 'POST',
    headers: { 'Content-type': 'application/json' },
    body: JSON.stringify({
      postID,
    }),
  });
}
