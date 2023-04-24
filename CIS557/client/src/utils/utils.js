export const baseURL = process.env.NODE_ENV === 'production'
  ? '/api' : 'http://localhost:8080/api';

export function checkString(str) {
  if (str === '') {
    return false;
  }
  return true;
}

export function checkUsername(str) {
  const usernameRegex = /^(?!\d)[\w]{3,20}$/;
  return usernameRegex.test(str);
}

export function checkPassword(str) {
  const passwordRegex = /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[$@!%*?&+-_])[a-zA-Z0-9$@!%*?&+-_]{8,32}$/;
  return passwordRegex.test(str);
}

export function checkFile(file) {
  if (file === null) {
    return true;
  }
  const supportFileType = ['image/jpeg', 'image/png', 'image/jpg', 'image/gif', 'video/mp4', 'audio/mpeg'];
  // fize size limitation: 30 MB
  if (file.size > 30 * 1024 * 1024) {
    return false;
  }
  if (supportFileType.indexOf(file.type) < 0) {
    return false;
  }

  return true;
}

export function checkImage(file) {
  if (file === null || file === undefined) {
    return false;
  }
  const supportFileType = ['image/jpeg', 'image/png', 'image/jpg', 'image/gif'];
  // fize size limitation: 5 MB
  if (file.size > 5 * 1024 * 1024) {
    return false;
  }
  if (supportFileType.indexOf(file.type) < 0) {
    return false;
  }

  return true;
}

export function checkAudio(file) {
  if (file === null || file === undefined) {
    return false;
  }
  const supportFileType = ['audio/mpeg'];
  // fize size limitation: 10 MB
  if (file.size > 10 * 1024 * 1024) {
    return false;
  }
  if (supportFileType.indexOf(file.type) < 0) {
    return false;
  }

  return true;
}

export function checkVideo(file) {
  if (file === null || file === undefined) {
    return false;
  }
  const supportFileType = ['video/mp4'];
  // fize size limitation: 15 MB
  if (file.size > 15 * 1024 * 1024) {
    return false;
  }
  if (supportFileType.indexOf(file.type) < 0) {
    return false;
  }

  return true;
}

export function convertTime(timeStamp) {
  const dic = {
    1: 'Jan', 2: 'Feb', 3: 'Mar', 4: 'Apr', 5: 'May', 6: 'Jun', 7: 'Jul', 8: 'Aug', 9: 'Sep', 10: 'Oct', 11: 'Nov', 12: 'Dec',
  };
  const date = new Date(timeStamp);
  return (`${dic[date.getMonth() + 1]} ${date.getDate()} ${date.getFullYear()} `);
}

export function messageTime(timeStamp) {
  const time = new Date(timeStamp);
  return (`${time.toLocaleDateString()} ${time.toLocaleTimeString()}`);
}

export function parseFileType(fileType) {
  if (fileType) {
    const prefix = fileType.split('/')[0];
    if (prefix === 'image') {
      return ('img');
    } if (prefix === 'video') {
      return ('video');
    } if (prefix === 'audio') {
      return ('audio');
    }
    return ('img');
  }
  return ('img');
}

export function checkUserInPublicGroup(userInfo, groupID) {
  if (userInfo) {
    if (userInfo.publicgroups.indexOf(groupID) >= 0) {
      return true;
    }
    return false;
  }
  return false;
}

export function checkUserInPrivateGroup(userInfo, groupID) {
  if (userInfo) {
    if (userInfo.privategroups.indexOf(groupID) >= 0) {
      return true;
    }
    return false;
  }
  return false;
}

export function checkUserRequested(userID, group) {
  if (group) {
    const requests = group.joinRequest;

    if (requests.indexOf(userID) >= 0) {
      return true;
    }
  }
  return false;
}
