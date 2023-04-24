import React from 'react';
import { useNavigate } from 'react-router-dom';
import { Button } from '@mui/material';

import * as fetchGroup from '../../utils/fetchPublicGroup';
import * as utils from '../../utils/utils';

export default function JoinButton({ userInfo, group }) {
  const groupID = group ? group.id : null;
  const userID = userInfo ? userInfo.id : null;
  const navigate = useNavigate();

  const handleClickJoin = async () => {
    if (!userID) {
      navigate('/login');
    } else {
      const res = await fetchGroup.requestJoin(groupID, userID);
      navigate(0);
    }
  };

  if (utils.checkUserInPublicGroup(userInfo, groupID)) {
    return (
      <Button
        sx={{
          height: '40px',
          width: '80px',
          borderRadius: '30px',
          border: '3px solid #9D2933',
          fontSize: 15,
        }}
        variant="outlined"
        color="secondary"
        disabled
      >
        Joined
      </Button>
    );
  }
  if (utils.checkUserRequested(userID, group)) {
    return (
      <Button
        sx={{
          height: '40px',
          width: '80px',
          borderRadius: '30px',
          border: '3px solid #9D2933',
          fontSize: 11,
        }}
        variant="outlined"
        color="secondary"
        disabled
      >
        Requested
      </Button>
    );
  }
  return (
    <Button
      sx={{
        height: '40px',
        width: '80px',
        borderRadius: '30px',
        border: '3px solid #9D2933',
        fontSize: 20,
        '&:hover': {
          background: '#9D2933',
          color: '#ffffff',
          border: '3px solid #9D2933',
        },
      }}
      variant="outlined"
      color="secondary"
      onClick={handleClickJoin}
    >
      Join
    </Button>
  );
}
